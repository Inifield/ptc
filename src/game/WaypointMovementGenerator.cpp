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

#include "WaypointMovementGenerator.h"
#include "ObjectMgr.h"
#include "MapManager.h" // for flightmaster grid preloading
#include "Creature.h"
#include "CreatureAI.h"
#include "CreatureGroups.h"
#include "Player.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Spell.h"

void WaypointMovementGenerator<Creature>::LoadPath(Creature& creature)
{
    if (!path_id)
        path_id = creature.GetWaypointPath();

    i_path = sWaypointMgr->GetPath(path_id);

    if (!i_path)
    {
        // No movement found for entry
        sLog.outErrorDb("WaypointMovementGenerator::LoadPath: creature %s (Entry: %u GUID: %u) doesn't have waypoint path id: %u", creature.GetName(), creature.GetEntry(), creature.GetGUIDLow(), path_id);
        return;
    }

    StartMoveNow(creature);
}

void WaypointMovementGenerator<Creature>::Initialize(Creature& creature)
{
    LoadPath(creature);
    creature.AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
}

void WaypointMovementGenerator<Creature>::Finalize(Creature& creature)
{
    creature.ClearUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    creature.SetWalk(false);
}

void WaypointMovementGenerator<Creature>::Reset(Creature& creature)
{
    creature.AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    StartMoveNow(creature);
}

void WaypointMovementGenerator<Creature>::OnArrived(Creature& creature)
{
    if (!i_path || i_path->empty())
        return;
    if (m_isArrivalDone)
        return;

    creature.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
    m_isArrivalDone = true;

    if (i_path->at(i_currentNode)->event_id && urand(0, 99) < i_path->at(i_currentNode)->event_chance)
    {
        ;//sLog->outDebug(LOG_FILTER_MAPSCRIPTS, "Creature movement start script %u at point %u for "UI64FMTD".", i_path->at(i_currentNode)->event_id, i_currentNode, creature.GetGUID());
        creature.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        creature.GetMap()->ScriptsStart(sWaypointScripts, i_path->at(i_currentNode)->event_id, &creature, NULL);
    }

    // Inform script
    MovementInform(creature);
    creature.UpdateWaypointID(i_currentNode);

    if (i_path->at(i_currentNode)->delay)
    {
        creature.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        Stop(i_path->at(i_currentNode)->delay);
    }
}

bool WaypointMovementGenerator<Creature>::StartMove(Creature& creature)
{
    if (!i_path || i_path->empty())
        return false;

    // Xinef: Dont allow dead creatures to move
    if (!creature.IsAlive())
        return false;

    if (Stopped())
        return true;

    if (m_isArrivalDone)
    {
        // Xinef: not true... update this at every waypoint!
        //if ((i_currentNode == i_path->size() - 1) && !repeating) // If that's our last waypoint
        {
            float x = i_path->at(i_currentNode)->x;
            float y = i_path->at(i_currentNode)->y;
            float z = i_path->at(i_currentNode)->z;
            float o = creature.GetOrientation();
        }

        // Xinef: moved the upper IF here
        if ((i_currentNode == i_path->size() - 1) && !repeating) // If that's our last waypoint
        {
            creature.GetMotionMaster()->Initialize();
            return false;
        }

        i_currentNode = (i_currentNode + 1) % i_path->size();
    }

    // xinef: do not initialize motion if we got stunned in movementinform
    if (creature.HasUnitState(UNIT_STATE_NOT_MOVE))
        return true;

    WaypointData const* node = i_path->at(i_currentNode);

    m_isArrivalDone = false;

    creature.AddUnitState(UNIT_STATE_ROAMING_MOVE);

    Movement::Location formationDest(node->x, node->y, node->z, 0.0f);
    Movement::MoveSplineInit init(creature);

    //! Do not use formationDest here, MoveTo requires transport offsets due to DisableTransportPathTransformations() call
    //! but formationDest contains global coordinates
    init.MoveTo(node->x, node->y, node->z);

    //! Accepts angles such as 0.00001 and -0.00001, 0 must be ignored, default value in waypoint table
    if (node->orientation && node->delay)
        init.SetFacing(node->orientation);

    if (!node->run)
        init.SetWalk(true);
    else
        init.SetWalk(false);

    init.Launch();

    //Call for creature group update
    if (creature.GetFormation() && creature.GetFormation()->getLeader() == &creature)
        creature.GetFormation()->LeaderMoveTo(formationDest.x, formationDest.y, formationDest.z);

    return true;
}

bool WaypointMovementGenerator<Creature>::Update(Creature& creature, const uint32& diff)
{
    // Waypoint movement can be switched on/off
    // This is quite handy for escort quests and other stuff
    if (creature.HasUnitState(UNIT_STATE_NOT_MOVE))
    {
        creature.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }
    // prevent a crash at empty waypoint path.
    if (!i_path || i_path->empty())
        return false;

    // Xinef: Dont allow dead creatures to move
    if (!creature.IsAlive())
        return false;

    // prevent movement while casting spells with cast time or channel time
    if (creature.HasUnitState(UNIT_STATE_CASTING))
    {
        bool stop = true;
        if (Spell* spell = creature.GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            if (!(spell->m_spellInfo->ChannelInterruptFlags & (AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING)) && !(spell->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT))
                stop = false;

        if (stop)
        {
            Stop(1000);
            if (!creature.IsStopped())
                creature.StopMoving();
            return true;
        }
    }

    if (Stopped())
    {
        if (CanMove(diff))
            return StartMove(creature);
    }
    else
    {
        if (creature.IsStopped())
            Stop(STOP_TIME_FOR_PLAYER);
        else
        {
            // xinef: code to detect pre-empetively if we should start movement to next waypoint
            // xinef: do not start pre-empetive movement if current node has delay or we are ending waypoint movement
            bool finished = creature.movespline->Finalized();
            if (!finished && !i_path->at(i_currentNode)->delay && ((i_currentNode != i_path->size() - 1) || repeating))
                finished = (creature.movespline->_Spline().length(creature.movespline->_currentSplineIdx() + 1) - creature.movespline->timePassed()) < 200;

            if (finished)
            {
                OnArrived(creature);
                return StartMove(creature);
            }
        }
    }
    return true;
}

void WaypointMovementGenerator<Creature>::MovementInform(Creature& creature)
{
    if (creature.AI())
        creature.AI()->MovementInform(WAYPOINT_MOTION_TYPE, i_currentNode);
}

//----------------------------------------------------//
uint32 FlightPathMovementGenerator::GetPathAtMapEnd() const
{
    if (i_currentNode >= i_path->size())
        return i_path->size();

    uint32 curMapId = (*i_path)[i_currentNode].mapid;
    for (uint32 i = i_currentNode; i < i_path->size(); ++i)
    {
        if ((*i_path)[i].mapid != curMapId)
            return i;
    }

    return i_path->size();
}

void FlightPathMovementGenerator::Initialize(Player& player)
{
    Reset(player);

    // For preloading end grid
    InitEndGridInfo();
}

void FlightPathMovementGenerator::Finalize(Player& player)
{
    // remove flag to prevent send object build movement packets for flight state and crash (movement generator already not at top of stack)
    player.ClearUnitState(UNIT_FLAG_DISABLE_MOVE | UNIT_STATE_IN_FLIGHT);

    player.Dismount();
    player.RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_TAXI_FLIGHT);

    if (player.m_taxi.empty())
    {
        player.getHostileRefManager().setOnlineOfflineState(true);
        if (player.pvpInfo.inHostileArea)
            player.CastSpell(&player, 2479, true);

        // update z position to ground and orientation for landing point
        // this prevent cheating with landing  point at lags
        // when client side flight end early in comparison server side
        player.StopMoving();
    }
}

#define PLAYER_FLIGHT_SPEED        32.0f

void FlightPathMovementGenerator::Reset(Player& player)
{
    player.getHostileRefManager().setOnlineOfflineState(false);
    player.AddUnitState(UNIT_STATE_IN_FLIGHT);
    player.SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_TAXI_FLIGHT);

    Movement::MoveSplineInit init(player);
    uint32 end = GetPathAtMapEnd();
    for (uint32 i = GetCurrentNode(); i != end; ++i)
    {
        G3D::Vector3 vertice((*i_path)[i].x, (*i_path)[i].y, (*i_path)[i].z);
        init.Path().push_back(vertice);
    }
    init.SetFirstPointId(GetCurrentNode());
    init.SetFly();
    init.SetVelocity(PLAYER_FLIGHT_SPEED);
    init.Launch();
}

bool FlightPathMovementGenerator::Update(Player& player, const uint32& diff)
{
    uint32 pointId = (uint32)player.movespline->currentPathIdx();
    if (pointId > i_currentNode)
    {
        bool departureEvent = true;
        do
        {
            DoEventIfAny(player, (*i_path)[i_currentNode], departureEvent);
            if (pointId == i_currentNode)
                break;
            if (i_currentNode == m_preloadTargetNode)
                PreloadEndGrid();
            i_currentNode += (uint32)departureEvent;
            departureEvent = !departureEvent;
        }
        while (true);
    }

    return i_currentNode < (i_path->size()-1);
}

void FlightPathMovementGenerator::SetCurrentNodeAfterTeleport()
{
    if (i_path->empty())
        return;

    uint32 map0 = (*i_path)[0].mapid;
    for (size_t i = 1; i < i_path->size(); ++i)
    {
        if ((*i_path)[i].mapid != map0)
        {
            i_currentNode = i;
            return;
        }
    }
}

void FlightPathMovementGenerator::DoEventIfAny(Player& player, TaxiPathNodeEntry const& node, bool departure)
{
    if (uint32 eventid = departure ? node.departureEventID : node.arrivalEventID)
    {
        sLog.outDebug("Taxi %s event %u of node %u of path %u for player %s", departure ? "departure" : "arrival", eventid, node.index, node.path, player.GetName());
        player.GetMap()->ScriptsStart(sEventScripts, eventid, &player, &player);
    }
}

bool FlightPathMovementGenerator::GetResetPosition(Player&, float& x, float& y, float& z, float& o) const
{
    const TaxiPathNodeEntry& node = (*i_path)[i_currentNode];
    x = node.x; y = node.y; z = node.z;
    return true;
}

void FlightPathMovementGenerator::InitEndGridInfo()
{
    /*! Storage to preload flightmaster grid at end of flight. For multi-stop flights, this will
       be reinitialized for each flightmaster at the end of each spline (or stop) in the flight. */
    uint32 nodeCount = (*i_path).size();         //! Number of nodes in path.
    m_endMapId = (*i_path)[nodeCount - 1].mapid; //! MapId of last node
    m_preloadTargetNode = nodeCount - 3;
    m_endGridX = (*i_path)[nodeCount - 1].x;
    m_endGridY = (*i_path)[nodeCount - 1].y;
}

void FlightPathMovementGenerator::PreloadEndGrid()
{
    // used to preload the final grid where the flightmaster is
    Map* endMap = MapManager::Instance().FindMap(m_endMapId);

    // Load the grid
    if (endMap)
    {
        sLog.outDetail("Preloading flightmaster at grid (%f, %f) for map %u", m_endGridX, m_endGridY, m_endMapId);
        endMap->LoadGrid(m_endGridX, m_endGridY);
    }
    else
        sLog.outDetail("Unable to determine map to preload flightmaster grid");
}
