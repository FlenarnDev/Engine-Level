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
#define CURRENT_ESP "CAS_TBM_03.esp"

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

		extern TESForm* CustomProximityMapMarkerForm;
		extern BGSLocationRefType* CustomProximityMapMarkerRefType;

		namespace PipboyMap {
			class Area
			{
			public:
				Area() : _x(0), _y(0), _radius(0), _live_tracking(false) { _quest = nullptr; _objective = nullptr; }
				Area(float x, float y, float radius, const TESQuest* quest, const BGSQuestObjective* objective) : _x(x), _y(y), _radius(radius), _live_tracking(false) { _quest = quest; _objective = objective;}
				Area(float x, float y, float radius, const TESQuest* quest, const BGSQuestObjective* objective, bool live_tracking) : _x(x), _y(y), _radius(radius), _live_tracking(live_tracking) { _quest = quest; _objective = objective; }

				float _x;
				float _y;
				float _radius;
				const TESQuest* _quest;
				const BGSQuestObjective* _objective;
				bool _live_tracking;
			};

			extern std::map<const TESQuest*, std::map<std::uint32_t, Area>> MapProximityAreas;

			bool pap_add(std::monostate, std::uint32_t id, float x, float y, float radius, bool live_tracking, TESForm* quest);
			bool pap_remove(std::monostate, TESForm* quest);

			void InitializeActiveObjectives();

			bool RegisterFuncs(BSScript::IVirtualMachine* vm);
		}
		
	}

	namespace Additions {
		namespace Workbench_Additions {
			extern bool bIsScrappingAllJunk;
			extern TESGlobal* Scrap_SkillMult;
			extern TESGlobal* Repair_WorkbenchSkillEffectReduction;
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