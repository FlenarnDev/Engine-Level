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

			for (BGSInventoryItem& item : reverseInv)
			{
				if (item.object->formType == ENUM_FORM_ID::kMISC)
				{
					TESObjectMISC* miscItem = (TESObjectMISC*)(item.object);
					
					if (miscItem->componentData)
					{
						for (auto& componentData : *miscItem->componentData)
						{
							if (componentData.first == a_form)
							{
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
			Scaleform::GFx::Value openValue;
			menu->uiMovie->asMovieRoot->GetVariable(&openValue, "root.HUDNotificationsGroup_mc.XPMeter_mc.visible");

			return openValue.GetBoolean();
		}

		BGSKeyword* GetAmmoKeywordStandard(TESAmmo* ammo)
		{
			std::uint32_t keywordCount = ammo->GetNumKeywords();
			const char* standardListPrefix = "CAS_AmmoSwitch_Standard_";

			for (std::uint32_t i = 0; i <= keywordCount; i++)
			{
				std::optional<BGSKeyword*> keyword = ammo->GetKeywordAt(i);
				if (keyword.has_value())
				{
					const char* keywordEDID = keyword.value()->GetFormEditorID();

					if (strncmp(keywordEDID, standardListPrefix, strlen(standardListPrefix)) == 0)
					{

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
			if (!obj || obj->formType.get() != RE::ENUM_FORM_ID::kMISC) {
				return false;
			}
			auto* misc = static_cast<RE::TESObjectMISC*>(obj);
			if ((misc->formID & 0x00FFFFFFu) == 0x0000000Fu || (misc->formID & 0x00FFFFFFu) == 0x0000000Au || misc->HasKeyword(Shared::notScrappableKeyword)) return false;  // caps or bobby pins, never

			if (misc->componentData && !misc->componentData->empty()) {
				//if (misc->componentData->at(0).first->GetFormType() != RE::ENUM_FORM_ID::kMISC) {
				//	return false;
				//}

				// auto compObj = static_cast<RE::TESObjectMISC*>(misc->componentData->at(0).first);

				//if (compObj == nullptr && compObj->IsBoundObject())
					//return false;

				//return compObj && compObj->componentData->at(0);
				return true;

				//return compObj;
			}

			return false;
		}

		RE::BGSComponent* GetBaseComponentFromForm(RE::TESForm* a_form)
		{
			if (!a_form) return nullptr;

			auto miscObj = static_cast<RE::TESObjectMISC*>(a_form);

			if (!miscObj || a_form->GetFormType() != RE::ENUM_FORM_ID::kMISC) {
				auto compObj = static_cast<RE::BGSComponent*>(a_form);
				return compObj ? compObj : nullptr;
			}

			if (!miscObj->componentData || miscObj->componentData->empty() || !miscObj->componentData->at(0).first)
				return nullptr;

			const auto first = miscObj->componentData->at(0).first;

			RE::BGSComponent* compObj = static_cast<RE::BGSComponent*>(first);
			//if (!compObj || !compObj->scrapItem)
				//return nullptr;

			return compObj;
		}
	}
}
