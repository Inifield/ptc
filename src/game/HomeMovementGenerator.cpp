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

#include "HomeMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"

void HomeMovementGenerator<Creature>::Initialize(Creature& owner)
{
    _setTargetLocation(owner);
}

void HomeMovementGenerator<Creature>::Finalize(Creature & owner)
{
    if (arrived)
    {
        owner.ClearUnitState(UNIT_STATE_EVADE);
        owner.SetWalk(!owner.HasUnitState(UNIT_STATE_RUNNING_STATE) && !owner.IsLevitating());
        owner.LoadCreaturesAddon(true);
        owner.AI()->JustReachedHome();
    }
}

void HomeMovementGenerator<Creature>::Reset(Creature&) { }

void HomeMovementGenerator<Creature>::_setTargetLocation(Creature& owner)
{
    if (owner.HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
        return;

    float x, y, z;
    Movement::MoveSplineInit init(owner);

    owner.GetHomePosition(x, y, z, ori);

    init.SetFacing(ori);
    init.MoveTo(x, y, z, true);
    init.SetWalk(false);
    init.Launch();

    arrived = false;
    owner.ClearUnitState(UNIT_STATE_ALL_STATE);
}

bool HomeMovementGenerator<Creature>::Update(Creature& owner, const uint32& time_diff)
{
    arrived = owner.movespline->Finalized();
    return !arrived;
}
