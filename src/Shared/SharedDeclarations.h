#pragma once

using namespace RE;

#define MOD_ESM "FalloutCascadia.esm"
#define CURRENT_ESP "CAS_TBM_01.esp"

namespace Cascadia
{
	namespace Shared
	{
		extern BGSKeyword* noDegradation;
		extern BGSKeyword* notScrappableKeyword;

		extern TESLoadScreen* forcedLoadScreen;
		void InitializeSharedVariables();
		void InitializeSharedGameVariables();
		extern bool noArmorDegradation;
		extern bool noWeaponDegradation;

		// Multi Location Pipboy Travel
		extern bool bIsMultiTravelling;
		extern RE::ObjectRefHandle CurrentMultiLoc_Target;
		extern RE::TESObjectREFR* FinalMultiDestination;
		extern std::uint32_t chosenI;
		extern std::uint32_t markerID;
	}

	namespace Additions {
		namespace Workbench_Additions {
			extern bool bIsScrappingAllJunk;
			extern TESGlobal* Scrap_SkillMult;
			extern TESGlobal* Repair_workbenchSkillEffectReduction;
		}

		namespace Bartering_Additions {
			extern TESGlobal* Barter_SkillMult;
			extern TESGlobal* Barter_SkillBuyAddition;
			extern TESGlobal* Barter_SkillSellAddition;
		}

		namespace ItemDegradation {
			extern TESGlobal* Armor_CND_ReductionPerPercent;
			extern TESGlobal* Weapon_CND_ReductionPerPercent;
			extern TESGlobal* Weapon_AutomaticMult;
			extern TESGlobal* Weapon_BoltMult;
		}

		namespace AI_Detection {
			extern TESGlobal* Radio_MaxDistanceEffect;
			extern TESGlobal* Radio_DistanceScaling;
			extern TESGlobal* Radio_AdditionBase;
			extern TESGlobal* Light_Addition;
		}

		namespace Mobility {
			extern TESGlobal* JumpCost_BaseCost;
			extern TESGlobal* JumpCost_BrokenLegsAdditionalMult;
		}
	}

	namespace Recipes {
		extern RE::BGSListForm* WeaponRecipesRepairList;
		extern RE::BGSListForm* ArmorRecipesRepairList;

		extern RE::BGSListForm* CustomRecipesListWeapon;
		extern RE::BGSListForm* CustomRecipesListArmor;

		extern RE::BGSConstructibleObject* DefaultOneHandedRecipe;
		extern RE::BGSConstructibleObject* DefaultMeleeRecipe;
		extern RE::BGSConstructibleObject* DefaultTwoHandedRecipe;
		extern RE::BGSConstructibleObject* DefaultArmorRecipe;

		extern std::unordered_map<const RE::TESObjectWEAP*, const RE::BGSConstructibleObject*> weaponToCOBJ_Map;
		extern std::unordered_map<const RE::TESObjectARMO*, const RE::BGSConstructibleObject*> armorToCOBJ_Map;

		extern std::unordered_map<const RE::TESObjectWEAP*, const RE::BGSConstructibleObject*> custom_weaponToCOBJ_Map;
		extern std::unordered_map<const RE::TESObjectARMO*, const RE::BGSConstructibleObject*> custom_armorToCOBJ_Map;

		const RE::BGSConstructibleObject* GetCOBJ_FromWeapon(RE::TESObjectWEAP* weaponObj, RE::ExtraDataList* weaponInstanceData);
		const RE::BGSConstructibleObject* GetCOBJ_FromArmor(const RE::TESObjectARMO* armorObj);

		void InitializeRecipes(RE::TESDataHandler* dataHandler);
	}

	

	
}