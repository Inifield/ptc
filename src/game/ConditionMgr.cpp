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

#include "Player.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "GameEventMgr.h"
#include "ObjectMgr.h"
#include "InstanceData.h"
#include "ConditionMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"

INSTANTIATE_SINGLETON_1(ConditionMgr);

// Checks if object meets the condition
// Can have CONDITION_SOURCE_TYPE_NONE && !ReferenceId if called from a special event (ie: eventAI)
bool Condition::Meets(ConditionSourceInfo& sourceInfo)
{
    ASSERT(ConditionTarget < MAX_CONDITION_TARGETS);
    WorldObject* object = sourceInfo.mConditionTargets[ConditionTarget];
    // object not present, return false
    if (!object)
    {
        sLog.outDebug("Condition object not found for condition (Entry: %u Type: %u Group: %u)", SourceEntry, SourceType, SourceGroup);
        return false;                                   // player not present, return false
    }
    bool condMeets = false;
    bool sendErrorMsg = false;
    switch (Type)
    {
        case CONDITION_NONE:
            condMeets = true;                                    // empty condition, always met
            break;
        case CONDITION_AURA:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->HasAura(ConditionValue1, ConditionValue2);
            break;
        }
        case CONDITION_ITEM:
        {
            if (Player* player = object->ToPlayer())
            {
                // don't allow 0 items (it's checked during table load)
                ASSERT(ConditionValue2);
                bool checkBank = ConditionValue3 ? true : false;
                condMeets = player->HasItemCount(ConditionValue1, ConditionValue2, checkBank);
            }
            break;
        }
        case CONDITION_ITEM_EQUIPPED:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->GetItemOrItemWithGemEquipped(ConditionValue1);
            break;
        }
        case CONDITION_ZONEID:
            condMeets = object->GetZoneId() == ConditionValue1;
            break;
        case CONDITION_REPUTATION_RANK:
        {
            if (Player* player = object->ToPlayer())
            {
                if (FactionEntry const* faction = sFactionStore.LookupEntry(ConditionValue1))
                    condMeets = (ConditionValue2 & (1 << player->GetReputationRank(faction)));
            }
            break;
        }
        case CONDITION_TEAM:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->GetTeam() == ConditionValue1;
            break;
        }
        case CONDITION_CLASS:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->getClassMask() & ConditionValue1;
            break;
        }
        case CONDITION_RACE:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->getRaceMask() & ConditionValue1;
            break;
        }
        case CONDITION_GENDER:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->getGender() == ConditionValue1;
            break;
        }
        case CONDITION_SKILL:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasSkill(ConditionValue1) && player->GetBaseSkillValue(ConditionValue1) >= ConditionValue2;
            break;
        }
        case CONDITION_QUESTREWARDED:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->GetQuestRewardStatus(ConditionValue1);
            break;
        }
        case CONDITION_QUESTTAKEN:
        {
            if (Player* player = object->ToPlayer())
            {
                QuestStatus status = player->GetQuestStatus(ConditionValue1);
                condMeets = (status == QUEST_STATUS_INCOMPLETE);
            }
            break;
        }
        case CONDITION_QUEST_COMPLETE:
        {
            if (Player* player = object->ToPlayer())
            {
                QuestStatus status = player->GetQuestStatus(ConditionValue1);
                condMeets = (status == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(ConditionValue1));
            }
            break;
        }
        case CONDITION_QUEST_NONE:
        {
            if (Player* player = object->ToPlayer())
            {
                QuestStatus status = player->GetQuestStatus(ConditionValue1);
                condMeets = (status == QUEST_STATUS_NONE);
            }
            break;
        }
        case CONDITION_ACTIVE_EVENT:
            condMeets = sGameEventMgr.IsActiveEvent(ConditionValue1);
            break;
        case CONDITION_INSTANCE_INFO:
        {
            Map* map = object->GetMap();
            if (map && map->IsDungeon())
            {
                if (InstanceData* instance = ((InstanceMap*)map)->GetInstanceData())
                {
                    switch (ConditionValue3)
                    {
                        case INSTANCE_INFO_DATA:
                            condMeets = instance->GetData(ConditionValue1) == ConditionValue2;
                            break;
                        case INSTANCE_INFO_DATA64:
                            condMeets = instance->GetData64(ConditionValue1) == ConditionValue2;
                            break;
                        /* @todo Implement this feature
                        case INSTANCE_INFO_BOSS_STATE:
                            condMeets = instance->GetBossState(ConditionValue1) == ConditionValue2;
                            break;*/
                    }
                }
            }
            break;
        }
        case CONDITION_SPELL_SCRIPT_TARGET:
            condMeets = true; //spell target condition is handled in spellsystem, here it is always true
            break;
        case CONDITION_MAPID:
            condMeets = object->GetMapId() == ConditionValue1;
            break;
        case CONDITION_AREAID:
            condMeets = object->GetAreaId() == ConditionValue1;
            break;
        case CONDITION_SPELL:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasSpell(ConditionValue1);
            break;
        }
        case CONDITION_LEVEL:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue2), static_cast<uint32>(unit->getLevel()), ConditionValue1);
            break;
        }
        case CONDITION_DRUNKENSTATE:
        {
            if (Player* player = object->ToPlayer())
                condMeets = (uint32)Player::GetDrunkenstateByValue(player->GetDrunkValue()) >= ConditionValue1;
            break;
        }
        case CONDITION_NEAR_CREATURE:
        {
            condMeets = GetClosestCreatureWithEntry(object, ConditionValue1, (float)ConditionValue2) ? true : false;
            break;
        }
        case CONDITION_NEAR_GAMEOBJECT:
        {
            condMeets = GetClosestGameObjectWithEntry(object, ConditionValue1, (float)ConditionValue2) ? true : false;
            break;
        }
        case CONDITION_OBJECT_ENTRY_GUID:
        {
            if (object->GetTypeId() == ConditionValue1)
            {
                condMeets = !ConditionValue2 || (object->GetEntry() == ConditionValue2);

                if (ConditionValue3)
                {
                    switch (object->GetTypeId())
                    {
                        case TYPEID_UNIT:
                            condMeets &= object->ToCreature()->GetDBTableGUIDLow() == ConditionValue3;
                            break;
                        case TYPEID_GAMEOBJECT:
                            condMeets &= object->ToGameObject()->GetDBTableGUIDLow() == ConditionValue3;
                            break;
                    }
                }
            }  
            break;
        }
        case CONDITION_TYPE_MASK:
        {
            condMeets = object->isType(ConditionValue1);
            break;
        }
        case CONDITION_RELATION_TO:
        {
            if (WorldObject* toObject = sourceInfo.mConditionTargets[ConditionValue1])
            {
                Unit* toUnit = toObject->ToUnit();
                Unit* unit = object->ToUnit();
                if (toUnit && unit)
                {
                    switch (ConditionValue2)
                    {
                        case RELATION_SELF:
                            condMeets = unit == toUnit;
                            break;
                        case RELATION_IN_PARTY:
                            condMeets = unit->IsInPartyWith(toUnit);
                            break;
                        case RELATION_IN_RAID_OR_PARTY:
                            condMeets = unit->IsInRaidWith(toUnit);
                            break;
                        case RELATION_OWNED_BY:
                            condMeets = unit->GetOwnerGUID() == toUnit->GetGUID();
                            break;
                        case RELATION_CREATED_BY:
                            condMeets = unit->GetCreatorGUID() == toUnit->GetGUID();
                            break;
                    }
                }
            }
            break;
        }
        case CONDITION_REACTION_TO:
        {
            /* @todo Implement this feature
            if (WorldObject* toObject = sourceInfo.mConditionTargets[ConditionValue1])
            {
                Unit* toUnit = toObject->ToUnit();
                Unit* unit = object->ToUnit();
                if (toUnit && unit)
                    condMeets = (1 << unit->GetReactionTo(toUnit)) & ConditionValue2;
            }*/
            condMeets = false;
            break;
        }
        case CONDITION_DISTANCE_TO:
        {
            if (WorldObject* toObject = sourceInfo.mConditionTargets[ConditionValue1])
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue3), object->GetDistance(toObject), static_cast<float>(ConditionValue2));
            break;
        }
        case CONDITION_ALIVE:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->IsAlive();
            break;
        }
        case CONDITION_HP_VAL:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue2), unit->GetHealth(), static_cast<uint32>(ConditionValue1));
            break;
        }
        case CONDITION_HP_PCT:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = CompareValues(static_cast<ComparisionType>(ConditionValue2), unit->GetHealthPct(), static_cast<float>(ConditionValue1));
            break;
        }
        case CONDITION_WORLD_STATE:
        {
            /* @todo Implement this feature
            condMeets = ConditionValue2 == sWorld.getWorldState(ConditionValue1);*/
            condMeets = false;
            break;
        }
        case CONDITION_TITLE:
        {
            if (Player* player = object->ToPlayer())
                condMeets = player->HasTitle(ConditionValue1);
            break;
        }
        case CONDITION_SPAWNMASK:
        {
            condMeets = ((1 << object->GetMap()->GetSpawnMode()) & ConditionValue1);
            break;
        }
        case CONDITION_UNIT_STATE:
        {
            if (Unit* unit = object->ToUnit())
                condMeets = unit->HasUnitState(ConditionValue1);
            break;
        }
        case CONDITION_CREATURE_TYPE:
        {
            if (Creature* creature = object->ToCreature())
                condMeets = creature->GetCreatureTemplate()->type == ConditionValue1;
            break;
        }
        default:
            condMeets = false;
            break;
    }

    if (NegativeCondition)
        condMeets = !condMeets;

    if (!condMeets)
        sourceInfo.mLastFailedCondition = this;

    /* @ConditionTodo
    bool script = sScriptMgr->OnConditionCheck(this, sourceInfo); // Returns true by default.
    return condMeets && script;
    */

    return condMeets;
}

uint32 Condition::GetMaxAvailableConditionTargets()
{
    // returns number of targets which are available for given source type
    switch(SourceType)
    {
        case CONDITION_SOURCE_TYPE_SPELL:
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU:
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION:
            return 2;
        default:
            return 1;
    }
}

ConditionMgr::ConditionMgr()
{
}

ConditionMgr::~ConditionMgr()
{
    Clean();
}

ConditionList ConditionMgr::GetConditionReferences(uint32 refId)
{
    ConditionList conditions;
    ConditionReferenceContainer::const_iterator ref = ConditionReferenceStore.find(refId);
    if (ref != ConditionReferenceStore.end())
        conditions = (*ref).second;
    return conditions;
}

bool ConditionMgr::IsObjectMeetToConditionList(ConditionSourceInfo& sourceInfo, ConditionList const& conditions)
{
    //     groupId, groupCheckPassed
    std::map<uint32, bool> ElseGroupStore;
    for (ConditionList::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
    {
        sLog.outDebug("ConditionMgr::IsPlayerMeetToConditionList condType: %u val1: %u",(*i)->Type,(*i)->ConditionValue1);
        if ((*i)->isLoaded())
        {
            //! Find ElseGroup in ElseGroupStore
            std::map<uint32, bool>::const_iterator itr = ElseGroupStore.find((*i)->ElseGroup);
            //! If not found, add an entry in the store and set to true (placeholder)
            if (itr == ElseGroupStore.end())
                ElseGroupStore[(*i)->ElseGroup] = true;
            else if (!(*itr).second)
                continue;

            if ((*i)->ReferenceId)//handle reference
            {
                ConditionReferenceContainer::const_iterator ref = ConditionReferenceStore.find((*i)->ReferenceId);
                if (ref != ConditionReferenceStore.end())
                {
                    if (!IsObjectMeetToConditionList(sourceInfo, (*ref).second))
                        ElseGroupStore[(*i)->ElseGroup] = false;
                }
                else
                {
                    sLog.outDebug("IsPlayerMeetToConditionList: Reference template -%u not found",
                        (*i)->ReferenceId);//checked at loading, should never happen
                }
            }
            else //handle normal condition
            {
                if (!(*i)->Meets(sourceInfo))
                    ElseGroupStore[(*i)->ElseGroup] = false;
            }            
        }
    }
    for (std::map<uint32, bool>::const_iterator i = ElseGroupStore.begin(); i != ElseGroupStore.end(); ++i)
        if (i->second)
            return true;

    return false;
}

bool ConditionMgr::IsObjectMeetToConditions(WorldObject* object, ConditionList const& conditions)
{
    ConditionSourceInfo info(object);
    return IsObjectMeetToConditions(info, conditions);
}

bool ConditionMgr::IsObjectMeetToConditions(WorldObject* object1, WorldObject* object2, ConditionList const& conditions)
{
    ConditionSourceInfo srcInfo = ConditionSourceInfo(object1, object2);
    return IsObjectMeetToConditions(srcInfo, conditions);
}

bool ConditionMgr::IsObjectMeetToConditions(ConditionSourceInfo& sourceInfo, ConditionList const& conditions)
{
    if (conditions.empty())
        return true;

    sLog.outDebug("ConditionMgr::IsObjectMeetToConditions");
    return IsObjectMeetToConditionList(sourceInfo, conditions);
}

ConditionList ConditionMgr::GetConditionsForNotGroupedEntry(ConditionSourceType sourceType, uint32 entry)
{
    ConditionList spellCond;
    if (sourceType > CONDITION_SOURCE_TYPE_NONE && sourceType < CONDITION_SOURCE_TYPE_MAX)
    {
        ConditionContainer::const_iterator itr = ConditionStore.find(sourceType);
        if (itr != ConditionStore.end())
        {
            ConditionTypeContainer::const_iterator i = (*itr).second.find(entry);
            if (i != (*itr).second.end())
            {
                spellCond = (*i).second;
                sLog.outDebug("GetConditionsForNotGroupedEntry: found conditions for type %u and entry %u", uint32(sourceType), entry);
            }
        }
    }
    return spellCond;
}

void ConditionMgr::LoadConditions(bool isReload)
{
    Clean();

    //must clear all custom handled cases (groupped types) before reload
    if (isReload)
    {
        sLog.outString("Reseting Loot Conditions...");
        LootTemplates_Creature.ResetConditions();
        LootTemplates_Fishing.ResetConditions();
        LootTemplates_Gameobject.ResetConditions();
        LootTemplates_Item.ResetConditions();
        LootTemplates_Mail.ResetConditions();
        LootTemplates_Pickpocketing.ResetConditions();
        LootTemplates_Reference.ResetConditions();
        LootTemplates_Skinning.ResetConditions();
        LootTemplates_Disenchant.ResetConditions();
        LootTemplates_Prospecting.ResetConditions();

        sLog.outString("Re-Loading `gossip_menu` Table for Conditions!");
        sObjectMgr.LoadGossipMenu();

        sLog.outString("Re-Loading `gossip_menu_option` Table for Conditions!");
        sObjectMgr.LoadGossipMenuItems();
    }

    uint32 count = 0;
    QueryResult_AutoPtr result = WorldDatabase.Query("SELECT SourceTypeOrReferenceId, SourceGroup, SourceEntry, ElseGroup, ConditionTypeOrReference, ConditionTarget, ConditionValue1, ConditionValue2, ConditionValue3, NegativeCondition, ErrorType, ErrorTextId, ScriptName FROM conditions");

    if (!result)
    {
        sLog.outString();
        sLog.outErrorDb(">> Loaded `conditions`, table is empty!");
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        Condition* cond = new Condition();
        int32 iSourceTypeOrReferenceId  = fields[0].GetInt32();
        cond->SourceGroup               = fields[1].GetUInt32();
        cond->SourceEntry               = fields[2].GetUInt32();
        cond->ElseGroup                 = fields[3].GetUInt32();
        int32 iConditionTypeOrReference = fields[4].GetInt32();
        cond->ConditionTarget           = fields[5].GetUInt8();
        cond->ConditionValue1           = fields[6].GetUInt32();
        cond->ConditionValue2           = fields[7].GetUInt32();
        cond->ConditionValue3           = fields[8].GetUInt32();
        cond->NegativeCondition         = fields[9].GetUInt8();
        cond->ErrorType                 = fields[10].GetUInt32();
        cond->ErrorTextId               = fields[11].GetUInt32();
        cond->ScriptId                  = sObjectMgr.GetScriptId(fields[12].GetString());

        if (iConditionTypeOrReference >= 0)
            cond->Type = ConditionType(iConditionTypeOrReference);

        if (iSourceTypeOrReferenceId >= 0)
            cond->SourceType = ConditionSourceType(iSourceTypeOrReferenceId);

        if (iConditionTypeOrReference < 0)//it has a reference
        {
            if (iConditionTypeOrReference == iSourceTypeOrReferenceId)//self referencing, skipp
            {
                sLog.outErrorDb("Condition reference %i is referencing self, skipped", iSourceTypeOrReferenceId);
                delete cond;
                continue;
            }
            cond->ReferenceId = uint32(abs(iConditionTypeOrReference));

            const char* rowType = "reference template";
            if (iSourceTypeOrReferenceId >= 0)
                rowType = "reference";
            //check for useless data
            if (cond->ConditionTarget)
                sLog.outErrorDb("Condition %s %i has useless data in ConditionTarget (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionTarget);
            if (cond->ConditionValue1)
                sLog.outErrorDb("Condition %s %i has useless data in value1 (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionValue1);
            if (cond->ConditionValue2)
                sLog.outErrorDb("Condition %s %i has useless data in value2 (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionValue2);
            if (cond->ConditionValue3)
                sLog.outErrorDb("Condition %s %i has useless data in value3 (%u)!", rowType, iSourceTypeOrReferenceId, cond->ConditionValue3);
            if (cond->NegativeCondition)
                sLog.outErrorDb("Condition %s %i has useless data in NegativeCondition (%u)!", rowType, iSourceTypeOrReferenceId, cond->NegativeCondition);
            if (cond->SourceGroup && iSourceTypeOrReferenceId < 0)
                sLog.outErrorDb("Condition %s %i has useless data in SourceGroup (%u)!", rowType, iSourceTypeOrReferenceId, cond->SourceGroup);
            if (cond->SourceEntry && iSourceTypeOrReferenceId < 0)
                sLog.outErrorDb("Condition %s %i has useless data in SourceEntry (%u)!", rowType, iSourceTypeOrReferenceId, cond->SourceEntry);
        }
        else if (!isConditionTypeValid(cond))//doesn't have reference, validate ConditionType
        {
            delete cond;
            continue;
        }

        if (iSourceTypeOrReferenceId < 0)//it is a reference template
        {
            uint32 uRefId = abs(iSourceTypeOrReferenceId);
            if (ConditionReferenceStore.find(uRefId) == ConditionReferenceStore.end())//make sure we have a list for our conditions, based on reference id
            {
                ConditionList mCondList;
                ConditionReferenceStore[uRefId] = mCondList;
            }
            ConditionReferenceStore[uRefId].push_back(cond);//add to reference storage
            count++;
            continue;
        }//end of reference templates

        //if not a reference and SourceType is invalid, skip
        if (iConditionTypeOrReference >= 0 && !isSourceTypeValid(cond))
        {
            delete cond;
            continue;
        }

        if (cond->ErrorType && cond->SourceType != CONDITION_SOURCE_TYPE_SPELL)
        {
            sLog.outError("Condition type %u entry %i can't have ErrorType (%u), set to 0!", uint32(cond->SourceType), cond->SourceEntry, cond->ErrorType);
            cond->ErrorType = 0;
        }

        if (cond->ErrorTextId && !cond->ErrorType)
        {
            sLog.outError("Condition type %u entry %i has any ErrorType, ErrorTextId (%u) is set, set to 0!", uint32(cond->SourceType), cond->SourceEntry, cond->ErrorTextId);
            cond->ErrorTextId = 0;
        }

        //Grouping is only allowed for some types (loot templates, gossip menus, gossip items)
        if (cond->SourceGroup && !isGroupable(cond->SourceType))
        {
            sLog.outErrorDb("Condition type %u has not allowed grouping %u!", uint32(cond->SourceType), cond->SourceGroup);
            delete cond;
            continue;
        }
        else if (cond->SourceGroup)
        {
            bool valid = false;
            // handle grouped conditions
            switch (cond->SourceType)
            {
                case CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Creature.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Disenchant.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Fishing.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Gameobject.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Item.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Mail.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Pickpocketing.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Prospecting.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Reference.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE:
                    valid = addToLootTemplate(cond, LootTemplates_Skinning.GetLootForConditionFill(cond->SourceGroup));
                    break;
                case CONDITION_SOURCE_TYPE_GOSSIP_MENU:
                    valid = addToGossipMenus(cond);
                    break;
                case CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION:
                    valid = addToGossipMenuItems(cond);
                    break;
            }

            if (!valid)
            {
                sLog.outErrorDb("Not handled grouped condition, SourceGroup %u", cond->SourceGroup);
                delete cond;
            }
            else
            {
                AllocatedMemoryStore.push_back(cond);
                ++count;
            }
            continue;
        }

        //handle not grouped conditions
        //make sure we have a storage list for our SourceType
        if (ConditionStore.find(cond->SourceType) == ConditionStore.end())
        {
            ConditionTypeContainer mTypeMap;
            ConditionStore[cond->SourceType] = mTypeMap;//add new empty list for SourceType
        }

        //make sure we have a condition list for our SourceType's entry
        if (ConditionStore[cond->SourceType].find(cond->SourceEntry) == ConditionStore[cond->SourceType].end())
        {
            ConditionList mCondList;
            ConditionStore[cond->SourceType][cond->SourceEntry] = mCondList;
        }

        //add new Condition to storage based on Type/Entry
        ConditionStore[cond->SourceType][cond->SourceEntry].push_back(cond);
        ++count;
    }
    while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u conditions", count);
}

bool ConditionMgr::addToLootTemplate(Condition* cond, LootTemplate* loot)
{
    if (!loot)
    {
        sLog.outErrorDb("ConditionMgr: LootTemplate %u not found", cond->SourceGroup);
        return false;
    }

    if (loot->addConditionItem(cond))
        return true;

    sLog.outErrorDb("ConditionMgr: Item %u not found in LootTemplate %u", cond->SourceEntry, cond->SourceGroup);
    return false;
}

bool ConditionMgr::addToGossipMenus(Condition* cond)
{
    GossipMenusMapBoundsNonConst pMenuBounds = sObjectMgr.GetGossipMenusMapBoundsNonConst(cond->SourceGroup);

    if (pMenuBounds.first != pMenuBounds.second)
    {
        for (GossipMenusMap::iterator itr = pMenuBounds.first; itr != pMenuBounds.second; ++itr)
        {
            if ((*itr).second.entry == cond->SourceGroup && (*itr).second.text_id == cond->SourceEntry)
            {
                (*itr).second.conditions.push_back(cond);
                return true;
            }
        }
    }

    sLog.outErrorDb("addToGossipMenus: GossipMenu %u not found", cond->SourceGroup);
    return false;
}

bool ConditionMgr::addToGossipMenuItems(Condition* cond)
{
    GossipMenuItemsMapBoundsNonConst pMenuItemBounds = sObjectMgr.GetGossipMenuItemsMapBoundsNonConst(cond->SourceGroup);
    if (pMenuItemBounds.first != pMenuItemBounds.second)
    {
        for (GossipMenuItemsMap::iterator itr = pMenuItemBounds.first; itr != pMenuItemBounds.second; ++itr)
        {
            if ((*itr).second.menu_id == cond->SourceGroup && (*itr).second.id == cond->SourceEntry)
            {
                (*itr).second.conditions.push_back(cond);
                return true;
            }
        }
    }

    sLog.outErrorDb("addToGossipMenuItems: GossipMenuIt %u Item %u not found", cond->SourceGroup, cond->SourceEntry);
    return false;
}

bool ConditionMgr::isSourceTypeValid(Condition* cond)
{
    if (cond->SourceType == CONDITION_SOURCE_TYPE_NONE || cond->SourceType >= CONDITION_SOURCE_TYPE_MAX)
    {
        sLog.outErrorDb("Invalid ConditionSourceType %u in `conditions` table, ignoring.", uint32(cond->SourceType));
        return false;
    }

    switch (cond->SourceType)
    {
        case CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Creature.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `creature_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }
            LootTemplate* loot = LootTemplates_Creature.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Disenchant.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `disenchant_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Disenchant.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Fishing.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `fishing_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Fishing.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Gameobject.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `gameobject_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Gameobject.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Item.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `item_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Item.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Mail.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `mail_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Mail.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Pickpocketing.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `pickpocketing_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Pickpocketing.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Prospecting.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `prospecting_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Prospecting.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Reference.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `reference_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Reference.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE:
        {
            if (!LootTemplates_Skinning.HaveLootFor(cond->SourceGroup))
            {
                sLog.outErrorDb("SourceGroup %u in `condition` table, does not exist in `skinning_loot_template`, ignoring.", cond->SourceGroup);
                return false;
            }

            LootTemplate* loot = LootTemplates_Skinning.GetLootForConditionFill(cond->SourceGroup);
            ItemPrototype const* pItemProto = sItemStorage.LookupEntry<ItemPrototype>(cond->SourceEntry);
            if (!pItemProto && !loot->isReference(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, does not exist in `item_template`, ignoring.", cond->SourceType, cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_SPELL_SCRIPT_TARGET:
        {
            if (cond->Type != CONDITION_SPELL_SCRIPT_TARGET)
            {
                sLog.outErrorDb("SourceEntry %u in `condition` table, has ConditionType %u. Only CONDITION_SPELL_SCRIPT_TARGET(17) is valid for CONDITION_SOURCE_TYPE_SPELL_SCRIPT_TARGET(14), ignoring.", cond->SourceEntry, uint32(cond->Type));
                return false;
            }

            SpellEntry const* spellProto = sSpellStore.LookupEntry(cond->SourceEntry);
            if (!spellProto)
            {
                sLog.outErrorDb("SourceEntry %u in `condition` table, does not exist in `spell.dbc`, ignoring.", cond->SourceEntry);
                return false;
            }

            bool targetfound = false;
            for (uint8 i = 0; i < 3; ++i)
            {
                if (spellProto->EffectImplicitTargetA[i] == TARGET_UNIT_AREA_ENTRY_SRC ||
                    spellProto->EffectImplicitTargetB[i] == TARGET_UNIT_AREA_ENTRY_SRC ||
                    spellProto->EffectImplicitTargetA[i] == TARGET_UNIT_AREA_ENTRY_DST ||
                    spellProto->EffectImplicitTargetB[i] == TARGET_UNIT_AREA_ENTRY_DST ||
                    spellProto->EffectImplicitTargetA[i] == TARGET_UNIT_NEARBY_ENTRY ||
                    spellProto->EffectImplicitTargetB[i] == TARGET_UNIT_NEARBY_ENTRY ||
                    spellProto->EffectImplicitTargetA[i] == TARGET_GAMEOBJECT_NEARBY_ENTRY ||
                    spellProto->EffectImplicitTargetB[i] == TARGET_GAMEOBJECT_NEARBY_ENTRY ||
                    spellProto->EffectImplicitTargetA[i] == TARGET_DST_NEARBY_ENTRY ||
                    spellProto->EffectImplicitTargetB[i] == TARGET_DST_NEARBY_ENTRY ||
                    spellProto->EffectImplicitTargetA[i] == TARGET_UNIT_CONE_ENTRY ||
                    spellProto->EffectImplicitTargetB[i] == TARGET_UNIT_CONE_ENTRY)
                {
                    targetfound = true;
                    break;
                }
            }
            if (!targetfound)
            {
                sLog.outErrorDb("SourceEntry %u in `condition` table does not have any implicit target TARGET_UNIT_NEARBY_ENTRY(38) or TARGET_DST_NEARBY_ENTRY (46)\
                                ,TARGET_UNIT_AREA_ENTRY_SRC(7), TARGET_UNIT_AREA_ENTRY_DST(8), TARGET_UNIT_CONE_ENTRY(60), TARGET_GAMEOBJECT_NEARBY_ENTRY(40)",cond->SourceEntry);
                return false;
            }
            break;
        }          
        case CONDITION_SOURCE_TYPE_CREATURE_TEMPLATE_VEHICLE:
        {
            if (!sCreatureStorage.LookupEntry<CreatureInfo>(cond->SourceEntry))
            {
                sLog.outErrorDb("SourceEntry %u in `condition` table, does not exist in `creature_template`, ignoring.", cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_SPELL:
        {
            SpellEntry const* spellProto = sSpellStore.LookupEntry(cond->SourceEntry);
            if (!spellProto)
            {
                sLog.outErrorDb("SourceEntry %u in `condition` table, does not exist in `spell.dbc`, ignoring.", cond->SourceEntry);
                return false;
            }
            break;
        }
        case CONDITION_SOURCE_TYPE_QUEST_ACCEPT:
            {
                Quest const *Quest = sObjectMgr.GetQuestTemplate(cond->SourceEntry);
                if (!Quest)
                {
                    sLog.outErrorDb("CONDITION_SOURCE_TYPE_QUEST_ACCEPT specifies non-existing quest (%u), skipped", cond->SourceEntry);
                    return false;
                }
            }
            break;
        case CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK:
            {
                Quest const *Quest = sObjectMgr.GetQuestTemplate(cond->SourceEntry);
                if (!Quest)
                {
                    sLog.outErrorDb("CONDITION_SOURCE_TYPE_QUEST_SHOW_MARK specifies non-existing quest (%u), skipped", cond->SourceEntry);
                    return false;
                }
            }
            break;
        case CONDITION_SOURCE_TYPE_UNUSED_18:
            sLog.outErrorDb("Found SourceTypeOrReferenceId = CONDITION_SOURCE_TYPE_UNUSED_18 in `condition` table - ignoring");
            return false;
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU:
        case CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION:
        case CONDITION_SOURCE_TYPE_NONE:
        default:
            break;
    }

    return true;
}
bool ConditionMgr::isConditionTypeValid(Condition* cond)
{
    if (cond->Type == CONDITION_NONE || cond->Type >= CONDITION_MAX)
    {
        sLog.outErrorDb("Invalid ConditionType %u at SourceEntry %u in `condition` table, ignoring.", uint32(cond->Type),cond->SourceEntry);
        return false;
    }

    if (cond->ConditionTarget >= cond->GetMaxAvailableConditionTargets())
    {
        sLog.outErrorDb("SourceType %u, SourceEntry %u in `condition` table, has incorrect ConditionTarget set, ignoring.", cond->SourceType, cond->SourceEntry);
        return false;
    }

    switch (cond->Type)
    {
        case CONDITION_AURA:
        {
            if (!sSpellStore.LookupEntry(cond->ConditionValue1))
            {
                sLog.outErrorDb("Aura condition has non existing spell (Id: %d), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2 > 2)
            {
                sLog.outErrorDb("Aura condition has non existing effect index (%u) (must be 0..2), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                sLog.outErrorDb("Aura condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_ITEM:
        {
            ItemPrototype const *proto = sObjectMgr.GetItemPrototype(cond->ConditionValue1);
            if (!proto)
            {
                sLog.outErrorDb("Item condition has non existing item (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (!cond->ConditionValue2)
            {
                sLog.outErrorDb("Item condition has 0 set for item count in value2 (%u), skipped", cond->ConditionValue2);
                return false;
            }
            break;
        }
        case CONDITION_ITEM_EQUIPPED:
        {
            ItemPrototype const *proto = sObjectMgr.GetItemPrototype(cond->ConditionValue1);
            if (!proto)
            {
                sLog.outErrorDb("ItemEquipped condition has non existing item (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("ItemEquipped condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_ZONEID:
        {
            AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(cond->ConditionValue1);
            if (!areaEntry)
            {
                sLog.outErrorDb("Zone condition has non existing area (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (areaEntry->zone != 0)
            {
                sLog.outErrorDb("Zone condition requires to be in area (%u) which is a subzone but zone expected, skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("Zone condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_REPUTATION_RANK:
        {
            FactionEntry const* factionEntry = sFactionStore.LookupEntry(cond->ConditionValue1);
            if (!factionEntry)
            {
                sLog.outErrorDb("Reputation condition has non existing faction (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_TEAM:
        {
            if (cond->ConditionValue1 != ALLIANCE && cond->ConditionValue1 != HORDE)
            {
                sLog.outErrorDb("Team condition specifies unknown team (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("Team condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_SKILL:
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(cond->ConditionValue1);
            if (!pSkill)
            {
                sLog.outErrorDb("Skill condition specifies non-existing skill (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2 < 1 || cond->ConditionValue2 > sWorld.GetConfigMaxSkillValue())
            {
                sLog.outErrorDb("Skill condition specifies invalid skill value (%u), skipped", cond->ConditionValue2);
                return false;
            }
            break;
        }
        case CONDITION_QUESTREWARDED:
        case CONDITION_QUESTTAKEN:
        case CONDITION_QUEST_NONE:
        case CONDITION_QUEST_COMPLETE:
        {
            Quest const *Quest = sObjectMgr.GetQuestTemplate(cond->ConditionValue1);
            if (!Quest)
            {
                sLog.outErrorDb("Quest condition specifies non-existing quest (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2 > 1)
                sLog.outErrorDb("Quest condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_ACTIVE_EVENT:
        {
            GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();
            if (cond->ConditionValue1 >=events.size() || !events[cond->ConditionValue1].isValid())
            {
                sLog.outErrorDb("Active event condition has non existing event id (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("Active event condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_CLASS:
        {
            if (!(cond->ConditionValue1 & CLASSMASK_ALL_PLAYABLE))
            {
                sLog.outErrorDb("Class condition has non existing classmask (%u), skipped", cond->ConditionValue1 & ~CLASSMASK_ALL_PLAYABLE);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("Class condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_RACE:
        {
            if (!(cond->ConditionValue1 & RACEMASK_ALL_PLAYABLE))
            {
                sLog.outErrorDb("Race condition has non existing racemask (%u), skipped", cond->ConditionValue1 & ~RACEMASK_ALL_PLAYABLE);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("Race condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_SPELL_SCRIPT_TARGET:
        {
            if (cond->ConditionValue1 >= MAX_SPELL_TARGET_TYPE)
            {
                sLog.outErrorDb("SpellTarget condition has non existing spell target type (%u), skipped", cond->ConditionValue1);
                return false;
            }

            switch(cond->ConditionValue1)
            {
                case SPELL_TARGET_TYPE_GAMEOBJECT:
                {
                    if (cond->ConditionValue2 && !sGOStorage.LookupEntry<GameObjectInfo>(cond->ConditionValue2))
                    {
                        sLog.outErrorDb("SpellTarget condition has non existing gameobject (%u) as target, skipped", cond->ConditionValue2);
                        return false;
                    }
                    break;
                }
                case SPELL_TARGET_TYPE_CREATURE:
                case SPELL_TARGET_TYPE_DEAD:
                {
                    if (cond->ConditionValue2 && !sCreatureStorage.LookupEntry<CreatureInfo>(cond->ConditionValue2))
                    {
                        sLog.outErrorDb("SpellTarget condition has non existing creature template entry (%u) as target, skipped", cond->ConditionValue2);
                        return false;
                    }

                    const CreatureInfo* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(cond->ConditionValue2);
                    if (cond->SourceEntry == 30427 && !cInfo->SkinLootId)
                    {
                        sLog.outErrorDb("SpellTarget condition has creature entry %u as a target of spellid 30427, but this creature has no skinlootid. Gas extraction will not work!, skipped", cond->ConditionValue2);
                        return false;
                    }
                    break;
                }
            }

            if (cond->ConditionValue3)
                sLog.outErrorDb("SpellTarget condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_TITLE:
        {
            CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(cond->ConditionValue1);
            if (!titleEntry)
            {
                sLog.outErrorDb("Title condition has non existing title in value1 (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_SPAWNMASK:
        {
            if (cond->ConditionValue1 >= 2)
            {
                sLog.outError("Map Difficulty condition has non existing map difficulty in value1 (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_GENDER:
        {
            /* ConditionTodo
            if (!Player::IsValidGender(uint8(cond->ConditionValue1)))
            {
                sLog.outError("Gender condition has invalid gender (%u), skipped", cond->ConditionValue1);
                return false;
            }*/

            if (cond->ConditionValue2)
                sLog.outError("Gender condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                sLog.outError("Gender condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_UNIT_STATE:
        {
            if (cond->ConditionValue1 > uint32(UNIT_STATE_ALL_STATE))
            {
                sLog.outError("UnitState condition has non existing UnitState in value1 (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_MAPID:
        {
            MapEntry const * me = sMapStore.LookupEntry(cond->ConditionValue1);
            if (!me)
            {
                sLog.outErrorDb("Map condition has non existing map (%u), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("Map condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_CREATURE_TYPE:
        {
            if (!cond->ConditionValue1 || cond->ConditionValue1 > CREATURE_TYPE_GAS_CLOUD)
            {
                sLog.outErrorDb("CreatureType condition has non existing CreatureType in value1 (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_SPELL:
        {
            if (!sSpellStore.LookupEntry(cond->ConditionValue1))
            {
                sLog.outErrorDb("Spell condition has non existing spell (Id: %d), skipped", cond->ConditionValue1);
                return false;
            }

            if (cond->ConditionValue2)
                sLog.outErrorDb("Spell condition has useless data in value2 (%u)!", cond->ConditionValue2);
            break;
        }
        case CONDITION_LEVEL:
        {
            if (cond->ConditionValue2 >= COMP_TYPE_MAX)
            {
                sLog.outErrorDb("Level condition has invalid option (%u), skipped", cond->ConditionValue2);
                return false;
            }
            break;
        }
        case CONDITION_DRUNKENSTATE:
            {
                if (cond->ConditionValue1 > DRUNKEN_SMASHED)
                {
                    sLog.outErrorDb("DrunkState condition has invalid state (%u), skipped", cond->ConditionValue1);
                    return false;
                }
                break;
            }
        case CONDITION_NEAR_CREATURE:
        {
            if (!sCreatureStorage.LookupEntry<CreatureInfo>(cond->ConditionValue1))
            {
                sLog.outErrorDb("NearCreature condition has non existing creature template entry (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_NEAR_GAMEOBJECT:
        {
            if (!sGOStorage.LookupEntry<GameObjectInfo>(cond->ConditionValue1))
            {
                sLog.outErrorDb("NearGameObject condition has non existing gameobject template entry (%u), skipped", cond->ConditionValue1);
                return false;
            }
            break;
        }
        case CONDITION_OBJECT_ENTRY_GUID:
        {
            switch (cond->ConditionValue1)
            {
                case TYPEID_UNIT:
                    if (cond->ConditionValue2 && !sObjectMgr.GetCreatureTemplate(cond->ConditionValue2))
                    {
                        sLog.outErrorDb("ObjectEntryGuid condition has non existing creature template entry  (%u), skipped", cond->ConditionValue2);
                        return false;
                    }
                    if (cond->ConditionValue3)
                    {
                        if (CreatureData const* creatureData = sObjectMgr.GetCreatureData(cond->ConditionValue3))
                        {
                            if (cond->ConditionValue2 && creatureData->id != cond->ConditionValue2)
                            {
                                sLog.outErrorDb("ObjectEntryGuid condition has guid %u set but does not match creature entry (%u), skipped", cond->ConditionValue3, cond->ConditionValue2);
                                return false;
                            }
                        }
                        else
                        {
                            sLog.outErrorDb("ObjectEntryGuid condition has non existing creature guid (%u), skipped", cond->ConditionValue3);
                            return false;
                        }
                    }
                    break;
                case TYPEID_GAMEOBJECT:
                    if (cond->ConditionValue2 && !sObjectMgr.GetGameObjectInfo(cond->ConditionValue2))
                    {
                        sLog.outErrorDb("ObjectEntryGuid condition has non existing game object template entry  (%u), skipped", cond->ConditionValue2);
                        return false;
                    }
                    if (cond->ConditionValue3)
                    {
                        if (GameObjectData const* goData = sObjectMgr.GetGOData(cond->ConditionValue3))
                        {
                            if (cond->ConditionValue2 && goData->id != cond->ConditionValue2)
                            {
                                sLog.outErrorDb("ObjectEntryGuid condition has guid %u set but does not match gameobject entry (%u), skipped", cond->ConditionValue3, cond->ConditionValue2);
                                return false;
                            }
                        }
                        else
                        {
                            sLog.outErrorDb("ObjectEntryGuid condition has non existing gameobject guid (%u), skipped", cond->ConditionValue3);
                            return false;
                        }
                    }
                    break;
                case TYPEID_PLAYER:
                case TYPEID_CORPSE:
                    if (cond->ConditionValue2)
                        sLog.outErrorDb("ObjectEntryGuid condition has useless data in value2 (%u)!", cond->ConditionValue2);
                    if (cond->ConditionValue3)
                        sLog.outErrorDb("ObjectEntryGuid condition has useless data in value3 (%u)!", cond->ConditionValue3);
                    break;
                default:
                    sLog.outErrorDb("ObjectEntryGuid condition has wrong typeid set (%u), skipped", cond->ConditionValue1);
                    return false;
            } 
            break;
        }
        case CONDITION_TYPE_MASK:
        {
            if (!cond->ConditionValue1 || (cond->ConditionValue1 & ~(TYPEMASK_UNIT | TYPEMASK_PLAYER | TYPEMASK_GAMEOBJECT | TYPEMASK_CORPSE)))
            {
                sLog.outErrorDb("TypeMask condition has invalid typemask set (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue2)
                sLog.outErrorDb("TypeMask condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                sLog.outErrorDb("TypeMask condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_RELATION_TO:
        {
            if (cond->ConditionValue1 >= cond->GetMaxAvailableConditionTargets())
            {
                sLog.outErrorDb("RelationTo condition has invalid ConditionValue1(ConditionTarget selection) (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue1 == cond->ConditionTarget)
            {
                sLog.outErrorDb("RelationTo condition has ConditionValue1(ConditionTarget selection) set to self (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue2 >= RELATION_MAX)
            {
                sLog.outErrorDb("RelationTo condition has invalid ConditionValue2(RelationType) (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                sLog.outErrorDb("RelationTo condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_REACTION_TO:
        {
            if (cond->ConditionValue1 >= cond->GetMaxAvailableConditionTargets())
            {
                sLog.outErrorDb("ReactionTo condition has invalid ConditionValue1(ConditionTarget selection) (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue1 == cond->ConditionTarget)
            {
                sLog.outErrorDb("ReactionTo condition has ConditionValue1(ConditionTarget selection) set to self (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (!cond->ConditionValue2)
            {
                sLog.outErrorDb("ConditionValue2 condition has invalid ConditionValue2(rankMask) (%u), skipped", cond->ConditionValue2);
                return false;
            }
            break;
        }
        case CONDITION_DISTANCE_TO:
        {
            if (cond->ConditionValue1 >= cond->GetMaxAvailableConditionTargets())
            {
                sLog.outErrorDb("DistanceTo condition has invalid ConditionValue1(ConditionTarget selection) (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue1 == cond->ConditionTarget)
            {
                sLog.outErrorDb("DistanceTo condition has ConditionValue1(ConditionTarget selection) set to self (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue3 >= COMP_TYPE_MAX)
            {
                sLog.outErrorDb("DistanceTo condition has invalid ComparisionType (%u), skipped", cond->ConditionValue3);
                return false;
            }
            break;
        }
        case CONDITION_ALIVE:
        {
            if (cond->ConditionValue1)
                sLog.outErrorDb("Alive condition has useless data in value1 (%u)!", cond->ConditionValue1);
            if (cond->ConditionValue2)
                sLog.outErrorDb("Alive condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                sLog.outErrorDb("Alive condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_HP_VAL:
        {
            if (cond->ConditionValue2 >= COMP_TYPE_MAX)
            {
                sLog.outErrorDb("HpVal condition has invalid ComparisionType (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                sLog.outErrorDb("HpVal condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_HP_PCT:
        {
            if (cond->ConditionValue1 > 100)
            {
                sLog.outErrorDb("HpPct condition has too big percent value (%u), skipped", cond->ConditionValue1);
                return false;
            }
            if (cond->ConditionValue2 >= COMP_TYPE_MAX)
            {
                sLog.outErrorDb("HpPct condition has invalid ComparisionType (%u), skipped", cond->ConditionValue2);
                return false;
            }
            if (cond->ConditionValue3)
                sLog.outErrorDb("HpPct condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
        case CONDITION_AREAID:
        case CONDITION_INSTANCE_INFO:
            break;
        case CONDITION_WORLD_STATE:
        {
            /* @todo Implement this feature
            if (!sWorld->getWorldState(cond->ConditionValue1))
            {
                sLog.outErrorDb("World state condition has non existing world state in value1 (%u), skipped", cond->ConditionValue1);
                return false;
            }*/
            if (cond->ConditionValue2)
                sLog.outErrorDb("World state condition has useless data in value2 (%u)!", cond->ConditionValue2);
            if (cond->ConditionValue3)
                sLog.outErrorDb("World state condition has useless data in value3 (%u)!", cond->ConditionValue3);
            break;
        }
    }
    return true;
}

void ConditionMgr::Clean()
{
    for (ConditionReferenceContainer::iterator itr = ConditionReferenceStore.begin(); itr != ConditionReferenceStore.end(); ++itr)
    {
        for (ConditionList::const_iterator it = itr->second.begin(); it != itr->second.end(); ++it)
            delete *it;
        itr->second.clear();
    }

    ConditionReferenceStore.clear();

    for (ConditionContainer::iterator itr = ConditionStore.begin(); itr != ConditionStore.end(); ++itr)
    {
        for (ConditionTypeContainer::iterator it = itr->second.begin(); it != itr->second.end(); ++it)
        {
            for (ConditionList::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
                delete *i;
            it->second.clear();
        }
        itr->second.clear();
    }

    ConditionStore.clear();

    // this is a BIG hack, feel free to fix it if you can figure out the ConditionMgr ;)
    for (std::list<Condition*>::const_iterator itr = AllocatedMemoryStore.begin(); itr != AllocatedMemoryStore.end(); ++itr)
        delete *itr;

    AllocatedMemoryStore.clear();
}
