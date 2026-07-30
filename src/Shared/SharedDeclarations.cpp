#include "Shared/SharedDeclarations.h"
#include "InventoryHelpers.h"

namespace Cascadia
{
	namespace Additions {
		namespace Workbench_Additions {
			bool bIsScrappingAllJunk;
			TESGlobal* Scrap_SkillMult;
			TESGlobal* Repair_workbenchSkillEffectReduction;
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
			Workbench_Additions::Repair_workbenchSkillEffectReduction = dataHandler->LookupForm<TESGlobal>(0x000C2F, CURRENT_ESP);

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
			Recipes::InitializeRecipes(dataHandler);

		}

		void InitializeSharedVariables()
		{
			noArmorDegradation = false;
			noWeaponDegradation = false;
		}
	}

	namespace Recipes {
		RE::BGSListForm* WeaponRecipesRepairList;
		RE::BGSListForm* ArmorRecipesRepairList;

		RE::BGSListForm* CustomRecipesListWeapon;
		RE::BGSListForm* CustomRecipesListArmor;

		RE::BGSConstructibleObject* DefaultOneHandedRecipe;
		RE::BGSConstructibleObject* DefaultMeleeRecipe;
		RE::BGSConstructibleObject* DefaultTwoHandedRecipe;
		RE::BGSConstructibleObject* DefaultArmorRecipe;

		std::unordered_map<const RE::TESObjectWEAP*, const RE::BGSConstructibleObject*> weaponToCOBJ_Map;
		std::unordered_map<const RE::TESObjectARMO*, const RE::BGSConstructibleObject*> armorToCOBJ_Map;

		std::unordered_map<const RE::TESObjectWEAP*, const RE::BGSConstructibleObject*> custom_weaponToCOBJ_Map;
		std::unordered_map<const RE::TESObjectARMO*, const RE::BGSConstructibleObject*> custom_armorToCOBJ_Map;

		const RE::BGSConstructibleObject* GetCOBJ_FromWeapon(RE::TESObjectWEAP* weaponObj, RE::ExtraDataList* weaponInstanceData)
		{
			if (custom_weaponToCOBJ_Map.contains(weaponObj)) {
				return custom_weaponToCOBJ_Map.at(weaponObj);
			}

			if (weaponToCOBJ_Map.contains(weaponObj)) {
				return weaponToCOBJ_Map.at(weaponObj);
			}
			return nullptr;
		}

		const RE::BGSConstructibleObject* GetCOBJ_FromArmor(const RE::TESObjectARMO* armorObj)
		{
			if (custom_armorToCOBJ_Map.contains(armorObj)) {
				return custom_armorToCOBJ_Map.at(armorObj);
			}

			return armorToCOBJ_Map.contains(armorObj) ? armorToCOBJ_Map.at(armorObj) : nullptr;
		}

		void InitializeRecipes(RE::TESDataHandler* dataHandler) {
			WeaponRecipesRepairList = dataHandler->LookupForm<BGSListForm>(0x000D11, CURRENT_ESP);
			ArmorRecipesRepairList = dataHandler->LookupForm<BGSListForm>(0x000D12, CURRENT_ESP);

			CustomRecipesListArmor = dataHandler->LookupForm<BGSListForm>(0x000D13, CURRENT_ESP);
			CustomRecipesListWeapon = dataHandler->LookupForm<BGSListForm>(0x000D14, CURRENT_ESP);

			DefaultOneHandedRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x000D0E, CURRENT_ESP);
			DefaultTwoHandedRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x000D0F, CURRENT_ESP);
			DefaultMeleeRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x000D0D, CURRENT_ESP);
			DefaultArmorRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x000D10, CURRENT_ESP);



			for (std::uint32_t i = 0; i < CustomRecipesListWeapon->arrayOfForms.size(); i++) {
				const RE::BGSConstructibleObject* customRecipeWeapon = static_cast<RE::BGSConstructibleObject*>(CustomRecipesListWeapon->arrayOfForms.at(i));
				if (!customRecipeWeapon) {
					REX::WARN("Index {} in the 'F76_CustomWeaponModders' list in CK is not a recipe (Constructible Object). Ignoring...", i);
					continue;
				}
				auto weapObj = static_cast<RE::TESObjectWEAP*>(customRecipeWeapon->GetCreatedItem());
				if (!weapObj) {
					REX::WARN("Index {} in the 'F76_CustomWeaponModders' list in CK is a recipe but it's CreatedItem is not a weapon or is not set!. Ignoring...", i);
					continue;
				}

				custom_weaponToCOBJ_Map[weapObj] = customRecipeWeapon;
			}

			for (std::uint32_t i = 0; i < CustomRecipesListArmor->arrayOfForms.size(); i++) {
				const RE::BGSConstructibleObject* customRecipeArmor = static_cast<RE::BGSConstructibleObject*>(CustomRecipesListArmor->arrayOfForms.at(i));
				if (!customRecipeArmor) {
					REX::WARN("Index {} in the 'F76_CustomArmorModders' list in CK is not a recipe (Constructible Object). Ignoring...", i);
					continue;
				}
				auto armorObj = static_cast<RE::TESObjectARMO*>(customRecipeArmor->GetCreatedItem());
				if (!armorObj) {
					REX::WARN("Index {} in the 'F76_CustomArmorModders' list in CK is a recipe but it's CreatedItem is not an armor or is not set!. Ignoring...", i);
					continue;
				}

				custom_armorToCOBJ_Map[armorObj] = customRecipeArmor;
			}



			for (std::uint32_t i = 0; i < WeaponRecipesRepairList->arrayOfForms.size(); i++) {
				const RE::BGSConstructibleObject* constructible = static_cast<RE::BGSConstructibleObject*>(WeaponRecipesRepairList->arrayOfForms.at(i));
				auto weap = static_cast<RE::TESObjectWEAP*>(constructible->GetCreatedItem());

				weaponToCOBJ_Map.emplace(weap, constructible);
			}

			for (const auto objInList : ArmorRecipesRepairList->arrayOfForms) {
				const RE::BGSConstructibleObject* constructible = static_cast<RE::BGSConstructibleObject*>(objInList);
				armorToCOBJ_Map.emplace(static_cast<RE::TESObjectARMO*>(constructible->GetCreatedItem()), constructible);
			}
		}
	}

	
}