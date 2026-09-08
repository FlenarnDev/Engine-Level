#include "SharedFunctions.h"
#include "SharedDeclarations.h"

namespace Cascadia
{
	namespace Shared
	{
		//	Converts a UInt8 percentage to float (eg. 38% becomes 0.38)
		float ConvertPercentageToFloat(std::uint8_t percentage)
		{
			return (percentage / static_cast<float>(100));
		}

		bool HasVMScript(TESForm* form, const char* scriptName)
		{
			BSTSmartPointer<BSScript::IVirtualMachine> vm = GameVM::GetSingleton()->GetVM();
			const BSScript::IObjectHandlePolicy& handlePolicy = vm->GetObjectHandlePolicy();
			std::size_t handle = handlePolicy.GetHandleForObject(form->formType.underlying(), form);

			BSTSmartPointer<BSScript::Object> identifier;

			return vm->FindBoundObject(handle, scriptName, 1, identifier, 0);
		}

		std::string trim(const std::string& str) 
		{
			size_t start = str.find_first_not_of(" \t\n\r");
			size_t end = str.find_last_not_of(" \t\n\r");
			return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
		}

		bool InMenuMode()
		{
			UI* ui = UI::GetSingleton();
			return (
				(ui->menuMode >= 1)
				|| ui->GetMenuOpen("CookingMenu")
				|| ui->GetMenuOpen("FaderMenu")
				|| ui->GetMenuOpen("FavoritesMenu")
				|| ui->GetMenuOpen("PowerArmorModMenu")
				|| ui->GetMenuOpen("RobotModMenu")
				|| ui->GetMenuOpen("VATSMenu")
				|| ui->GetMenuOpen("WorkshopMenu")
				|| ui->GetMenuOpen("DialogueMenu")
			);
		}

		std::uint32_t GetAvailableComponentCount(BGSInventoryList* a_list, TESForm* a_form)
		{
			auto reverseInv = a_list->data;
			std::reverse(reverseInv.begin(), reverseInv.end());
			std::uint32_t amount = 0;

			for (BGSInventoryItem& item : reverseInv) {
				if (item.object->formType == ENUM_FORM_ID::kMISC) {
					TESObjectMISC* miscItem = (TESObjectMISC*)(item.object);
					
					if (miscItem->componentData) {
						for (auto& componentData : *miscItem->componentData) {
							if (componentData.first == a_form) {
								std::uint32_t count = [item]
									{
										std::uint32_t _count = 0;
										BSTSmartPointer<BGSInventoryItem::Stack> pointer = item.stackData;

										while (pointer)
										{
											_count += pointer->GetCount();
											pointer = pointer->nextStack;
										}

										return _count;
									}();
								
								amount += componentData.second.i * count;
								break;
							}
						}
					}
				}
			}

			return amount;
		}

		bool IsXPMetervisible()
		{
			BSFixedString menuString("HUDMenu");
			IMenu* menu = UI::GetSingleton()->GetMenu(menuString).get();
			if (!menu || !menu->uiMovie || !menu->uiMovie->asMovieRoot)
			{
				// HUDMenu not up yet (e.g. very early game state) - nothing to be visible.
				return false;
			}

			Scaleform::GFx::Value openValue;
			menu->uiMovie->asMovieRoot->GetVariable(&openValue, "root.HUDNotificationsGroup_mc.XPMeter_mc.visible");

			return openValue.GetBoolean();
		}

		BGSKeyword* GetAmmoKeywordStandard(TESAmmo* ammo)
		{
			std::uint32_t keywordCount = ammo->GetNumKeywords();
			const char* standardListPrefix = "CAS_AmmoSwitch_Standard_";

			for (std::uint32_t i = 0; i <= keywordCount; i++) {
				std::optional<BGSKeyword*> keyword = ammo->GetKeywordAt(i);
				if (keyword.has_value()) {
					const char* keywordEDID = keyword.value()->GetFormEditorID();

					if (strncmp(keywordEDID, standardListPrefix, strlen(standardListPrefix)) == 0) {

						return keyword.value();
					}
				}
			}

			return nullptr;
		}

		bool IsJunkItem(RE::TESBoundObject* obj)
		{
			// "Take my junk" must take ONLY scrappable junk. caps (0xF), bobby pins
			// (0xA), keys, quest items and collectibles are all kMISC too, so a bare
			// kMISC test wrongly swept them up (reported by a VR user). The engine's
			// signal for real junk is a non-empty crafting-component list
			// (TESObjectMISC::componentData); the non-junk MISC above have none.
			// (Books are kBOOK and were never matched here.)
			if (!obj || obj->formType.get() != ENUM_FORM_ID::kMISC) {
				return false;
			}
			auto* misc = static_cast<TESObjectMISC*>(obj);
			if ((misc->formID & 0x00FFFFFFu) == 0x0000000Fu || (misc->formID & 0x00FFFFFFu) == 0x0000000Au || misc->HasKeyword(Shared::notScrappableKeyword)) return false;  // caps or bobby pins, never

			if (misc->componentData && !misc->componentData->empty()) {
				return true;
			}

			return false;
		}

		BGSComponent* GetBaseComponentFromForm(TESForm* a_form)
		{
			if (!a_form) return nullptr;

			TESObjectMISC* miscObj = static_cast<TESObjectMISC*>(a_form);

			if (!miscObj || a_form->GetFormType() != ENUM_FORM_ID::kMISC) {
				auto compObj = static_cast<BGSComponent*>(a_form);
				return compObj ? compObj : nullptr;
			}

			if (!miscObj->componentData || miscObj->componentData->empty() || !miscObj->componentData->at(0).first) {
				return nullptr;
			}

			const auto first = miscObj->componentData->at(0).first;
			BGSComponent* compObj = static_cast<RE::BGSComponent*>(first);

			return compObj;
		}
		void ApplyFormulaForRepairRequirements(const RE::BSTArray<RE::ExamineMenu::ModChoiceData>& modArray, const RE::ExtraDataList* extraData, RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>& recipeReqItems,
			RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>& reqItems, const float currentCondition, const float CurrentRepairSkill)
		{
			const std::uint32_t repairSkillReduction = CurrentRepairSkill / Cascadia::Additions::Workbench_Additions::Repair_WorkbenchSkillEffectReduction->GetValue();
			std::uint32_t i = 0;

			// auto vm = RE::GameVM::GetSingleton()->GetVM();

			std::vector<RE::TESObjectMISC*> mods;




			auto* player = RE::PlayerCharacter::GetSingleton();

			// We first add the mods if any
			RE::BGSObjectInstanceExtra* instExtra = extraData->GetByType<RE::BGSObjectInstanceExtra>();
			if (instExtra && player && player->inventoryList) {
				for (auto& idx : instExtra->GetIndexData()) {
					// idx.objectID is the OMOD's raw FormID — resolve it to the real form
					auto mod = RE::TESForm::GetFormByID<RE::BGSMod::Attachment::Mod>(idx.objectID);
					if (!mod)
						continue;

					for (const auto& modInModArray : modArray) {
						if (!modInModArray.mod || !modInModArray.recipe || !modInModArray.recipe->requiredItems) continue;

						if (modInModArray.mod->GetFormID() == mod->GetFormID()) {

							for (auto needed = modInModArray.recipe->requiredItems->begin(); needed != modInModArray.recipe->requiredItems->end(); ++needed) {
								auto finalTuple = RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(needed->first, needed->second);
								reqItems.push_back(finalTuple);
							}
						}
					}
				}
			}

			// Shared::GetAvailableComponentCount(inventoryList, form);



			for (auto needed = recipeReqItems.begin(); needed != recipeReqItems.end(); ++needed) {

				RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>* neededCopy = new RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(needed->first, needed->second.i);

				RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>* foundInMod = nullptr;
				int ii = 0;
				for (auto reqInList = reqItems.begin(); reqInList != reqItems.end(); ++reqInList) {
					if ((std::uint32_t)reqInList->first->GetFormID() == (std::uint32_t)needed->first->GetFormID()) {
						foundInMod = reqInList;
						neededCopy->second.i += (reqInList->second.i * (1.0f - currentCondition));
						break;
					}
					ii++;
				}


				const std::uint32_t oldCount = neededCopy->second.i;

				if (repairSkillReduction >= 1) {
					int newCount = neededCopy->second.i;
					newCount -= repairSkillReduction;
					neededCopy->second.i = std::max(newCount, 1);
				}

				// Condition Reduction
				neededCopy->second.i = (std::uint32_t) std::max(neededCopy->second.i * (1.0f - currentCondition), 1.0f);

				auto finalTuple = RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>(neededCopy->first, neededCopy->second.i);
				if (foundInMod) {
					reqItems[ii] = finalTuple;
				}
				else {
					reqItems.push_back(finalTuple);
				}








				//(*currentModChoiceData->recipe->requiredItems)[i] = *neededCopy;
				// needed.second.f = max((int)needed.second.f / repairSkillReduction, 1);
				// (*currentModChoiceData->recipe->requiredItems)[i] = needed;

				REX::DEBUG(std::format("RepairReduction - Comp from {} to {}. Skill Reduction: {}. With mods? {}", oldCount, neededCopy->second.i, repairSkillReduction, foundInMod ? 1 : 0).c_str());
				i++;
			}
		}

		bool IsMeleeWeapon(RE::WEAPON_TYPE weaponType)
		{
			bool result = false;

			switch (weaponType) {
				case WEAPON_TYPE::kHandToHand:
				case WEAPON_TYPE::kOneHandAxe:
				case WEAPON_TYPE::kOneHandDagger:
				case WEAPON_TYPE::kOneHandMace:
				case WEAPON_TYPE::kOneHandSword:
				case WEAPON_TYPE::kStaff:
				case WEAPON_TYPE::kTwoHandAxe:
				case WEAPON_TYPE::kTwoHandSword:
					result = true;
					break;
				default:
					result = false;
					break;
			}

			return result;
		}
		bool IsObjectiveDisplayed(const BGSQuestObjective* Objective)
		{
			if (Objective) {
				char cState = Objective->state;
				if (((cState - 1) & 0xF9) == 0 && cState != 7) {
					return 1;
				}
			}

			return false;
		}

		bool IsQuestActive(const TESQuest* Quest)
		{
			if (Quest) {
				return (Quest->data.flags >> 11) & 1;
			}

			return false;
		}
		bool RegisterPapyrusFuncs(BSScript::IVirtualMachine* vm)
		{
			if (!Shared::PipboyMap::RegisterFuncs(vm))
				return false;

			return true;
		}
		bool IsRadiusMarkerStatic(const TESObjectREFR* object)
		{
			return Shared::CustomProximityMapMarkerForm->GetFormID() == object->GetObjectReference()->GetFormID() && object->extraList != nullptr && object->extraList->HasType<ExtraRadius>();
		}
	}
}
