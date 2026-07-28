#include "Shared/SharedDeclarations.h"

namespace Cascadia
{
	namespace Additions {
		namespace Workbench_Additions {
			bool bIsScrappingAllJunk;
			TESGlobal* Scrap_SkillMult;
		}

		namespace Bartering_Additions {
			TESGlobal* Barter_SkillMult;
			TESGlobal* Barter_SkillBuyAddition;
			TESGlobal* Barter_SkillSellAddition;
		}

		namespace ItemDegradation {
			TESGlobal* Armor_CND_ReductionPerPercent;
			TESGlobal* Weapon_CND_ReductionPerPercent;
			TESGlobal* Weapon_AutomaticMult;
			TESGlobal* Weapon_BoltMult;
		}

		namespace AI_Detection {
			TESGlobal* Radio_MaxDistanceEffect;
			TESGlobal* Radio_DistanceScaling;
			TESGlobal* Radio_AdditionBase;
			TESGlobal* Light_Addition;
		}

		namespace Mobility {
			TESGlobal* JumpCost_BaseCost;
			TESGlobal* JumpCost_BrokenLegsAdditionalMult;
		}
	}

	namespace Shared
	{	
		BGSKeyword* notScrappableKeyword;
		BGSKeyword* noDegradation;
		TESLoadScreen* forcedLoadScreen;
		bool noArmorDegradation;
		bool noWeaponDegradation;

		void InitializeGameAdditionalVars(RE::TESDataHandler* dataHandler) {
			using namespace Additions;
			Workbench_Additions::Scrap_SkillMult = dataHandler->LookupForm<TESGlobal>(0x0009A0, CURRENT_ESP);

			Bartering_Additions::Barter_SkillMult = dataHandler->LookupForm<TESGlobal>(0x0009A2, CURRENT_ESP);
			Bartering_Additions::Barter_SkillSellAddition = dataHandler->LookupForm<TESGlobal>(0x0009A3, CURRENT_ESP);
			Bartering_Additions::Barter_SkillBuyAddition = dataHandler->LookupForm<TESGlobal>(0x0009A4, CURRENT_ESP);

			ItemDegradation::Armor_CND_ReductionPerPercent = dataHandler->LookupForm<TESGlobal>(0x0009A5, CURRENT_ESP);
			ItemDegradation::Weapon_BoltMult = dataHandler->LookupForm<TESGlobal>(0x0009A6, CURRENT_ESP);
			ItemDegradation::Weapon_AutomaticMult = dataHandler->LookupForm<TESGlobal>(0x0009A7, CURRENT_ESP);
			ItemDegradation::Weapon_CND_ReductionPerPercent = dataHandler->LookupForm<TESGlobal>(0x0009A8, CURRENT_ESP);

			AI_Detection::Radio_MaxDistanceEffect = dataHandler->LookupForm<TESGlobal>(0x0009A9, CURRENT_ESP);
			AI_Detection::Radio_DistanceScaling = dataHandler->LookupForm<TESGlobal>(0x0009AA, CURRENT_ESP);
			AI_Detection::Radio_AdditionBase = dataHandler->LookupForm<TESGlobal>(0x0009AB, CURRENT_ESP);
			AI_Detection::Light_Addition = dataHandler->LookupForm<TESGlobal>(0x0009AC, CURRENT_ESP);

			Mobility::JumpCost_BaseCost = dataHandler->LookupForm<TESGlobal>(0x0009AD, CURRENT_ESP);
			Mobility::JumpCost_BrokenLegsAdditionalMult = dataHandler->LookupForm<TESGlobal>(0x0009AE, CURRENT_ESP);
		}

		void InitializeSharedGameVariables()
		{
			TESDataHandler* dataHandler = TESDataHandler::GetSingleton();
			noDegradation = dataHandler->LookupForm<BGSKeyword>(0x2BD72E, MOD_ESM);
			notScrappableKeyword = dataHandler->LookupForm<BGSKeyword>(0x0009A1, CURRENT_ESP);

			InitializeGameAdditionalVars(dataHandler);
		}

		void InitializeSharedVariables()
		{
			noArmorDegradation = false;
			noWeaponDegradation = false;
		}
	}

	
}