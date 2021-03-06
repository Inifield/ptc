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

/* ScriptData
SDName: Boss_Maiden_of_Virtue
SD%Complete: 100
SDComment:
SDCategory: Karazhan
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

#define SAY_AGGRO               -1532018
#define SAY_SLAY1               -1532019
#define SAY_SLAY2               -1532020
#define SAY_SLAY3               -1532021
#define SAY_REPENTANCE1         -1532022
#define SAY_REPENTANCE2         -1532023
#define SAY_DEATH               -1532024

#define SPELL_REPENTANCE        29511
#define SPELL_HOLYFIRE          29522
#define SPELL_HOLYWRATH         32445
#define SPELL_HOLYGROUND        29512
#define SPELL_BERSERK           26662

struct boss_maiden_of_virtueAI : public ScriptedAI
{
    boss_maiden_of_virtueAI(Creature* c) : ScriptedAI(c) {}

    uint32 Repentance_Timer;
    uint32 Holyfire_Timer;
    uint32 Holywrath_Timer;
    uint32 Holyground_Timer;
    uint32 Enrage_Timer;

    bool Enraged;

    void Reset()
    {
        Repentance_Timer    = 30000 + (rand() % 15000);
        Holyfire_Timer      = 8000 + (rand() % 17000);
        Holywrath_Timer     = 20000 + (rand() % 10000);
        Holyground_Timer    = 3000;
        Enrage_Timer        = 600000;

        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_AURA_PERIODIC_MANA_LEECH, true);

        Enraged = false;
    }

    void KilledUnit(Unit* /*Victim*/)
    {
        if (urand(0, 1) == 0)
            DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2, SAY_SLAY3), me);
    }

    void JustDied(Unit* /*Killer*/)
    {
        DoScriptText(SAY_DEATH, me);
    }

    void EnterCombat(Unit* /*who*/)
    {
        DoScriptText(SAY_AGGRO, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Enrage_Timer <= diff && !Enraged)
        {
            DoCast(me, SPELL_BERSERK, true);
            Enraged = true;
        }
        else Enrage_Timer -= diff;

        if (Holyground_Timer <= diff)
        {
            DoCast(me, SPELL_HOLYGROUND, true);   //Triggered so it doesn't interrupt her at all
            Holyground_Timer = 1500;
        }
        else Holyground_Timer -= diff;

        if (Repentance_Timer <= diff)
        {
            DoCastVictim( SPELL_REPENTANCE);
            DoScriptText(RAND(SAY_REPENTANCE1, SAY_REPENTANCE2), me);

            Repentance_Timer = 32000;
        }
        else Repentance_Timer -= diff;

        if (Holyfire_Timer <= diff)
        {
            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                DoCast(pTarget, SPELL_HOLYFIRE);

            Holyfire_Timer = urand(10000, 17000);     //Anywhere from 8 to 25 seconds, good luck having several of those in a row!
        }
        else Holyfire_Timer -= diff;

        if (Holywrath_Timer <= diff)
        {
            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                DoCast(pTarget, SPELL_HOLYWRATH);

            Holywrath_Timer = urand(8000, 15000);
        }
        else Holywrath_Timer -= diff;

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_maiden_of_virtue(Creature* pCreature)
{
    return new boss_maiden_of_virtueAI (pCreature);
}

void AddSC_boss_maiden_of_virtue()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "boss_maiden_of_virtue";
    newscript->GetAI = &GetAI_boss_maiden_of_virtue;
    newscript->RegisterSelf();
}

