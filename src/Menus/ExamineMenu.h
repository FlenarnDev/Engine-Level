#pragma once

#include "Shared/SharedFunctions.h"
#include <string.h>
#include <cstdarg>
#include <cstdint>
#include <RE/B/BGSInventoryInterface.h>
#include <RE/B/BGSInventoryItem.h>
#include <RE/B/BSFixedString.h>
#include <RE/B/BSInputEventUser.h>
#include <RE/B/BS_BUTTON_CODE.h>
#include <RE/B/ButtonEvent.h>
#include <RE/E/ENUM_FORM_ID.h>
#include <RE/E/ExamineMenu.h>
#include <RE/I/IMenu.h>
#include <RE/I/InventoryUserUIInterfaceEntry.h>
#include <RE/P/PipboyDataManager.h>
#include <RE/P/PlayerCharacter.h>
#include <RE/S/SendHUDMessage.h>
#include <RE/S/Setting.h>
#include <RE/T/TESDataHandler.h>
#include <RE/T/TESObjectMISC.h>
#include <RE/T/TESObjectREFR.h>
#include <RE/U/UI.h>
#include <Scaleform/G/GFx_FunctionHandler.h>
#include <Scaleform/G/GFx_Movie.h>
#include <Scaleform/G/GFx_Value.h>
#include <Scaleform/P/Ptr.h>
#include <REL/Relocation.h>
#include <REX/LOG.h>
#include <Shared/SharedDeclarations.h>
#include <Systems/Skills.h>


namespace Cascadia
{
	namespace ExamineMenu
	{

		class hkOnButtonEvent
		{
		public:
			static void InstallHook()
			{
				static REL::Relocation<std::uintptr_t> vtbl{ RE::ExamineMenu::VTABLE[1] };
				_OnButtonEvent = vtbl.write_vfunc(0x8, reinterpret_cast<std::uintptr_t>(OnButtonEvent));
			}
		private:
			static void OnButtonEvent(BSInputEventUser* a_this, const ButtonEvent* a_event)
			{
				if (a_event->QJustPressed() && (a_event->GetBSButtonCode() == BS_BUTTON_CODE::kK || a_event->GetBSButtonCode() == BS_BUTTON_CODE::kV))
				{
					BSFixedString menuString("ExamineMenu");
					IMenu* menu = UI::GetSingleton()->GetMenu(menuString).get();
					Scaleform::GFx::Value a_params[4];

					a_params[0] = "";
					a_params[1] = true;
					a_params[2] = a_event->device.underlying();
					a_params[3] = a_event->idCode;

					menu->uiMovie->asMovieRoot->Invoke("root.BaseInstance.ProcessUserEventExternal", nullptr, a_params, 4);
					return;
				}
				else
				{
					return _OnButtonEvent(a_this, a_event);
				}
			}

			inline static REL::Relocation<decltype(&OnButtonEvent)> _OnButtonEvent;
		};

		class EligableRepair : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					*a_params.retVal = nullptr;
					Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
					std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
					if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
					{
						InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);

						const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);
						*a_params.retVal = false;

						if (inventoryItem)
						{
							BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));
							if (stack)
							{
								switch (examineMenu->GetCurrentObj()->formType.get())
								{
								case ENUM_FORM_ID::kARMO:
								case ENUM_FORM_ID::kWEAP:
									if (stack->extra->GetHealthPerc() != -1.0f && stack->extra->GetHealthPerc() != 1.0f)
									{
										*a_params.retVal = true;
									}
									break;
								}
							}
						}
					}
				}
			}
		};

		class RepairKitCount : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				TESObjectMISC* repairKit = TESDataHandler::GetSingleton()->LookupForm<TESObjectMISC>(0x1D59F7, MOD_ESM);
				std::uint32_t itemCount;
				PlayerCharacter::GetSingleton()->GetItemCount(itemCount, repairKit, 0);
				*a_params.retVal = itemCount;
			}
		};

		class HasRepairKits : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				TESObjectMISC* repairKit = TESDataHandler::GetSingleton()->LookupForm<TESObjectMISC>(0x1D59F7, MOD_ESM);
				std::uint32_t itemCount;
				PlayerCharacter::GetSingleton()->GetItemCount(itemCount, repairKit, 0);
				*a_params.retVal = (itemCount > 0);
			}
		};

		class RepairNotNeeded : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
				SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sNoNeedToRepairMessage")->GetString().data(), nullptr, true, true);
			}
		};

		class NoRepairKits : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				SendHUDMessage::ShowHUDMessage("$CAS_NoRepairKits", nullptr, true, true);
			}
		};

		class RepairFunction : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

					std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
					if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
					{
						InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
						const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

						if (inventoryItem)
						{
							PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

							if (!playerCharacter->IsGodMode())
							{
								TESObjectMISC* repairKit = TESDataHandler::GetSingleton()->LookupForm<TESObjectMISC>(0x1D59F7, MOD_ESM);
								TESObjectREFR::RemoveItemData removeItemData{ repairKit, 1 };
								playerCharacter->RemoveItem(removeItemData);
							}

							switch (examineMenu->GetCurrentObj()->formType.get())
							{
							case ENUM_FORM_ID::kARMO:
							case ENUM_FORM_ID::kWEAP:
								BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));
								if (stack)
								{
									// TODO - Check for 'Master Mechanic', if player has the perk, repair amount is set to 100% instead of scaling with repair skill.
									float repairSkill = playerCharacter->GetActorValue(*Skills::CascadiaActorValues.Repair);
									float repairAmount = 0.2f + 0.3f * (repairSkill / 100.0f); // Repair amount scales from 20% at 0 repair skill to 50% at 100 repair skill.

									if (stack->extra->GetHealthPerc() + repairAmount > 1.0f)
									{
										stack->extra->SetHealthPerc(1.0f);
									}
									else
									{
										stack->extra->SetHealthPerc(stack->extra->GetHealthPerc() + repairAmount);
									}

									BGSInventoryItem::CheckStackIDFunctor compareFunction(inventoryUUIEntry->stackIndex.at(0));
									BGSInventoryItem::SetHealthFunctor writeFunction(stack->extra->GetHealthPerc());
									writeFunction.shouldSplitStacks = 0x101;

									playerCharacter->FindAndWriteStackDataForInventoryItem(examineMenu->GetCurrentObj(), compareFunction, writeFunction);

									examineMenu->UpdateOptimizedAutoBuildInv();
									selectedIndex = examineMenu->GetSelectedIndex();
									examineMenu->UpdateItemList(selectedIndex);
									examineMenu->uiMovie->Invoke("RefreshList", nullptr, nullptr, 0);

									PipboyDataManager* pipboyDataManager = PipboyDataManager::GetSingleton();
									pipboyDataManager->inventoryData.RepopulateItemCardOnSection(examineMenu->GetCurrentObj()->formType.get());
								}
								break;
							}
						}
					}
				}
			}
		};

		class RepairWorkbench : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (!UI::GetSingleton()->GetMenuOpen<RE::ExamineMenu>()) return;

				Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

				examineMenu->repairing = true;
				examineMenu->TryCreate();
			}
		};

		class NeedsRepair : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				*a_params.retVal = nullptr;
				if (!UI::GetSingleton()->GetMenuOpen<RE::ExamineMenu>()) return;
				Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

				std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
				if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
				{
					InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
					const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

					if (inventoryItem)
					{
						float tempCurrentPerc;
						*a_params.retVal = false;

						switch (examineMenu->GetCurrentObj()->formType.get())
						{
						case ENUM_FORM_ID::kARMO:
						case ENUM_FORM_ID::kWEAP:
							BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));
							if (stack)
							{
								tempCurrentPerc = stack->extra->GetHealthPerc();

								// Temporarily bump condition by 0.01 to avoid wasting a repair kit on effectively non-existant damage.
								stack->extra->SetHealthPerc(tempCurrentPerc + 0.01);
								*a_params.retVal = stack->extra->IsDamaged();
								// Restore condition back to original.
								stack->extra->SetHealthPerc(tempCurrentPerc);
							}
							break;
						}
					}
				}
			}
		};

		class NoJunk : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				RE::GameSettingCollection* gameSettingCollection = RE::GameSettingCollection::GetSingleton();
				// @TODO: Translations
				RE::SendHUDMessage::ShowHUDMessage("You don't have any junk to scrap!", nullptr, true, true);
			}
		};

		class OnEscapePress : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				REX::DEBUG("this.BGSCodeObj.OnEscapePress");
				Cascadia::Additions::Workbench_Additions::bIsScrappingAllJunk = false;
				Scaleform::Ptr<RE::ExamineMenu> examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
				if (examineMenu)
				{
					REX::DEBUG("Examine menu OnEscapePress!");
					examineMenu->repairing = false;
					//examineMenu->uiMovie->Invoke("ForceInventorySelectionMode", nullptr, nullptr, 0); // TODO - update custom buttons for this functionality.
					// examineMenu->uiMovie->asMovieRoot->Invoke("root.BaseInstance.UpdateButtons", nullptr, nullptr, 0);
				}

				
			}
		};

		class HasAnyJunk : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				auto player = RE::PlayerCharacter::GetSingleton();

				bool bHasJunk = false;

				for (std::uint32_t i = 0; i < player->inventoryList->data.size(); i++)
				{
					RE::BGSInventoryItem inventoryItem = player->inventoryList->data.at(i);

					if (!inventoryItem.object || !Shared::IsJunkItem(inventoryItem.object) || inventoryItem.IsQuestObject(0))
						continue;

					auto baseComp = Shared::GetBaseComponentFromForm(inventoryItem.object);
					if (!baseComp || !baseComp->scrapItem || baseComp->scrapItem->GetFormID() == inventoryItem.object->GetFormID())
						continue;



					switch (inventoryItem.object->GetFormType())
					{
					case RE::ENUM_FORM_ID::kWEAP: {
						if (static_cast<RE::TESObjectWEAP*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
							continue;
						}
						break;
					}
					case RE::ENUM_FORM_ID::kARMO: {
						if (static_cast<RE::TESObjectARMO*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
							continue;
						}
						break;
					}
					default:
						break;
					}

					bHasJunk = true;
					break;
				}

				*a_params.retVal = bHasJunk;
			}
		};

		class IsInAllJunk : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				*a_params.retVal = Cascadia::Additions::Workbench_Additions::bIsScrappingAllJunk;
			}
		};

		class ScrapAllJunk : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				Scaleform::Ptr<RE::ExamineMenu> examineMenu = RE::UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
				if (!examineMenu) return;
				Cascadia::Additions::Workbench_Additions::bIsScrappingAllJunk = true;
				//examineMenu->BuildWeaponScrappingArray();
				// @TODO: Add this to .as file.
				examineMenu->uiMovie->asMovieRoot->Invoke("root.BaseInstance.scrapAllJunkCallbackFromCPP", nullptr, nullptr, 0);
			}
		};

		class Workbench_HasAnyJunkExamine : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (!Cascadia::Additions::Workbench_Additions::bIsScrappingAllJunk) {
					*a_params.retVal = false;
					return;
				}

				auto player = RE::PlayerCharacter::GetSingleton();

				bool bHasJunk = false;

				for (std::uint32_t i = 0; i < player->inventoryList->data.size(); i++)
				{
					RE::BGSInventoryItem inventoryItem = player->inventoryList->data.at(i);

					if (!inventoryItem.object || !Shared::IsJunkItem(inventoryItem.object) || inventoryItem.IsQuestObject(0))
						continue;

					auto baseComp = Shared::GetBaseComponentFromForm(inventoryItem.object);
					if (!baseComp || !baseComp->scrapItem || baseComp->scrapItem->GetFormID() == inventoryItem.object->GetFormID())
						continue;

					switch (inventoryItem.object->GetFormType())
					{
					case RE::ENUM_FORM_ID::kWEAP: {
						if (static_cast<RE::TESObjectWEAP*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
							continue;
						}
						break;
					}
					case RE::ENUM_FORM_ID::kARMO: {
						if (static_cast<RE::TESObjectARMO*>(inventoryItem.object)->HasKeyword(Shared::notScrappableKeyword)) {
							continue;
						}
						break;
					}
					default:
						break;
					}


					bHasJunk = true;
					break;
				}

				*a_params.retVal = bHasJunk;
			}
		};

		class Debug_ActionScript : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params) {
				std::string s = std::format("Type is not registered for debug: {}", (std::int32_t)a_params.args[0].GetType());
				switch (a_params.args[0].GetType()) {
				case Scaleform::GFx::Value::ValueType::kUInt:
					s = std::to_string(a_params.args[0].GetUInt());
					break;
				case Scaleform::GFx::Value::ValueType::kInt:
					s = std::to_string(a_params.args[0].GetInt());
					break;
				case Scaleform::GFx::Value::ValueType::kBoolean:
					s = std::to_string(a_params.args[0].GetBoolean());
					break;
				case Scaleform::GFx::Value::ValueType::kString:
					s = a_params.args[0].GetString();
					break;
				default:
					break;
				}

				REX::DEBUG(std::format("Debug_ActionScript: {}", s).c_str());
			}
		};


		bool RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value)
		{
			Scaleform::GFx::Value currentSWFPath;
			// Register native code handlers.
			if (a_view->asMovieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url"))
			{
				if (_stricmp(currentSWFPath.GetString(), "Interface/ExamineMenu.swf") == 0)
				{
					Scaleform::GFx::Value bgsCodeObj;
					a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.BaseInstance.BGSCodeObj");

					Shared::RegisterFunction<NoRepairKits>(&bgsCodeObj, a_view->asMovieRoot, "NoRepairKits");
					Shared::RegisterFunction<RepairNotNeeded>(&bgsCodeObj, a_view->asMovieRoot, "RepairNotNeeded");
					Shared::RegisterFunction<EligableRepair>(&bgsCodeObj, a_view->asMovieRoot, "EligableRepair");
					Shared::RegisterFunction<RepairKitCount>(&bgsCodeObj, a_view->asMovieRoot, "repairKitCount");
					Shared::RegisterFunction<HasRepairKits>(&bgsCodeObj, a_view->asMovieRoot, "hasRepairKits");
					Shared::RegisterFunction<NeedsRepair>(&bgsCodeObj, a_view->asMovieRoot, "needsRepair");
					Shared::RegisterFunction<RepairFunction>(&bgsCodeObj, a_view->asMovieRoot, "CASRepairItem");
					Shared::RegisterFunction<RepairWorkbench>(&bgsCodeObj, a_view->asMovieRoot, "RepairWorkbench");

					Shared::RegisterFunction<Debug_ActionScript>(&bgsCodeObj, a_view->asMovieRoot, "DebugPrint");
					Shared::RegisterFunction<ScrapAllJunk>(&bgsCodeObj, a_view->asMovieRoot, "ScrapAllJunk");
					Shared::RegisterFunction<IsInAllJunk>(&bgsCodeObj, a_view->asMovieRoot, "IsInAllJunk");
					Shared::RegisterFunction<HasAnyJunk>(&bgsCodeObj, a_view->asMovieRoot, "HasAnyJunk");
					Shared::RegisterFunction<NoJunk>(&bgsCodeObj, a_view->asMovieRoot, "NoJunk");
					Shared::RegisterFunction<OnEscapePress>(&bgsCodeObj, a_view->asMovieRoot, "CancelBackPressed");
				}
				if (_stricmp(currentSWFPath.GetString(), "Interface/ExamineConfirmMenu.swf") == 0)
				{
					Scaleform::GFx::Value bgsCodeObj;
					a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.Menu_mc.BGSCodeObj");

					Shared::RegisterFunction<Debug_ActionScript>(&bgsCodeObj, a_view->asMovieRoot, "DebugPrintExamine");
					Shared::RegisterFunction<OnEscapePress>(&bgsCodeObj, a_view->asMovieRoot, "OnEscapePress");
					Shared::RegisterFunction<Workbench_HasAnyJunkExamine>(&bgsCodeObj, a_view->asMovieRoot, "HasAnyJunkExamine");
					Shared::RegisterFunction<OnEscapePress>(&bgsCodeObj, a_view->asMovieRoot, "CancelConfirmMenu");
					//Shared::RegisterFunction<Workbench_CompleteScrapAllJunk>(&bgsCodeObj, a_view->asMovieRoot, "CompleteScrapAllJunk");
				}
				return true;
			}
			return false;
		}
	}
}