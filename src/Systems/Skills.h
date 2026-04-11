#pragma once

namespace RE
{
	namespace Cascadia
	{
		namespace Skills
		{
			typedef std::vector<ActorValueInfo*> AVVector;
			extern BSTArray<BGSPerk*> CascadiaPerksLevelUp;
			extern BSTArray<ActorValueInfo*> CascadiaSkillsLevelUp;

			enum SPECIALFormIDs
			{
				StrengthID = 706,
				PerceptionID,
				EnduranceID,
				CharismaID,
				IntelligenceID,
				AgilityID,
				LuckID,
				ExperienceID
			};

			struct VanillaAV_Struct
			{
				// Vanilla SPECIAL Values
				ActorValueInfo* Strength;
				ActorValueInfo* Perception;
				ActorValueInfo* Endurance;
				ActorValueInfo* Charisma;
				ActorValueInfo* Intelligence;
				ActorValueInfo* Agility;
				ActorValueInfo* Luck;
			};
			extern VanillaAV_Struct VanillaActorValues;

			struct CascadiaAV_Struct
			{
				// Skill Values
				ActorValueInfo* Barter;
				ActorValueInfo* EnergyWeapons;
				ActorValueInfo* Explosives;
				ActorValueInfo* Lockpick;
				ActorValueInfo* Medicine;
				ActorValueInfo* MeleeWeapons;
				ActorValueInfo* Repair;
				ActorValueInfo* Science;
				ActorValueInfo* Guns;
				ActorValueInfo* Sneak;
				ActorValueInfo* Speech;
				ActorValueInfo* Unarmed;
				ActorValueInfo* Survival;
			};
			extern CascadiaAV_Struct CascadiaActorValues;

			extern std::unordered_map<const ActorValueInfo*, std::vector<ActorValueInfo*>> specialToSkillsMap;
			extern std::unordered_map<const ActorValueInfo*, ActorValueInfo*> skillToSpecialMap;

			ActorValueInfo* GetSkillByName(std::string mySkill);
			float GetBaseSkillValueByName(Actor* myActor, std::string mySkill);

			void RegisterLinkedAV(ActorValueInfo* skill, ActorValueInfo* special);
			void RegisterForSkillLink();

			float GetAVValue(Actor* myActor, ActorValueInfo* myAV);
			float GetBaseAVValue(Actor* myActor, ActorValueInfo* myAV);
			void ModBaseAVValue(Actor* myActor, ActorValueInfo* myAV, float fModAmount);
			void SetBaseAVValue(Actor* myActor, ActorValueInfo* myAV, float fSetAmount);

			float GetPlayerAVValue(ActorValueInfo* myAV);
			float GetPlayerBaseAVValue(ActorValueInfo* myAV);
			void ModPlayerBaseAVValue(ActorValueInfo* myAV, float fModAmount);
			void SetPlayerBaseAVValue(ActorValueInfo* myAV, float fSetAmount);

			void GetLevelUpFormsFromGame();

			bool DefineSkillsFormsFromGame();
		}
	}
}
