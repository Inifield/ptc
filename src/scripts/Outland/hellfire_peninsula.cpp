/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2010-2011 OregonCore <http://www.oregoncore.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Hellfire_Peninsula
SD%Complete: 95
 SDComment: Quest support: 9375, 9418, 10129, 10146, 10162, 10163, 10340, 10346, 10347, 10382 (Special flight paths), 10909, 10935.
SDCategory: Hellfire Peninsula
EndScriptData */

/* ContentData
npc_aeranas
npc_ancestral_wolf
go_haaleshi_altar
npc_naladu
npc_tracy_proudwell
npc_trollbane
npc_fel_guard_hound
npc_wing_commander_dabiree
npc_gryphoneer_leafbeard
npc_wing_commander_brack
npc_wounded_blood_elf
npc_hand_berserker
npc_anchorite_relic_bunny
npc_anchorite_barada
npc_darkness_released
npc_foul_purge
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

/*######
## npc_aeranas
######*/
enum eAeranas
{
    SAY_SUMMON                      = -1000138,
    SAY_FREE                        = -1000139,

    FACTION_HOSTILE                 = 16,
    FACTION_FRIENDLY                = 35,

    SPELL_ENVELOPING_WINDS          = 15535,
    SPELL_SHOCK                     = 12553,

    C_AERANAS                       = 17085
};

struct npc_aeranasAI : public ScriptedAI
{
    npc_aeranasAI(Creature* c) : ScriptedAI(c) {}

    uint32 Faction_Timer;
    uint32 EnvelopingWinds_Timer;
    uint32 Shock_Timer;

    void Reset()
    {
        Faction_Timer = 8000;
        EnvelopingWinds_Timer = 9000;
        Shock_Timer = 5000;

        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->setFaction(FACTION_FRIENDLY);

        DoScriptText(SAY_SUMMON, me);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if (Faction_Timer)
        {
            if (Faction_Timer <= diff)
            {
                me->setFaction(FACTION_HOSTILE);
                Faction_Timer = 0;
            } else Faction_Timer -= diff;
        }

        if (!UpdateVictim())
            return;

        if ((me->GetHealth()*100) / me->GetMaxHealth() < 30)
        {
            me->setFaction(FACTION_FRIENDLY);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->RemoveAllAuras();
            me->DeleteThreatList();
            me->CombatStop();
            DoScriptText(SAY_FREE, me);
            return;
        }

        if (Shock_Timer <= diff)
        {
            DoCast(me->getVictim(),SPELL_SHOCK);
            Shock_Timer = 10000;
        } else Shock_Timer -= diff;

        if (EnvelopingWinds_Timer <= diff)
        {
            DoCast(me->getVictim(),SPELL_ENVELOPING_WINDS);
            EnvelopingWinds_Timer = 25000;
        } else EnvelopingWinds_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_aeranas(Creature* pCreature)
{
    return new npc_aeranasAI (pCreature);
}

/*######
## npc_ancestral_wolf
######*/

enum eAncestralWolf
{
    EMOTE_WOLF_LIFT_HEAD            = -1000496,
    EMOTE_WOLF_HOWL                 = -1000497,
    SAY_WOLF_WELCOME                = -1000498,

    SPELL_ANCESTRAL_WOLF_BUFF       = 29981,

    NPC_RYGA                        = 17123
};

struct npc_ancestral_wolfAI : public npc_escortAI
{
    npc_ancestral_wolfAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        if (pCreature->GetOwner() && pCreature->GetOwner()->GetTypeId() == TYPEID_PLAYER)
            Start(false, false, pCreature->GetOwner()->GetGUID());
        else
            sLog.outError("TRINITY: npc_ancestral_wolf can not obtain owner or owner is not a player.");

        pCreature->SetSpeed(MOVE_WALK, 1.5f);
        Reset();
    }

    Unit* pRyga;

    void Reset()
    {
        pRyga = NULL;
        DoCast(me, SPELL_ANCESTRAL_WOLF_BUFF, true);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!pRyga && pWho->GetTypeId() == TYPEID_UNIT && pWho->GetEntry() == NPC_RYGA && me->IsWithinDistInMap(pWho, 15.0f))
            pRyga = pWho;

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 0:
                DoScriptText(EMOTE_WOLF_LIFT_HEAD, me);
                break;
            case 2:
                DoScriptText(EMOTE_WOLF_HOWL, me);
                break;
            case 50:
                if (pRyga && pRyga->isAlive() && !pRyga->isInCombat())
                    DoScriptText(SAY_WOLF_WELCOME, pRyga);
                break;
        }
    }
};

CreatureAI* GetAI_npc_ancestral_wolf(Creature* pCreature)
{
    return new npc_ancestral_wolfAI(pCreature);
}

/*######
## go_haaleshi_altar
######*/

bool GOHello_go_haaleshi_altar(Player *player, GameObject* _GO)
{
    _GO->SummonCreature(C_AERANAS,-1321.79f, 4043.80f, 116.24f, 1.25f, TEMPSUMMON_TIMED_DESPAWN, 180000);
    return false;
}


/*######
## npc_naladu
######*/

#define GOSSIP_NALADU_ITEM1 "Why don't you escape?"

enum eNaladu
{
    GOSSIP_TEXTID_NALADU1   = 9788
};

bool GossipHello_npc_naladu(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_NALADU_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_naladu(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_NALADU1, pCreature->GetGUID());

    return true;
}

/*######
## npc_tracy_proudwell
######*/

#define GOSSIP_TEXT_REDEEM_MARKS        "I have marks to redeem!"
#define GOSSIP_TRACY_PROUDWELL_ITEM1    "I heard that your dog Fei Fei took Klatu's prayer beads..."
#define GOSSIP_TRACY_PROUDWELL_ITEM2    "<back>"

enum eTracy
{
    GOSSIP_TEXTID_TRACY_PROUDWELL1       = 10689,
    QUEST_DIGGING_FOR_PRAYER_BEADS       = 10916
};

bool GossipHello_npc_tracy_proudwell(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pCreature->isVendor())
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_REDEEM_MARKS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    if (pPlayer->GetQuestStatus(QUEST_DIGGING_FOR_PRAYER_BEADS) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TRACY_PROUDWELL_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_tracy_proudwell(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TRACY_PROUDWELL_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TRACY_PROUDWELL1, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_TRADE:
            pPlayer->SEND_VENDORLIST(pCreature->GetGUID());
            break;
    }

    return true;
}

/*######
## npc_trollbane
######*/

#define GOSSIP_TROLLBANE_ITEM1      "Tell me of the Sons of Lothar."
#define GOSSIP_TROLLBANE_ITEM2      "<more>"
#define GOSSIP_TROLLBANE_ITEM3      "Tell me of your homeland."

enum eTrollbane
{
    GOSSIP_TEXTID_TROLLBANE1        = 9932,
    GOSSIP_TEXTID_TROLLBANE2        = 9933,
    GOSSIP_TEXTID_TROLLBANE3        = 8772
};

bool GossipHello_npc_trollbane(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TROLLBANE_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TROLLBANE_ITEM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_trollbane(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TROLLBANE_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TROLLBANE1, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TROLLBANE2, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_TROLLBANE3, pCreature->GetGUID());
            break;
    }

    return true;
}

/*######
## npc_fel_guard_hound
######*/

enum eFelGuard
{
    SPELL_SUMMON_POO                              = 37688,

    NPC_DERANGED_HELBOAR                          = 16863
};

struct npc_fel_guard_houndAI : public ScriptedAI
{
    npc_fel_guard_houndAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    uint32 uiCheckTimer;
    uint64 uiHelboarGUID;

    void Reset()
    {
        uiCheckTimer = 5000; //check for creature every 5 sec
        uiHelboarGUID = 0;
    }

    void MovementInform(uint32 uiType, uint32 uiId)
    {
        if (uiType != POINT_MOTION_TYPE || uiId != 1)
            return;

        if (Creature* pHelboar = me->GetCreature(*me,uiHelboarGUID))
        {
            pHelboar->RemoveCorpse();
            DoCast(SPELL_SUMMON_POO);

            if (Player* pOwner = me->GetCharmerOrOwnerPlayerOrPlayerItself())
                me->GetMotionMaster()->MoveFollow(pOwner,0.0f,0.0f);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (uiCheckTimer <= uiDiff)
        {
            if (Creature* pHelboar = me->FindNearestCreature(NPC_DERANGED_HELBOAR, 10.0f, false))
            {
                if (pHelboar->GetGUID() != uiHelboarGUID && me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE && !me->FindCurrentSpellBySpellId(SPELL_SUMMON_POO))
                {
                    uiHelboarGUID = pHelboar->GetGUID();
                    me->GetMotionMaster()->MovePoint(1,pHelboar->GetPositionX(),pHelboar->GetPositionY(),pHelboar->GetPositionZ());
                }
            }
            uiCheckTimer = 5000;
        }else uiCheckTimer -= uiDiff;

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_fel_guard_hound(Creature* pCreature)
{
    return new npc_fel_guard_houndAI(pCreature);
}

/*######
## npc_wing_commander_dabiree
######*/

#define GOSSIP_ITEM1_DAB "Fly me to Murketh and Shaadraz Gateways"
#define GOSSIP_ITEM2_DAB "Fly me to Shatter Point"

bool GossipHello_npc_wing_commander_dabiree(Player *player, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        player->PrepareQuestMenu(pCreature->GetGUID());

    //Mission: The Murketh and Shaadraz Gateways
    if (player->GetQuestStatus(10146) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM1_DAB, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Shatter Point
    if (!player->GetQuestRewardStatus(10340))
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM2_DAB, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(pCreature), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_wing_commander_dabiree(Player *player, Creature* pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33768,true);               //TaxiPath 585 (Gateways Murket and Shaadraz)
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,35069,true);               //TaxiPath 612 (Taxi - Hellfire Peninsula - Expedition Point to Shatter Point)
    }
    return true;
}

/*######
## npc_gryphoneer_leafbeard
######*/

enum
{
    SPELL_TAXI_TO_SHATTERP      = 35066
};

#define GOSSIP_ITEM1_LEAF       "Fly me to Shatter Point"

bool GossipHello_npc_gryphoneer_leafbeard(Player *player, Creature* pCreature)
{
    //Go back to Shatter Point if player has completed the quest 10340 - Shatter Point
    if (player->GetQuestStatus(10340) == QUEST_STATUS_COMPLETE)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM1_LEAF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_gryphoneer_leafbeard(Player *player, Creature* pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        //TaxiPath 609 (3.x.x)
        player->CastSpell(player,SPELL_TAXI_TO_SHATTERP,true);
    }
    return true;
}

/*######
## npc_wing_commander_brack
######*/

#define GOSSIP_ITEM1_BRA "Fly me to Murketh and Shaadraz Gateways"
#define GOSSIP_ITEM2_BRA "Fly me to The Abyssal Shelf"
#define GOSSIP_ITEM3_BRA "Fly me to Spinebreaker Post"

bool GossipHello_npc_wing_commander_brack(Player *player, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        player->PrepareQuestMenu(pCreature->GetGUID());

    //Mission: The Murketh and Shaadraz Gateways
    if (player->GetQuestStatus(10129) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM1_BRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Mission: The Abyssal Shelf || Return to the Abyssal Shelf
    if (player->GetQuestStatus(10162) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10347) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM2_BRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    //Spinebreaker Post
    if (player->GetQuestStatus(10242) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(10242))
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM3_BRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    player->SEND_GOSSIP_MENU(player->GetGossipTextId(pCreature), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_wing_commander_brack(Player *player, Creature* pCreature, uint32 sender, uint32 action)
{
    switch(action)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33659,true);               //TaxiPath 584 (Gateways Murket and Shaadraz)
        break;
    case GOSSIP_ACTION_INFO_DEF + 2:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33825,true);               //TaxiPath 587 (Aerial Assault Flight (Horde))
        break;
    case GOSSIP_ACTION_INFO_DEF + 3:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,34578,true);               //TaxiPath 604 (Taxi - Reaver's Fall to Spinebreaker Ridge)
        break;
    }
        return true;
}

/*######
## npc_wounded_blood_elf
######*/

enum eWoundedBloodElf
{
    SAY_ELF_START               = -1000117,
    SAY_ELF_SUMMON1             = -1000118,
    SAY_ELF_RESTING             = -1000119,
    SAY_ELF_SUMMON2             = -1000120,
    SAY_ELF_COMPLETE            = -1000121,
    SAY_ELF_AGGRO               = -1000122,

    QUEST_ROAD_TO_FALCON_WATCH  = 9375
};

struct npc_wounded_blood_elfAI : public npc_escortAI
{
    npc_wounded_blood_elfAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (i)
        {
        case 0:
            DoScriptText(SAY_ELF_START, me, pPlayer);
            break;
        case 9:
            DoScriptText(SAY_ELF_SUMMON1, me, pPlayer);
            // Spawn two Haal'eshi Talonguard
            DoSpawnCreature(16967, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            DoSpawnCreature(16967, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            break;
        case 13:
            DoScriptText(SAY_ELF_RESTING, me, pPlayer);
            break;
        case 14:
            DoScriptText(SAY_ELF_SUMMON2, me, pPlayer);
            // Spawn two Haal'eshi Windwalker
            DoSpawnCreature(16966, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            DoSpawnCreature(16966, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            break;
        case 27:
            DoScriptText(SAY_ELF_COMPLETE, me, pPlayer);
            // Award quest credit
            pPlayer->GroupEventHappens(QUEST_ROAD_TO_FALCON_WATCH,me);
            break;
        }
    }

    void Reset() { }

    void EnterCombat(Unit* who)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            DoScriptText(SAY_ELF_AGGRO, me);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(me);
    }
};

CreatureAI* GetAI_npc_wounded_blood_elf(Creature* pCreature)
{
    return new npc_wounded_blood_elfAI(pCreature);
}

bool QuestAccept_npc_wounded_blood_elf(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ROAD_TO_FALCON_WATCH)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_wounded_blood_elfAI, pCreature->AI()))
            pEscortAI->Start(true, false, pPlayer->GetGUID());

        // Change faction so mobs attack
        pCreature->setFaction(775);
    }

    return true;
}

/*######
## npc_hand_berserker
######*/

enum
{
    SPELL_SOUL_BURDEN       = 38879,
    SPELL_ENRAGE            = 8599,
    SPELL_SUMMON_SPIRIT     = 39206,
    SPELL_CHARGE            = 35570,

    NPC_BUNNY               = 22444,
    NPC_FEL_SPIRIT          = 22454,
    SAY_SP                  = -1900130
};

struct npc_hand_berserkerAI : public ScriptedAI
{
    npc_hand_berserkerAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    void Reset() {}

    void AttackStart(Unit *pWho)
    {
        DoMeleeAttackIfReady();
        ScriptedAI::AttackStart(pWho);
    }

    void EnterCombat(Unit* pWho)
    {
        if (rand()%60)
        {
            DoCast(pWho, SPELL_CHARGE);
        }
    }   

    void DamageTaken(Unit* pDoneby, uint32 & Damage)
    {
        if (me->HasAura(SPELL_ENRAGE, 0))
            return;

        if (pDoneby->GetTypeId() == TYPEID_PLAYER && (me->GetHealth()*100 - Damage) / me->GetMaxHealth() < 30)
        {
            DoCast(SPELL_ENRAGE);
        }
    }

    void JustDied(Unit* pWho)
    {
        if (Creature* Bunny = me->FindNearestCreature(NPC_BUNNY, 17.5f))
        {
            me->CastSpell(Bunny, SPELL_SOUL_BURDEN, false);
            Bunny->CastSpell(Bunny, SPELL_SUMMON_SPIRIT, false);
        }
        else return;

        if (Creature* Spirit = me->FindNearestCreature(NPC_FEL_SPIRIT, 18.5f))
        {
            DoScriptText(SAY_SP, Spirit);
            Spirit->AI()->AttackStart(pWho);
        }
        else return;
    }
};

CreatureAI* GetAI_npc_hand_berserker(Creature* pCreature)
{
    return new npc_hand_berserkerAI(pCreature);
}

/*######
## npc_anchorite_relic_bunny
######*/

enum
{
    NPC_HAND_BERSERKER      = 16878,
    SPELL_CHANNELS          = 39184,
    GO_RELIC                = 185298      
};

struct npc_anchorite_relic_bunnyAI : public ScriptedAI
{
    npc_anchorite_relic_bunnyAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    uint32 uiChTimer;
    uint32 uiEndTimer;

    void Reset()
    {
        uiChTimer = 5000;
        uiEndTimer = 60000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (uiEndTimer <= uiDiff)
        {
            if (GameObject* pRelic = me->FindNearestGameObject(GO_RELIC, 5.0f))
            {
                pRelic->RemoveFromWorld();
                me->setDeathState(CORPSE);
            }
            else return;

            uiEndTimer = 60000;
        }
        else uiEndTimer -= uiDiff;

        if (uiChTimer <= uiDiff)
        {
            if (Creature* pBer = me->FindNearestCreature(NPC_HAND_BERSERKER, 17.5f, true))
            {
                DoCast(pBer, SPELL_CHANNELS);
            }
            else me->InterruptNonMeleeSpells(false);

            uiChTimer = 5000;
        }
        else uiChTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_anchorite_relic_bunny(Creature* pCreature)
{
    return new npc_anchorite_relic_bunnyAI(pCreature);
}

/*######
## npc_anchorite_barada
######*/

#define GOSSIP_ITEM_START      "I am ready Amchorite.Let us begin the exorcim."
#define SAY_BARADA1            -1900100
#define SAY_BARADA2            -1900101
#define SAY_BARADA3            -1900104
#define SAY_BARADA4            -1900105
#define SAY_BARADA5            -1900106
#define SAY_BARADA6            -1900107
#define SAY_BARADA7            -1900108
#define SAY_BARADA8            -1900109
#define SAY_COLONEL1           -1900110
#define SAY_COLONEL2           -1900111
#define SAY_COLONEL3           -1900112
#define SAY_COLONEL4           -1900113
#define SAY_COLONEL5           -1900114
#define SAY_COLONEL6           -1900115
#define SAY_COLONEL7           -1900116
#define SAY_COLONEL8           -1900117

enum
{
    QUEST_THE_EXORCIM          = 10935,
    NPC_COLONEL_JULES          = 22432,
    NPC_DARKNESS_RELEASED      = 22507,

    SPELL_EXORCIM              = 39277,
    SPELL_EXORCIM2             = 39278,
    SPELL_COLONEL1             = 39283,         
    SPELL_COLONEL2             = 39294,
    SPELL_COLONEL3             = 39284,
    SPELL_COLONEL4             = 39294,
    SPELL_COLONEL5             = 39295,
    SPELL_COLONEL7             = 39381,
    SPELL_COLONEL8             = 39380
};

struct npc_anchorite_baradaAI : public ScriptedAI
{
    npc_anchorite_baradaAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    bool Exorcim;

    uint64 uiPlayerGUID;
    uint32 uiStepsTimer;
    uint32 uiSteps;

    float colonel;

    void Reset()
    {
        Exorcim = false;
        uiStepsTimer = 0;
        uiSteps = 0;
        uiPlayerGUID = 0;
    }

    void AttackedBy(Unit* pWho) {}
    void AttackStart(Unit* pWho) {}

    void DoSpawnDarkness()
    {
        me->SummonCreature(NPC_DARKNESS_RELEASED, -710.924, 2754.683, 105.0, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (pWho->GetTypeId() == TYPEID_PLAYER)
        {
            if (CAST_PLR(pWho)->GetQuestStatus(QUEST_THE_EXORCIM) == QUEST_STATUS_INCOMPLETE)
            {
                if (me->IsWithinDistInMap(((Player *)pWho), 5))
                {
                    uiPlayerGUID = pWho->GetGUID();
                }
            }
        }
    }

    uint32 NextStep(uint32 uiSteps)
    {
        Creature* pColonel = me->FindNearestCreature(NPC_COLONEL_JULES, 15);
        colonel = me->GetAngle(pColonel->GetPositionX(), pColonel->GetPositionY());

        switch(uiSteps)
        {
            case 1:me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                   pColonel->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                   me->SetStandState(UNIT_STAND_STATE_STAND);return 2000;
            case 2:DoScriptText(SAY_BARADA1, me,0);return 5000;
            case 3:DoScriptText(SAY_BARADA2, me,0);return 3000;
            case 4:DoScriptText(SAY_COLONEL1, pColonel, 0);return 3000;
            case 5:me->SetUnitMovementFlags(MOVEFLAG_WALK_MODE);return 3000;
            case 6:me->GetMotionMaster()->MovePoint(0, -707.702f, 2749.038f, 101.590f);return 2000;
            case 7:me->GetMotionMaster()->MovePoint(0, -710.810f, 2748.376f, 101.590f);return 2100;
            case 8:me->SetOrientation(colonel);
                   me->SendMovementFlagUpdate();return 2000;
            case 9:me->CastSpell(me, SPELL_EXORCIM , false);return 10000;
            case 10:DoScriptText(SAY_BARADA3, me,0); return 10000;
            case 11:DoScriptText(SAY_COLONEL2, pColonel, 0);return 8000;
            case 12:me->RemoveAllAuras();
            case 13:me->CastSpell(me, SPELL_EXORCIM2 , false);
            case 14:pColonel->CastSpell(pColonel, SPELL_COLONEL1, false);
            case 15:pColonel->SetUnitMovementFlags(MOVEFLAG_LEVITATING);
                    pColonel->SetSpeed(MOVE_RUN, 0.17f);
                    pColonel->GetMotionMaster()->MovePoint(0, -710.611f, 2753.435f, 103.774f);
                    pColonel->CastSpell(pColonel, SPELL_COLONEL3, false);return 14000;
            case 16:DoScriptText(SAY_COLONEL3, pColonel, 0);
                    DoSpawnDarkness();
                    DoSpawnDarkness();return 14000;
            case 17:DoScriptText(SAY_BARADA4, me, 0);
                    DoSpawnDarkness();
                    DoSpawnDarkness();return 14000;
            case 18:DoScriptText(SAY_COLONEL4, pColonel, 0);
                    DoSpawnDarkness();return 14000;
            case 19:DoScriptText(SAY_BARADA5, me, 0); return 14000;
            case 20:pColonel->CastSpell(pColonel, SPELL_COLONEL4, false);
                    pColonel->CastSpell(pColonel, SPELL_COLONEL2, false);
                    DoSpawnDarkness();return 1500;
            case 21:pColonel->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 103.774f);return 5000;
            case 22:DoScriptText(SAY_COLONEL5, pColonel, 0);return 1000;
            case 23:pColonel->GetMotionMaster()->MovePoint(0, -707.784f, 2749.562f, 103.774f);
                    DoSpawnDarkness();return 4000;
            case 24:pColonel->GetMotionMaster()->MovePoint(0, -708.558f, 2744.923f, 103.774f);
                    pColonel->CastSpell(me,SPELL_COLONEL5, false);return 2500;
            case 25:DoScriptText(SAY_BARADA6, me, 0);
            case 26:pColonel->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 103.774f);
                    DoSpawnDarkness();return 3500;
            case 27:pColonel->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 103.774f);return 4000;
            case 28:pColonel->GetMotionMaster()->MovePoint(0, -707.784f, 2749.562f, 103.774f);
                    DoScriptText(SAY_COLONEL6, pColonel, 0);
                    DoSpawnDarkness();return 4000;
            case 29:pColonel->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 103.774f);return 4000;
            case 30:pColonel->GetMotionMaster()->MovePoint(0, -707.784f, 2749.562f, 103.774f);return 4000;
            case 31: DoScriptText(SAY_BARADA7, me, 0); return 0;
            case 32:pColonel->GetMotionMaster()->MovePoint(0, -708.558f, 2744.923f, 103.774f);
                    DoSpawnDarkness();return 4000;
            case 33:pColonel->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 103.774f);return 4000;
            case 34:pColonel->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 103.774f);
                    DoScriptText(SAY_COLONEL7, pColonel, 0);
                    DoSpawnDarkness();return 4000;
            case 35:pColonel->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 103.774f);return 4000;
            case 36:pColonel->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 103.774f);
                    DoSpawnDarkness();return 4000;
            case 37:DoScriptText(SAY_BARADA6, me, 0);
            case 38:pColonel->GetMotionMaster()->MovePoint(0, -707.784f, 2749.562f, 103.774f);return 4000;
            case 39:pColonel->GetMotionMaster()->MovePoint(0, -708.558f, 2744.923f, 103.774f);return 4000;
            case 40:pColonel->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 103.774f);
                    DoScriptText(SAY_COLONEL8, pColonel, 0);return 4000;
            case 41:pColonel->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 103.774f);return 4000;
            case 42:pColonel->GetMotionMaster()->MovePoint(0, -707.784f, 2749.562f, 103.774f);return 4000;
            case 43:DoScriptText(SAY_BARADA6, me, 0); return 1000;
            case 44:pColonel->GetMotionMaster()->MovePoint(0, -708.558f, 2744.923f, 103.774f);return 4000;         
            case 45:pColonel->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 103.774f);
                    pColonel->CastSpell(pColonel, SPELL_COLONEL8, false);return 4000;
            case 46:pColonel->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 103.774f);
                    pColonel->CastSpell(pColonel, SPELL_COLONEL7, false);return 4000;
            case 47:pColonel->GetMotionMaster()->MovePoint(0, -710.792f, 2750.693f, 103.774f);return 5000;
            case 48:DoScriptText(SAY_BARADA8, me, 0); return 1000;
            case 49:pColonel->GetMotionMaster()->MovePoint(0, -710.111f, 2754.346f, 102.367f);return 3000;
            case 50:pColonel->RemoveAllAuras();
            case 51:me->RemoveAllAuras();return 2000;
            case 52:me->SetUnitMovementFlags(MOVEFLAG_WALK_MODE);return 2000;
            case 53:me->GetMotionMaster()->MovePoint(0, -706.726f, 2751.632f, 101.591f);return 2200;
            case 54:me->GetMotionMaster()->MovePoint(0, -707.382f, 2753.994f, 101.591f);return 7000;
            case 55:me->SetStandState(UNIT_STAND_STATE_KNEEL);
                    me->CombatStop();return 3000;
            case 56:pColonel->SetFlag(UNIT_NPC_FLAGS, 1);return 6000;
            case 57:me->SetFlag(UNIT_NPC_FLAGS, 1);
                    if (Player* pPlayer = Unit::GetPlayer(*me, uiPlayerGUID))
                        pPlayer->AreaExploredOrEventHappens(QUEST_THE_EXORCIM);return 1000;
            case 58:Reset();
        default: return 0;
        }
    }

    void JustDied(Unit* pWho)
    {
        if (Creature* pColonel = me->FindNearestCreature(NPC_COLONEL_JULES, 15.0f, true))
        {
            pColonel->GetMotionMaster()->MovePoint(0, -710.111f, 2754.346f, 102.367f);
            pColonel->RemoveAllAuras();
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (uiStepsTimer <= uiDiff)
        {
            if (Exorcim)
                uiStepsTimer = NextStep(++uiSteps);
        }
        else uiStepsTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_anchorite_barada(Creature* pCreature)
{
    return new npc_anchorite_baradaAI(pCreature);
}

bool GossipHello_npc_anchorite_barada(Player *pPlayer, Creature *pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_THE_EXORCIM) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_anchorite_barada(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        ((npc_anchorite_baradaAI*)pCreature->AI())->Exorcim = true;
        pPlayer->CLOSE_GOSSIP_MENU();
    }
    return true;
}

/*######
## npc_darkness_released
######*/

enum
{
    SPELL_AURA_ME     = 39303,
    SPELL_DARKNESS    = 39307,
    NPC_BARADA        = 22431
};

struct npc_darkness_releasedAI : public ScriptedAI
{
    npc_darkness_releasedAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    uint32 uiAtTimer;
    uint32 uiChTimer;

    void Reset()
    {
        uiChTimer = 5000;
        uiAtTimer = 10000;
        DoCast(SPELL_AURA_ME);
        me->SetUnitMovementFlags(MOVEFLAG_LEVITATING);
        me->SetSpeed(MOVE_RUN, 0.10f);
        switch(urand(0,3))
        {
            case 0: me->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 105.0f); break;
            case 1: me->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 105.0f); break;
            case 2: me->GetMotionMaster()->MovePoint(0, -707.784f, 2749.562f, 105.0f); break;
            case 3: me->GetMotionMaster()->MovePoint(0, -708.558f, 2744.923f, 105.0f); break;
        }
    }

    void AttackedBy(Unit* pWho) {}
    void AttackStart(Unit* pWho) {}

    void JustDied(Unit* pWho)
    {
        me->RemoveCorpse();
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (uiAtTimer <= uiDiff)
        {
            DoCast(SPELL_DARKNESS);
            switch (urand(0,3))
            {
                case 0: me->GetMotionMaster()->MovePoint(0, -714.212f, 2750.606f, 105.0f); break;
                case 1: me->GetMotionMaster()->MovePoint(0, -713.406f, 2744.240f, 105.0f); break;
                case 2: me->GetMotionMaster()->MovePoint(0, -707.784f, 2749.562f, 105.0f); break;
                case 3: me->GetMotionMaster()->MovePoint(0, -708.558f, 2744.923f, 105.0f); break;
            }

        uiAtTimer = 10000;
        }
        else uiAtTimer -= uiDiff;

        if (uiChTimer <= uiDiff)
        {
            if (Creature* pBar = me->FindNearestCreature(NPC_BARADA, 15.0f, false))
                me->setDeathState(CORPSE);

            if (Creature* pBara = me->FindNearestCreature(NPC_BARADA, 15.0f, true))
            {
                if (!pBara->HasAura(SPELL_EXORCIM2, 0))
                    me->setDeathState(CORPSE);
            }

            uiChTimer = 5000;
        }
        else uiChTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_darkness_released(Creature* pCreature)
{
    return new npc_darkness_releasedAI(pCreature);
}

/*######
## npc_foul_purge
######*/

struct npc_foul_purgeAI : public ScriptedAI
{
    npc_foul_purgeAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    uint32 uiChTimer;

    void Reset()
    {
        if (Creature* pBara = me->FindNearestCreature(NPC_BARADA, 15.0f, true))
        {
            AttackStart(pBara);
        }
        uiChTimer = 4000;
    }

    void JustDied(Unit* pWho)
    {
        me->RemoveCorpse();
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (uiChTimer <= uiDiff)
        {
            if (Creature* pBar = me->FindNearestCreature(NPC_BARADA, 15.0f, false))
                me->setDeathState(CORPSE);

            if (Creature* pBara = me->FindNearestCreature(NPC_BARADA, 15.0f, true))
            {
                if (!pBara->HasAura(SPELL_EXORCIM2, 0))
                    me->setDeathState(CORPSE);
            }

            uiChTimer = 4000;
        }
        else uiChTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_foul_purge(Creature* pCreature)
{
    return new npc_foul_purgeAI(pCreature);
}

void AddSC_hellfire_peninsula()
{
    Script *newscript;

    newscript = new Script;
     newscript->Name = "npc_aeranas";
    newscript->GetAI = &GetAI_npc_aeranas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ancestral_wolf";
    newscript->GetAI = &GetAI_npc_ancestral_wolf;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_haaleshi_altar";
    newscript->pGOHello = &GOHello_go_haaleshi_altar;
    newscript->RegisterSelf();


    newscript = new Script;
    newscript->Name = "npc_naladu";
    newscript->pGossipHello = &GossipHello_npc_naladu;
    newscript->pGossipSelect = &GossipSelect_npc_naladu;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tracy_proudwell";
    newscript->pGossipHello = &GossipHello_npc_tracy_proudwell;
    newscript->pGossipSelect = &GossipSelect_npc_tracy_proudwell;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_trollbane";
    newscript->pGossipHello = &GossipHello_npc_trollbane;
    newscript->pGossipSelect = &GossipSelect_npc_trollbane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_wing_commander_dabiree";
    newscript->pGossipHello =   &GossipHello_npc_wing_commander_dabiree;
    newscript->pGossipSelect =  &GossipSelect_npc_wing_commander_dabiree;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_gryphoneer_leafbeard";
    newscript->pGossipHello = &GossipHello_npc_gryphoneer_leafbeard;
    newscript->pGossipSelect = &GossipSelect_npc_gryphoneer_leafbeard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_wing_commander_brack";
    newscript->pGossipHello =   &GossipHello_npc_wing_commander_brack;
    newscript->pGossipSelect =  &GossipSelect_npc_wing_commander_brack;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_wounded_blood_elf";
    newscript->GetAI = &GetAI_npc_wounded_blood_elf;
    newscript->pQuestAccept = &QuestAccept_npc_wounded_blood_elf;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_hand_berserker";
    newscript->GetAI = &GetAI_npc_hand_berserker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_anchorite_relic_bunny";
    newscript->GetAI = &GetAI_npc_anchorite_relic_bunny;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_anchorite_barada";
    newscript->pGossipHello =  &GossipHello_npc_anchorite_barada;
    newscript->pGossipSelect = &GossipSelect_npc_anchorite_barada;
    newscript->GetAI = &GetAI_npc_anchorite_barada;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_darkness_released";
    newscript->GetAI = &GetAI_npc_darkness_released;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_foul_purge";
    newscript->GetAI = &GetAI_npc_foul_purge;
    newscript->RegisterSelf();
}
