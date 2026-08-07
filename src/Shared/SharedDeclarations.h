#pragma once

using namespace RE;

// Retail
#define RETAIL_ESM01 "Fallout4.esm"
#define RETAIL_ESM02 "DLCRobot.esm"
#define RETAIL_ESM03 "DLCworkshop01.esm"
#define RETAIL_ESM04 "DLCCoast.esm"
#define RETAIL_ESM05 "DLCworkshop02.esm"
#define RETAIL_ESM06 "DLCworkshop03.esm"
#define RETAIL_ESM07 "DLCNukaWorld.esm"
#define RETAIL_ESM08 "DLCUltraHighResolution.esm"

// Internal
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
		extern ObjectRefHandle CurrentMultiLoc_Target;
		extern TESObjectREFR* FinalMultiDestination;
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
		extern BGSListForm* WeaponRecipesRepairList;
		extern BGSListForm* ArmorRecipesRepairList;

		extern BGSListForm* CustomRecipesListWeapon;
		extern BGSListForm* CustomRecipesListArmor;

		extern BGSConstructibleObject* DefaultOneHandedRecipe;
		extern BGSConstructibleObject* DefaultMeleeRecipe;
		extern BGSConstructibleObject* DefaultTwoHandedRecipe;
		extern BGSConstructibleObject* DefaultArmorRecipe;

		extern std::unordered_map<const TESObjectWEAP*, const BGSConstructibleObject*> weaponToCOBJ_Map;
		extern std::unordered_map<const TESObjectARMO*, const BGSConstructibleObject*> armorToCOBJ_Map;

		extern std::unordered_map<const TESObjectWEAP*, const BGSConstructibleObject*> custom_weaponToCOBJ_Map;
		extern std::unordered_map<const TESObjectARMO*, const BGSConstructibleObject*> custom_armorToCOBJ_Map;

		const BGSConstructibleObject* GetCOBJ_FromWeapon(TESObjectWEAP* weaponObj, ExtraDataList* weaponInstanceData);
		const BGSConstructibleObject* GetCOBJ_FromArmor(const TESObjectARMO* armorObj);

		void InitializeRecipes(TESDataHandler* dataHandler);
	}
}