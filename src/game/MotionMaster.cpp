/*
 * This file is part of the OregonCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"
#include "ConfusedMovementGenerator.h"
#include "FleeingMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "EscortMovementGenerator.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"

inline bool isStatic(MovementGenerator* mv)
{
    return (mv == &si_idleMovement);
}

void
MotionMaster::Initialize()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator* curr = top();
        pop();
        if (curr) DirectDelete(curr);
    }

    InitDefault();
}

// set new default movement generator
void MotionMaster::InitDefault()
{
    // Xinef: Do not allow to initialize any motion generator for dead creatures
    if (i_owner->GetTypeId() == TYPEID_UNIT && i_owner->IsAlive())
    {
        MovementGenerator* movement = FactorySelector::selectMovementGenerator(i_owner->ToCreature());
        Mutate(movement == NULL ? &si_idleMovement : movement, MOTION_SLOT_IDLE);
    }
    else
    {
        Mutate(&si_idleMovement, MOTION_SLOT_IDLE);
    }
}

MotionMaster::~MotionMaster()
{
    // clear ALL movement generators (including default)
    while (!empty())
    {
        MovementGenerator* curr = top();
        pop();
        if (curr && !isStatic(curr))
            delete curr;    // Skip finalizing on delete, it might launch new movement
    }
}

void
MotionMaster::UpdateMotion(uint32 diff)
{
    if (!i_owner)
        return;

    if (i_owner->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED)) // what about UNIT_STATE_DISTRACTED? Why is this not included?
        return;

    ASSERT(!empty());

    m_cleanFlag |= MMCF_INUSE;

    m_cleanFlag |= MMCF_UPDATE;
    if (!top()->Update(*i_owner, diff))
    {
        m_cleanFlag &= ~MMCF_UPDATE;
        MovementExpired();
    }
    else
        m_cleanFlag &= ~MMCF_UPDATE;

    if (m_expList)
    {
        for (size_t i = 0; i < m_expList->size(); ++i)
        {
            MovementGenerator* mg = (*m_expList)[i];
            DirectDelete(mg);
        }

        delete m_expList;
        m_expList = NULL;

        if (empty())
            Initialize();
        else if (needInitTop())
            InitTop();
        else if (m_cleanFlag & MMCF_RESET)
            top()->Reset(*i_owner);

        m_cleanFlag &= ~MMCF_RESET;
    }

    m_cleanFlag &= ~MMCF_INUSE;
}

void
MotionMaster::DirectClean(bool reset)
{
    while (size() > 1)
    {
        MovementGenerator* curr = top();
        pop();
        if (curr) DirectDelete(curr);
    }

    if (empty())
        return;

    if (needInitTop())
        InitTop();
    else if (reset)
        top()->Reset(*i_owner);
}

void
MotionMaster::DelayedClean()
{
    while (size() > 1)
    {
        MovementGenerator* curr = top();
        pop();
        if (curr)
            DelayedDelete(curr);
    }
}

void
MotionMaster::DirectExpire(bool reset)
{
    if (size() > 1)
    {
        MovementGenerator* curr = top();
        pop();
        DirectDelete(curr);
    }

    while (!empty() && !top())
        --i_top;

    if (empty())
        Initialize();
    else if (needInitTop())
        InitTop();
    else if (reset)
        top()->Reset(*i_owner);
}

void
MotionMaster::DelayedExpire()
{
    if (size() > 1)
    {
        MovementGenerator* curr = top();
        pop();
        DelayedDelete(curr);
    }

    while (!empty() && !top())
        --i_top;
}

void MotionMaster::MoveIdle(MovementSlot slot)
{
    //! Should be preceded by MovementExpired or Clear if there's an overlying movementgenerator active
    if (empty() || !isStatic(top()))
        Mutate(&si_idleMovement, MOTION_SLOT_IDLE);;
}

void
MotionMaster::MoveRandom(float spawndist)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->GetTypeId() == TYPEID_UNIT)
    {
        DEBUG_LOG("Creature (GUID: %u) start moving random", i_owner->GetGUIDLow());
        Mutate(new RandomMovementGenerator<Creature>(spawndist), MOTION_SLOT_IDLE);
    }
}

void
MotionMaster::MoveTargetedHome()
{
    Clear(false);

    if (i_owner->GetTypeId() == TYPEID_UNIT && !i_owner->ToCreature()->GetCharmerOrOwnerGUID())
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) targeted home", i_owner->GetEntry(), i_owner->GetGUIDLow());
        Mutate(new HomeMovementGenerator<Creature>(), MOTION_SLOT_ACTIVE);
    }
    else if (i_owner->GetTypeId() == TYPEID_UNIT && i_owner->ToCreature()->GetCharmerOrOwnerGUID())
    {
        i_owner->ClearUnitState(UNIT_STATE_EVADE);
        // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
        if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
            return;

        DEBUG_LOG("Creature (Entry: %u GUID: %u) targeted home", i_owner->GetEntry(), i_owner->GetGUIDLow());
        Unit* target = i_owner->ToCreature()->GetCharmerOrOwner();
        if (target)
        {
            ;//sLog->outStaticDebug("Following %s (GUID: %u)", target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature", target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : ((Creature*)target)->GetDBTableGUIDLow());
            Mutate(new FollowMovementGenerator<Creature>(*target, PET_FOLLOW_DIST, i_owner->GetFollowAngle()), MOTION_SLOT_ACTIVE);
        }
    }
    else
    {
        sLog.outError("Player (GUID: %u) attempt targeted home", i_owner->GetGUIDLow());
    }
}

void
MotionMaster::MoveConfused()
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) move confused", i_owner->GetGUIDLow());
        Mutate(new ConfusedMovementGenerator<Player>(), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) move confused", i_owner->GetEntry(), i_owner->GetGUIDLow());
        Mutate(new ConfusedMovementGenerator<Creature>(), MOTION_SLOT_CONTROLLED);
    }
}

void
MotionMaster::MoveChase(Unit* target, float dist, float angle)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    // ignore movement request if target not exist
    if (!target || target == i_owner || i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    //_owner->ClearUnitState(UNIT_STATE_FOLLOW);
    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) chase to %s (GUID: %u)",
            i_owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetDBTableGUIDLow());
        Mutate(new ChaseMovementGenerator<Player>(*target, dist, angle), MOTION_SLOT_ACTIVE);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) chase to %s (GUID: %u)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetDBTableGUIDLow());
        Mutate(new ChaseMovementGenerator<Creature>(*target, dist, angle), MOTION_SLOT_ACTIVE);
    }
}

void
MotionMaster::MoveFollow(Unit* target, float dist, float angle, MovementSlot slot)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    // ignore movement request if target not exist
    if (!target || target == i_owner || i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    //_owner->AddUnitState(UNIT_STATE_FOLLOW);
    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) follow to %s (GUID: %u)", i_owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetDBTableGUIDLow());
        Mutate(new FollowMovementGenerator<Player>(*target, dist, angle), slot);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) follow to %s (GUID: %u)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(),
            target->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId() == TYPEID_PLAYER ? target->GetGUIDLow() : target->ToCreature()->GetDBTableGUIDLow());
        Mutate(new FollowMovementGenerator<Creature>(*target, dist, angle), slot);
    }
}

void
MotionMaster::MovePoint(uint32 id, float x, float y, float z, bool usePathfinding)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) targeted point (Id: %u X: %f Y: %f Z: %f)", i_owner->GetGUIDLow(), id, x, y, z);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, usePathfinding), MOTION_SLOT_ACTIVE);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) targeted point (ID: %u X: %f Y: %f Z: %f)",
                  i_owner->GetEntry(), i_owner->GetGUIDLow(), id, x, y, z);
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, usePathfinding), MOTION_SLOT_ACTIVE);
    }
}

void 
MotionMaster::MoveSplinePath(Movement::PointsArray* path)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) targeted point (Id: %u X: %f Y: %f Z: %f)", i_owner->GetGUIDLow(), id, x, y, z);
        Mutate(new EscortMovementGenerator<Player>(path), MOTION_SLOT_ACTIVE);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) targeted point (ID: %u X: %f Y: %f Z: %f)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(), id, x, y, z);
        Mutate(new EscortMovementGenerator<Creature>(path), MOTION_SLOT_ACTIVE);
    }
}

void 
MotionMaster::MoveCharge(float x, float y, float z, float speed, uint32 id, const Movement::PointsArray* path, bool generatePath)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (Impl[MOTION_SLOT_CONTROLLED] && Impl[MOTION_SLOT_CONTROLLED]->GetMovementGeneratorType() != DISTRACT_MOTION_TYPE)
        return;

    i_owner->AddUnitState(UNIT_STATE_CHARGING);
    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) charge point (X: %f Y: %f Z: %f)", i_owner->GetGUIDLow(), x, y, z);
        Mutate(new PointMovementGenerator<Player>(id, x, y, z, speed, path, generatePath, generatePath), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) charge point (X: %f Y: %f Z: %f)",
                  i_owner->GetEntry(), i_owner->GetGUIDLow(), x, y, z);
        Mutate(new PointMovementGenerator<Creature>(id, x, y, z, speed, path, generatePath, generatePath), MOTION_SLOT_CONTROLLED);
    }
}

void MotionMaster::MoveFall(uint32 id /*=0*/, bool addFlagForNPC)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    // use larger distance for vmap height search than in most other cases
    float tz = i_owner->GetMap()->GetHeight(i_owner->GetPositionX(), i_owner->GetPositionY(), i_owner->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (tz <= INVALID_HEIGHT)
    {
        sLog.outDebug("MotionMaster::MoveFall: unable retrive a proper height at map %u (x: %f, y: %f, z: %f) Z: %f.",
            i_owner->GetMap()->GetId(), i_owner->GetPositionX(), i_owner->GetPositionX(), i_owner->GetPositionZ(), tz);
        return;
    }

    // Abort too if the ground is very near
    if (fabs(i_owner->GetPositionZ() - tz) < 0.1f)
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        i_owner->AddUnitMovementFlag(MOVEMENTFLAG_FALLING);
        i_owner->m_movementInfo.SetFallTime(0);
        i_owner->ToPlayer()->SetFallInformation(time(NULL), i_owner->GetPositionZ());
    }
    else if (i_owner->GetTypeId() == TYPEID_UNIT && addFlagForNPC) // pussywizard
    {
        i_owner->RemoveUnitMovementFlag(MOVEMENTFLAG_MOVING);
        i_owner->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING | MOVEMENTFLAG_CAN_FLY);
        i_owner->AddUnitMovementFlag(MOVEMENTFLAG_FALLING);
        i_owner->m_movementInfo.SetFallTime(0);
        i_owner->SendMovementFlagUpdate();
    }

    Movement::MoveSplineInit init(*i_owner);
    init.MoveTo(i_owner->GetPositionX(), i_owner->GetPositionY(), tz);
    init.SetFall();
    init.Launch();
    Mutate(new EffectMovementGenerator(id), MOTION_SLOT_CONTROLLED);
}

void
MotionMaster::MoveSeekAssistance(float x, float y, float z)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog.outError("Player (GUID: %u) attempt to seek assistance", i_owner->GetGUIDLow());
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) seek assistance (X: %f Y: %f Z: %f)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(), x, y, z);
        i_owner->AttackStop();
        i_owner->CastStop();
        i_owner->ToCreature()->SetReactState(REACT_PASSIVE);
        Mutate(new AssistanceMovementGenerator(x, y, z), MOTION_SLOT_ACTIVE);
    }
}

void 
MotionMaster::MoveSeekAssistanceDistract(uint32 time)
{
    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        sLog.outError("Player (GUID: %u) attempt to call distract after assistance", i_owner->GetGUIDLow());
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) is distracted after assistance call (Time: %u)",
            _owner->GetEntry(), _owner->GetGUIDLow(), time);
        Mutate(new AssistanceDistractMovementGenerator(time), MOTION_SLOT_ACTIVE);
    }
}

void
MotionMaster::MoveFleeing(Unit* enemy, uint32 time)
{
    if (!enemy)
        return;

    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) flee from %s (GUID: %u)", i_owner->GetGUIDLow(),
                  enemy->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
                  enemy->GetTypeId() == TYPEID_PLAYER ? enemy->GetGUIDLow() : enemy->ToCreature()->GetDBTableGUIDLow());
        Mutate(new FleeingMovementGenerator<Player>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) flee from %s (GUID: %u)",
                  i_owner->GetEntry(), i_owner->GetGUIDLow(),
                  enemy->GetTypeId() == TYPEID_PLAYER ? "player" : "creature",
                  enemy->GetTypeId() == TYPEID_PLAYER ? enemy->GetGUIDLow() : enemy->ToCreature()->GetDBTableGUIDLow());
        if (time)
            Mutate(new TimedFleeingMovementGenerator(enemy->GetGUID(), time), MOTION_SLOT_CONTROLLED);
        else
            Mutate(new FleeingMovementGenerator<Creature>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
}

void
MotionMaster::MoveTaxiFlight(uint32 path, uint32 pathnode)
{
    if (i_owner->GetTypeId() == TYPEID_PLAYER)
    {
        if (path < sTaxiPathNodesByPath.size())
        {
            sLog.outDebug("%s taxi to (Path %u node %u)", i_owner->GetName(), path, pathnode);
            FlightPathMovementGenerator* mgen = new FlightPathMovementGenerator(sTaxiPathNodesByPath[path], pathnode);
            // mgen->LoadPath(*i_owner->ToPlayer());
            Mutate(mgen, MOTION_SLOT_CONTROLLED);
        }
        else
        {
            sLog.outError("%s attempt taxi to (not existed Path %u node %u)",
                          i_owner->GetName(), path, pathnode);
        }
    }
    else
    {
        sLog.outError("Creature (Entry: %u GUID: %u) attempt taxi to (Path %u node %u)",
                      i_owner->GetEntry(), i_owner->GetGUIDLow(), path, pathnode);
    }
}

void
MotionMaster::MoveDistract(uint32 timer)
{
    if (Impl[MOTION_SLOT_CONTROLLED])
        return;

    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (i_owner->GetTypeId() == TYPEID_PLAYER)
        DEBUG_LOG("Player (GUID: %u) distracted (timer: %u)", i_owner->GetGUIDLow(), timer);
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) (timer: %u)",
                  i_owner->GetEntry(), i_owner->GetGUIDLow(), timer);
    }

    DistractMovementGenerator* mgen = new DistractMovementGenerator(timer);
    Mutate(mgen, MOTION_SLOT_CONTROLLED);
}

void MotionMaster::Mutate(MovementGenerator* m, MovementSlot slot)
{
    while (MovementGenerator *curr = Impl[slot])
    {
        bool delayed = (i_top == slot && (m_cleanFlag & MMCF_UPDATE));

        // pussywizard: clear slot AND decrease top immediately to avoid crashes when referencing null top in DirectDelete
        Impl[slot] = NULL;
        while (!empty() && !top())
            --i_top;

        if (delayed)
            DelayedDelete(curr);
        else
            DirectDelete(curr);
    }

    if (i_top < slot)
        i_top = slot;

    Impl[slot] = m;
    if (i_top > slot)
        needInit[slot] = true;
    else
    {
        needInit[slot] = false;
        m->Initialize(*i_owner);
    }
}

void MotionMaster::MovePath(uint32 path_id, bool repeatable)
{
    if (!path_id)
        return;

    // Xinef: do not allow to move with UNIT_FLAG_DISABLE_MOVE
    if (i_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    //We set waypoint movement as new default movement generator
    // clear ALL movement generators (including default)
    /*while (!empty())
    {
        MovementGenerator *curr = top();
        curr->Finalize(*i_owner);
        pop();
        if (!isStatic(curr))
            delete curr;
    }*/

    //i_owner->GetTypeId() == TYPEID_PLAYER ?
    //Mutate(new WaypointMovementGenerator<Player>(path_id, repeatable)):
    Mutate(new WaypointMovementGenerator<Creature>(path_id, repeatable), MOTION_SLOT_IDLE);

    DEBUG_LOG("%s (GUID: %u) start moving over path(Id:%u, repeatable: %s)",
              i_owner->GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature",
              i_owner->GetGUIDLow(), path_id, repeatable ? "YES" : "NO");
}

void MotionMaster::MoveRotate(uint32 time, RotateDirection direction)
{
    if (!time)
        return;

    Mutate(new RotateMovementGenerator(time, direction), MOTION_SLOT_ACTIVE);
}

void MotionMaster::propagateSpeedChange()
{
    /*Impl::container_type::iterator it = Impl::c.begin();
    for (; it != end(); ++it)
    {
        (*it)->unitSpeedChanged();
    }*/
    for (int i = 0; i <= i_top; ++i)
    {
        if (Impl[i])
            Impl[i]->unitSpeedChanged();
    }
}

void MotionMaster::ReinitializeMovement()
{
    for (int i = 0; i <= i_top; ++i)
    {
        if (Impl[i])
            Impl[i]->Reset(*i_owner);
    }
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
    if (empty())
        return IDLE_MOTION_TYPE;

    return top()->GetMovementGeneratorType();
}

MovementGeneratorType MotionMaster::GetMotionSlotType(int slot) const
{
    if (!Impl[slot])
        return NULL_MOTION_TYPE;
    else
        return Impl[slot]->GetMovementGeneratorType();
}

// Xinef: Escort system
uint32 MotionMaster::GetCurrentSplineId() const
{
    if (empty())
        return 0;

    return top()->GetSplineId();
}

void MotionMaster::InitTop()
{
    top()->Initialize(*i_owner);
    needInit[i_top] = false;
}

void MotionMaster::DirectDelete(_Ty curr)
{
    if (isStatic(curr))
        return;
    curr->Finalize(*i_owner);
    delete curr;
}

void MotionMaster::DelayedDelete(_Ty curr)
{
    sLog.outDebug("Unit (Entry %u) is trying to delete its updating MG (Type %u)!", i_owner->GetEntry(), curr->GetMovementGeneratorType());

    if (isStatic(curr))
        return;
    if (!m_expList)
        m_expList = new ExpireList();
    m_expList->push_back(curr);
}

bool MotionMaster::GetDestination(float& x, float& y, float& z)
{
    if (i_owner->movespline->Finalized())
        return false;

    const G3D::Vector3& dest = i_owner->movespline->FinalDestination();
    x = dest.x;
    y = dest.y;
    z = dest.z;
    return true;
}
