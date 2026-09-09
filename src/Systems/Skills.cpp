#include "Shared/SharedFunctions.h"
#include "../Shared/SharedDeclarations.h"
#include "Serialization/Serialization.h"
#include "Systems/Skills.h"

#include <cmath>
#include <iterator>

namespace Cascadia
{
	namespace Skills
	{
		BSTArray<BGSPerk*> CascadiaPerksLevelUp;
		BSTArray<ActorValueInfo*> CascadiaSkillsLevelUp;

		VanillaAV_Struct VanillaActorValues;

		CascadiaAV_Struct CascadiaActorValues;

		struct CascadiaPerks_Struct
		{
			// Weapon Type Perks
			BGSPerk* WeaponTypeEnergyWeaponsPerk;
			BGSPerk* WeaponTypeExplosivesPerk;
			BGSPerk* WeaponTypeMeleeWeaponsPerk;
			BGSPerk* WeaponTypeGunsPerk;
			BGSPerk* WeaponTypeUnarmedPerk;
		};
		CascadiaPerks_Struct CascadiaPerks;
		
		struct CascadiaGlobals_Struct
		{
			// Globals
			TESGlobal* TutorialWPNCND;
		};
		CascadiaGlobals_Struct CascadiaGlobals;

		/*
			Fallout New Vegas Calculations:

			Barter				(CHA * 2) + 2 + (LCK / 2)

			Energy Weapons		(PER * 2) + 2 + (LCK / 2)

			Explosives			(PER * 2) + 2 + (LCK / 2)

			Guns				(AGI * 2) + 2 + (LCK / 2)

			Lockpick 			(PER * 2) + 2 + (LCK / 2)

			Medicine 			(INT * 2) + 2 + (LCK / 2)

			Melee Weapons 		(STR * 2) + 2 + (LCK / 2)

			Repair				(INT * 2) + 2 + (LCK / 2)

			Science				(INT * 2) + 2 + (LCK / 2)

			Sneak				(AGI * 2) + 2 + (LCK / 2)

			Speech				(CHA * 2) + 2 + (LCK / 2)

			Survival			(END * 2) + 2 + (LCK / 2)

			Unarmed				(END * 2) + 2 + (LCK / 2)
		*/

		//	Most functions in this namespace are based on shad0wshayd3's work on Project Massachusetts & HcG x Grills rework for the Capital Wasteland project, alongside some corrections and adjustments.
		//	Link: https://github.com/shad0wshayd3/F4SE-dev/tree/master/f4se/f4se_plugins/ProjectMassachusetts
		//	Rewrite Link: https://github.com/shad0wshayd3/F4SE-dev/tree/pm-rewrite

		std::unordered_map<const ActorValueInfo*, std::vector<ActorValueInfo*>> specialToSkillsMap;
		std::unordered_map<const ActorValueInfo*, ActorValueInfo*> skillToSpecialMap;
		std::unordered_multimap<std::string, ActorValueInfo*> strSkillMap;

		AVVector CascadiaSkillsList;

		// Returns ActorValueInfo based on Skill Name.
		ActorValueInfo* GetSkillByName(std::string mySkill)
		{
			auto result = strSkillMap.find(mySkill);
			if (result != strSkillMap.end())
			{
				return result->second;
			}
			return nullptr;
		}

		//	Returns Actor Value of given Actor based on Skill Name
		float GetSkillValueByName(Actor* myActor, std::string mySkill)
		{
			ActorValueInfo* myAV = GetSkillByName(mySkill);

			if (myAV)
			{
				return myActor->GetActorValue(*myAV);
			}
			else
			{
				return -1;
			}
		}

		float GetBaseSkillValueByName(Actor* myActor, std::string mySkill)
		{
			ActorValueInfo* myAV = GetSkillByName(mySkill);

			if (myAV)
			{
				return myActor->GetBaseActorValue(*myAV);
			}
			else
			{
				return -1;
			}
		}

		float DeriveSkillValue(const ActorValueOwner* a_actor, const ActorValueInfo& a_info)
		{
			if (!a_actor)
			{
				return 0.0f;
			}

			auto specialIt = skillToSpecialMap.find(&a_info);
			if (specialIt == skillToSpecialMap.end())
			{
				return 0.0f;
			}

			const float special = a_actor->GetActorValue(*specialIt->second);
			const float luck = a_actor->GetActorValue(*VanillaActorValues.Luck);

			return 2.0f + std::floor(special * 2.0f) + std::ceil(luck * 0.5f);
		}

		float GetPermanentSpecialValue(ActorValueInfo* a_special)
		{
			if (!a_special)
			{
				return 0.0f;
			}

			if (Serialization::HasPermanentSpecial(a_special->formID))
			{
				return Serialization::GetPermanentSpecial(a_special->formID);
			}

			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			float snapshot = playerCharacter->GetPermanentActorValue(*a_special);
			Serialization::SetPermanentSpecial(a_special->formID, snapshot);
			return snapshot;
		}

		void ModPermanentSpecial(ActorValueInfo* a_special, float a_delta)
		{
			if (!a_special)
			{
				return;
			}

			float newValue = GetPermanentSpecialValue(a_special) + a_delta;
			Serialization::SetPermanentSpecial(a_special->formID, newValue);

			PlayerCharacter::GetSingleton()->ModActorValue(ACTOR_VALUE_MODIFIER::kPermanent, *a_special, a_delta);
		}

		float GetPermanentSkillValue(Actor* a_actor, ActorValueInfo* a_skill)
		{
			if (!a_actor || !a_skill)
			{
				return 0.0f;
			}

			auto specialIt = skillToSpecialMap.find(a_skill);
			if (specialIt == skillToSpecialMap.end())
			{
				return 0.0f;
			}

			float special;
			float luck;
			if (a_actor == PlayerCharacter::GetSingleton())
			{
				special = GetPermanentSpecialValue(specialIt->second);
				luck = GetPermanentSpecialValue(VanillaActorValues.Luck);
			}
			else
			{
				special = a_actor->GetPermanentActorValue(*specialIt->second);
				luck = a_actor->GetPermanentActorValue(*VanillaActorValues.Luck);
			}

			const float derived = 2.0f + std::floor(special * 2.0f) + std::ceil(luck * 0.5f);
			const float invested = a_actor->GetModifier(ACTOR_VALUE_MODIFIER::kPermanent, *a_skill);

			return derived + invested;
		}

		float GetPermanentPlayerSkillValue(ActorValueInfo* a_skill)
		{
			return GetPermanentSkillValue(PlayerCharacter::GetSingleton(), a_skill);
		}

		void RegisterLinkedAV(ActorValueInfo* skill, ActorValueInfo* special)
		{
			specialToSkillsMap[special].push_back(skill);
			skillToSpecialMap[skill] = special;
			skill->derivationFunction = DeriveSkillValue;

			auto addDependent = [](ActorValueInfo* a_owner, ActorValueInfo* a_dependent)
			{
				if (a_owner->numDependentActorValues < std::size(a_owner->dependentActorValues))
				{
					a_owner->dependentActorValues[a_owner->numDependentActorValues++] = a_dependent;
				}
				else
				{
					REX::ERROR("Skills: '{}' has no room left in dependentActorValues for '{}'.", a_owner->GetFormEditorID(), a_dependent->GetFormEditorID());
				}
			};

			addDependent(special, skill);
			addDependent(VanillaActorValues.Luck, skill);
		}

		void RegisterForSkillLink()
		{
			REX::INFO("Skills: Linking Skills from FalloutCascadia.esm");

			specialToSkillsMap.clear();
			skillToSpecialMap.clear();
			strSkillMap.clear();

			// Link Skills to their SPECIAL skills.
			RegisterLinkedAV(CascadiaActorValues.Barter, VanillaActorValues.Charisma);
			RegisterLinkedAV(CascadiaActorValues.EnergyWeapons, VanillaActorValues.Perception);
			RegisterLinkedAV(CascadiaActorValues.Explosives, VanillaActorValues.Perception);
			RegisterLinkedAV(CascadiaActorValues.Guns, VanillaActorValues.Agility);
			RegisterLinkedAV(CascadiaActorValues.Lockpick, VanillaActorValues.Perception);
			RegisterLinkedAV(CascadiaActorValues.Medicine, VanillaActorValues.Intelligence);
			RegisterLinkedAV(CascadiaActorValues.MeleeWeapons, VanillaActorValues.Strength);
			RegisterLinkedAV(CascadiaActorValues.Repair, VanillaActorValues.Intelligence);
			RegisterLinkedAV(CascadiaActorValues.Science, VanillaActorValues.Intelligence);
			RegisterLinkedAV(CascadiaActorValues.Sneak, VanillaActorValues.Agility);
			RegisterLinkedAV(CascadiaActorValues.Speech, VanillaActorValues.Charisma);
			RegisterLinkedAV(CascadiaActorValues.Survival, VanillaActorValues.Endurance);
			RegisterLinkedAV(CascadiaActorValues.Unarmed, VanillaActorValues.Endurance);

			// Add Skills to array with String so we can find them based on name.
			strSkillMap.emplace("Barter", CascadiaActorValues.Barter);
			strSkillMap.emplace("EnergyWeapons", CascadiaActorValues.EnergyWeapons);
			strSkillMap.emplace("Explosives", CascadiaActorValues.Explosives);
			strSkillMap.emplace("Guns", CascadiaActorValues.Guns);
			strSkillMap.emplace("Lockpick", CascadiaActorValues.Lockpick);
			strSkillMap.emplace("Medicine", CascadiaActorValues.Medicine);
			strSkillMap.emplace("MeleeWeapons", CascadiaActorValues.MeleeWeapons);
			strSkillMap.emplace("Repair", CascadiaActorValues.Repair);
			strSkillMap.emplace("Science", CascadiaActorValues.Science);
			strSkillMap.emplace("Sneak", CascadiaActorValues.Sneak);
			strSkillMap.emplace("Speech", CascadiaActorValues.Speech);
			strSkillMap.emplace("Survival", CascadiaActorValues.Survival);
			strSkillMap.emplace("Unarmed", CascadiaActorValues.Unarmed);
		}

		float GetAVValue(Actor* myActor, ActorValueInfo* myAV)
		{
			if (myActor)
			{
				return myActor->GetActorValue(*myAV);
			}

			return NULL;
		}

		float GetBaseAVValue(Actor* myActor, ActorValueInfo* myAV)
		{
			if (myActor)
			{
				return myActor->GetBaseActorValue(*myAV);
			}

			return NULL;
		}

		void ModBaseAVValue(Actor* myActor, ActorValueInfo* myAV, float fModAmount)
		{
			myActor->ModBaseActorValue(*myAV, fModAmount);
		}

		void SetBaseAVValue(Actor* myActor, ActorValueInfo* myAV, float fSetAmount)
		{
			myActor->SetBaseActorValue(*myAV, fSetAmount);
		}

		float GetPlayerAVValue(ActorValueInfo* myAV)
		{
			return GetAVValue(PlayerCharacter::GetSingleton(), myAV);
		}

		float GetPlayerBaseAVValue(ActorValueInfo* myAV)
		{
			return GetBaseAVValue(PlayerCharacter::GetSingleton(), myAV);
		}

		void ModPlayerBaseAVValue(ActorValueInfo* myAV, float fModAmount)
		{
			ModBaseAVValue(PlayerCharacter::GetSingleton(), myAV, fModAmount);
		}

		void SetPlayerBaseAVValue(ActorValueInfo* myAV, float fSetAmount)
		{
			SetBaseAVValue(PlayerCharacter::GetSingleton(), myAV, fSetAmount);
		}

		void GetLevelUpFormsFromGame()
		{
			TESDataHandler* tesDataHandler = TESDataHandler::GetSingleton();

			BGSListForm* perkList = tesDataHandler->LookupForm<BGSListForm>(0x1F9DFA, MOD_ESM);
			for (std::uint32_t perkEntry = 0; perkEntry < perkList->arrayOfForms.size(); perkEntry++)
			{
				BGSPerk* perk = static_cast<BGSPerk*>(perkList->arrayOfForms[perkEntry]);
				if (perk == nullptr)
				{
					// Entry in list is not a perk.
					REX::DEBUG("Skills::GetLevelUpFormsFromGame, form: {} in Perk formlist is not a perk.", perkList->arrayOfForms[perkEntry]->GetFormEditorID());
					continue;
				}

				if (!perk->data.trait && perk->data.playable)
				{
					CascadiaPerksLevelUp.push_back(perk);
				}
			}

			BGSListForm* skillList = tesDataHandler->LookupForm<BGSListForm>(0x1F9DF1, MOD_ESM);
			for (std::uint32_t skillEntry = 0; skillEntry < skillList->arrayOfForms.size(); skillEntry++)
			{
				ActorValueInfo* skill = static_cast<ActorValueInfo*>(skillList->arrayOfForms[skillEntry]);
				if (skill == nullptr)
				{
					// Entry in list is not a skill.
					REX::DEBUG("Skills::GetLevelUpFormsFromGame, form: {} in Skill formlist is not a skill.", perkList->arrayOfForms[skillEntry]->GetFormEditorID());
					continue;
				}
				CascadiaSkillsLevelUp.push_back(skill);
			}
		}

		bool DefineSkillsFormsFromGame()
		{
			TESDataHandler* tesDataHandler = TESDataHandler::GetSingleton();

			// Cascadia Actor Values
			CascadiaActorValues.Barter = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35ED, MOD_ESM);
			CascadiaActorValues.EnergyWeapons = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35EE, MOD_ESM);
			CascadiaActorValues.Explosives = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35EF, MOD_ESM);
			CascadiaActorValues.Guns = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F0, MOD_ESM);
			CascadiaActorValues.Lockpick = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F1, MOD_ESM);
			CascadiaActorValues.Medicine = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F2, MOD_ESM);
			CascadiaActorValues.MeleeWeapons = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F3, MOD_ESM);
			CascadiaActorValues.Repair = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F4, MOD_ESM);
			CascadiaActorValues.Science = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F5, MOD_ESM);
			CascadiaActorValues.Sneak = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F6, MOD_ESM);
			CascadiaActorValues.Speech = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F7, MOD_ESM);
			CascadiaActorValues.Survival = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F8, MOD_ESM);
			CascadiaActorValues.Unarmed = tesDataHandler->LookupForm<ActorValueInfo>(0x0F35F9, MOD_ESM);

			// Cascadia Perks
			CascadiaPerks.WeaponTypeEnergyWeaponsPerk = tesDataHandler->LookupForm<BGSPerk>(0x1F9DFE, MOD_ESM);
			CascadiaPerks.WeaponTypeExplosivesPerk = tesDataHandler->LookupForm<BGSPerk>(0x1F9DFF, MOD_ESM);
			CascadiaPerks.WeaponTypeMeleeWeaponsPerk = tesDataHandler->LookupForm<BGSPerk>(0x1F9E00, MOD_ESM);
			CascadiaPerks.WeaponTypeGunsPerk = tesDataHandler->LookupForm<BGSPerk>(0x1F9E01, MOD_ESM);
			CascadiaPerks.WeaponTypeUnarmedPerk = tesDataHandler->LookupForm<BGSPerk>(0x1F9E02, MOD_ESM);

			// Cascadia Skills List
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Barter);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.EnergyWeapons);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Explosives);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Guns);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Lockpick);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Medicine);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.MeleeWeapons);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Repair);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Science);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Sneak);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Speech);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Survival);
			CascadiaSkillsList.emplace_back(CascadiaActorValues.Unarmed);

			// Vanilla SPECIAL List
			ActorValue* avSingleton = ActorValue::GetSingleton();

			VanillaActorValues.Strength = avSingleton->strength;
			VanillaActorValues.Perception = avSingleton->perception;
			VanillaActorValues.Endurance = avSingleton->endurance;
			VanillaActorValues.Charisma = avSingleton->charisma;
			VanillaActorValues.Intelligence = avSingleton->intelligence;
			VanillaActorValues.Agility = avSingleton->agility;
			VanillaActorValues.Luck = avSingleton->luck;

			return true;
		}
	}
}
