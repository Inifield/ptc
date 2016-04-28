/*
Written by Xinef
 */

#include "EscortMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "World.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"

template<class T>
void EscortMovementGenerator<T>::Initialize(T& unit)
{
    if (!unit.IsStopped())
        unit.StopMoving();

    unit.AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
    i_recalculateSpeed = false;
    Movement::MoveSplineInit init(unit);

    if (m_precomputedPath.size() == 2) // xinef: simple case, just call move to
        init.MoveTo(m_precomputedPath[1].x, m_precomputedPath[1].y, m_precomputedPath[1].z);
    else if (m_precomputedPath.size())
        init.MovebyPath(m_precomputedPath);

    init.Launch();

    _splineId = unit.movespline->GetId();
}

template<class T>
bool EscortMovementGenerator<T>::Update(T& unit, const uint32& time_diff)
{
    if (!&unit)
        return false;

    if (unit.HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED))
    {
        unit.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    unit.AddUnitState(UNIT_STATE_ROAMING_MOVE);

    bool arrived = unit.movespline->Finalized();

    if (i_recalculateSpeed && !arrived)
    {
        i_recalculateSpeed = false;
        Movement::MoveSplineInit init(unit);

        // xinef: speed changed during path execution, calculate remaining path and launch it once more
        if (m_precomputedPath.size())
        {
            uint32 offset = std::min(uint32(unit.movespline->_currentSplineIdx()), uint32(m_precomputedPath.size()));
            Movement::PointsArray::iterator offsetItr = m_precomputedPath.begin();
            std::advance(offsetItr, offset);
            m_precomputedPath.erase(m_precomputedPath.begin(), offsetItr);

            // restore 0 element (current position)
            m_precomputedPath.insert(m_precomputedPath.begin(), G3D::Vector3(unit.GetPositionX(), unit.GetPositionY(), unit.GetPositionZ()));

            if (m_precomputedPath.size() > 2)
                init.MovebyPath(m_precomputedPath);
            else if (m_precomputedPath.size() == 2)
                init.MoveTo(m_precomputedPath[1].x, m_precomputedPath[1].y, m_precomputedPath[1].z);
        }

        init.Launch();
        // Xinef: Override spline Id on recalculate launch
        _splineId = unit.movespline->GetId();
    }

    return !arrived;
}

template<class T>
void EscortMovementGenerator<T>::Finalize(T& unit)
{
    unit.ClearUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
}

template<class T>
void EscortMovementGenerator<T>::Reset(T& unit)
{
    if (!unit.IsStopped())
        unit.StopMoving();

    unit.AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);
}

template void EscortMovementGenerator<Player>::Initialize(Player&);
template void EscortMovementGenerator<Creature>::Initialize(Creature&);
template void EscortMovementGenerator<Player>::Finalize(Player&);
template void EscortMovementGenerator<Creature>::Finalize(Creature&);
template void EscortMovementGenerator<Player>::Reset(Player&);
template void EscortMovementGenerator<Creature>::Reset(Creature&);
template bool EscortMovementGenerator<Player>::Update(Player& unit, const uint32&);
template bool EscortMovementGenerator<Creature>::Update(Creature& unit, const uint32&);
