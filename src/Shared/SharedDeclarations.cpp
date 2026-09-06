#include "SharedDeclarations.h"
#include "InventoryHelpers.h"
#include "SharedFunctions.h"

namespace Cascadia
{
	namespace Additions {
		namespace Workbench_Additions {
			bool bIsScrappingAllJunk;
			TESGlobal* Scrap_SkillMult;
			TESGlobal* Repair_WorkbenchSkillEffectReduction;
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

		// Multi Location Pipboy Travel
		bool bIsMultiTravelling;
		RE::ObjectRefHandle CurrentMultiLoc_Target;
		RE::TESObjectREFR* FinalMultiDestination;
		std::uint32_t chosenI;
		std::uint32_t markerID;

		TESForm* CustomProximityMapMarkerForm;
		BGSLocationRefType* CustomProximityMapMarkerRefType;

		namespace PipboyMap {
			std::map<const TESQuest*, std::map<std::uint32_t, Area>> MapProximityAreas;

			bool pap_add(std::monostate, std::uint32_t id, float x, float y, float radius, bool live_tracking, TESForm* quest)
			{
				const auto questCast = static_cast<TESQuest*>(quest);
				auto it = MapProximityAreas.find(questCast);
				if (it == MapProximityAreas.end()) {
					for (auto objj : questCast->objectives) {
						if (objj->index == id && objj->numTargets > 0 && objj->targets[0] != nullptr && !objj->targets[0]->target.empty()) {
							const auto reff = objj->targets[0]->target[0]->reference;
							if (Shared::IsRadiusMarkerStatic(reff.get().get())) {
								const float markerRadius = reff->extraList->GetByType<ExtraRadius>()->radius;
								REX::DEBUG("Added pap for a quest CUSTOM MARKER '{}', obj: '{}' with marker radius: {}", questCast->GetFullName(), objj->displayText.c_str(), markerRadius);
								MapProximityAreas[questCast][id] = Area(0.0f, 0.0f, markerRadius, questCast, objj);
							}
							else {
								MapProximityAreas[questCast][id] = Area(x, y, radius, questCast, objj);
								REX::DEBUG("Added pap for a quest NOT CUSTOM MARKER '{}', obj: '{}'", questCast->GetFullName(), objj->displayText.c_str());
							}
							
							break;
						}
					}
				} else {
					for (auto objj : questCast->objectives) {
						if (objj->index == id && objj->numTargets > 0 && objj->targets[0] != nullptr && !objj->targets[0]->target.empty()) {
							const auto reff = objj->targets[0]->target[0]->reference;
							if (Shared::IsRadiusMarkerStatic(reff.get().get())) {
								// @TODO GET RADIUS FROM THE MARKER FLEN!
								const float markerRadius = reff->extraList->GetByType<ExtraRadius>()->radius;
								REX::DEBUG("Added pap for a quest CUSTOM MARKER '{}', obj: '{}' with marker radius: {}", questCast->GetFullName(), objj->displayText.c_str(), markerRadius);
								MapProximityAreas[questCast][id] = Area(0.0f, 0.0f, markerRadius, questCast, objj);
							}
							else {
								MapProximityAreas[questCast][id] = Area(x, y, radius, questCast, objj);
								REX::DEBUG("Added pap for a quest NOT CUSTOM MARKER '{}', obj: '{}'", questCast->GetFullName(), objj->displayText.c_str());
							}
						}

						break;
					}
				}
				return true;
			}

			bool pap_remove(std::monostate, TESForm* quest)
			{
				const auto questCast = static_cast<TESQuest*>(quest);
				auto it = MapProximityAreas.find(questCast);
				if (it != MapProximityAreas.end())
				{
					MapProximityAreas[questCast].clear();
					MapProximityAreas.erase(it);
					REX::DEBUG("Cleaned up pap for quest {}", questCast->GetFullName());
				}
				return true;
			}

			void InitializeActiveObjectives()
			{
				REX::DEBUG("Quest Objectives checking....");
				const auto playerRef = PlayerCharacter::GetSingleton();
				for (auto qTarget = playerRef->objectives.begin(); qTarget != playerRef->objectives.end(); ++qTarget) {
					const TESQuest* quest = qTarget->objective->ownerQuest;
					if (quest == nullptr) {
						continue;
					}

					std::string buff = std::string("Objective ");
					if (Shared::IsObjectiveDisplayed(qTarget->objective) && Shared::IsQuestActive(quest)) {
						buff += qTarget->objective->displayText.data() + std::string(": Displayed");
					}
					else {
						buff += qTarget->objective->displayText.data() + std::string(": NOT Displayed");
					}

					REX::DEBUG(buff.c_str());
				}
			}

			bool RegisterFuncs(BSScript::IVirtualMachine * vm)
			{
				vm->BindNativeMethod("CAS:CAS_Global", "AddMapArea", pap_add);
				vm->BindNativeMethod("CAS:CAS_Global", "RemoveMapAreas", pap_remove);
				return true;
			}
		}

		void InitializeGameAdditionalVars(RE::TESDataHandler* dataHandler) {
			using namespace Additions;
			Workbench_Additions::Scrap_SkillMult = dataHandler->LookupForm<TESGlobal>(0x32AB29, MOD_ESM);
			Workbench_Additions::Repair_WorkbenchSkillEffectReduction = dataHandler->LookupForm<TESGlobal>(0x32AB3A, MOD_ESM);

			Bartering_Additions::Barter_SkillMult = dataHandler->LookupForm<TESGlobal>(0x32AB2B, MOD_ESM);
			Bartering_Additions::Barter_SkillSellAddition = dataHandler->LookupForm<TESGlobal>(0x32AB2C, MOD_ESM);
			Bartering_Additions::Barter_SkillBuyAddition = dataHandler->LookupForm<TESGlobal>(0x32AB2D, MOD_ESM);

			ItemDegradation::Armor_CND_ReductionPerPercent = dataHandler->LookupForm<TESGlobal>(0x32AB2E, MOD_ESM);
			ItemDegradation::Weapon_BoltMult = dataHandler->LookupForm<TESGlobal>(0x32AB2F, MOD_ESM);
			ItemDegradation::Weapon_AutomaticMult = dataHandler->LookupForm<TESGlobal>(0x32AB30, MOD_ESM);
			ItemDegradation::Weapon_CND_ReductionPerPercent = dataHandler->LookupForm<TESGlobal>(0x32AB31, MOD_ESM);

			AI_Detection::Radio_MaxDistanceEffect = dataHandler->LookupForm<TESGlobal>(0x32AB32, MOD_ESM);
			AI_Detection::Radio_DistanceScaling = dataHandler->LookupForm<TESGlobal>(0x32AB33, MOD_ESM);
			AI_Detection::Radio_AdditionBase = dataHandler->LookupForm<TESGlobal>(0x32AB34, MOD_ESM);
			AI_Detection::Light_Addition = dataHandler->LookupForm<TESGlobal>(0x32AB35, MOD_ESM);

			Mobility::JumpCost_BaseCost = dataHandler->LookupForm<TESGlobal>(0x32AB36, MOD_ESM);
			Mobility::JumpCost_BrokenLegsAdditionalMult = dataHandler->LookupForm<TESGlobal>(0x32AB37, MOD_ESM);
		}

		void InitializeSharedGameVariables()
		{
			TESDataHandler* dataHandler = TESDataHandler::GetSingleton();
			noDegradation = dataHandler->LookupForm<BGSKeyword>(0x2BD72E, MOD_ESM);
			notScrappableKeyword = dataHandler->LookupForm<BGSKeyword>(0x32AB2A, MOD_ESM);

			CustomProximityMapMarkerForm = dataHandler->LookupForm(0x001029, CURRENT_ESP);
			CustomProximityMapMarkerRefType = dataHandler->LookupForm<BGSLocationRefType>(0x00150D, CURRENT_ESP);

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
			WeaponRecipesRepairList = dataHandler->LookupForm<BGSListForm>(0x32AB4C, MOD_ESM);
			ArmorRecipesRepairList = dataHandler->LookupForm<BGSListForm>(0x32AB4D, MOD_ESM);

			CustomRecipesListArmor = dataHandler->LookupForm<BGSListForm>(0x32AB4E, MOD_ESM);
			CustomRecipesListWeapon = dataHandler->LookupForm<BGSListForm>(0x32AB4F, MOD_ESM);

			DefaultOneHandedRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x32AB49, MOD_ESM);
			DefaultTwoHandedRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x32AB4A, MOD_ESM);
			DefaultMeleeRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x32AB48, MOD_ESM);
			DefaultArmorRecipe = dataHandler->LookupForm<BGSConstructibleObject>(0x32AB4B, MOD_ESM);

			for (std::uint32_t i = 0; i < CustomRecipesListWeapon->arrayOfForms.size(); i++) {
				const BGSConstructibleObject* customRecipeWeapon = static_cast<BGSConstructibleObject*>(CustomRecipesListWeapon->arrayOfForms.at(i));
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
				const BGSConstructibleObject* customRecipeArmor = static_cast<BGSConstructibleObject*>(CustomRecipesListArmor->arrayOfForms.at(i));
				if (!customRecipeArmor) {
					REX::WARN("Index {} in the 'F76_CustomArmorModders' list in CK is not a recipe (Constructible Object). Ignoring...", i);
					continue;
				}
				auto armorObj = static_cast<TESObjectARMO*>(customRecipeArmor->GetCreatedItem());
				if (!armorObj) {
					REX::WARN("Index {} in the 'F76_CustomArmorModders' list in CK is a recipe but it's CreatedItem is not an armor or is not set!. Ignoring...", i);
					continue;
				}

				custom_armorToCOBJ_Map[armorObj] = customRecipeArmor;
			}

			for (std::uint32_t i = 0; i < WeaponRecipesRepairList->arrayOfForms.size(); i++) {
				const BGSConstructibleObject* constructible = static_cast<BGSConstructibleObject*>(WeaponRecipesRepairList->arrayOfForms.at(i));
				auto weap = static_cast<TESObjectWEAP*>(constructible->GetCreatedItem());

				weaponToCOBJ_Map.emplace(weap, constructible);
			}

			for (const auto objInList : ArmorRecipesRepairList->arrayOfForms) {
				const BGSConstructibleObject* constructible = static_cast<BGSConstructibleObject*>(objInList);
				armorToCOBJ_Map.emplace(static_cast<TESObjectARMO*>(constructible->GetCreatedItem()), constructible);
			}
		}
	}

	
}