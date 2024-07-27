#pragma once

#include "Shared/SharedFunctions.h"

namespace RE
{
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
					if (a_event->QJustPressed() && (a_event->GetBSButtonCode() == BS_BUTTON_CODE::kK))
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
						DEBUG("EligableRepair called.");
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
					DEBUG("RepairKitCount called.");
					TESObjectMISC* repairKit = TESDataHandler::GetSingleton()->LookupForm<TESObjectMISC>(0x1D59F7, "FalloutCascadia.esm");
					std::uint32_t itemCount;
					Cascadia::GetPlayerCharacter()->GetItemCount(itemCount, repairKit, 0);
					*a_params.retVal = itemCount;
				}
			};

			class HasRepairKits : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					DEBUG("HasRepairKits called.");
					TESObjectMISC* repairKit = TESDataHandler::GetSingleton()->LookupForm<TESObjectMISC>(0x1D59F7, "FalloutCascadia.esm");
					std::uint32_t itemCount;
					Cascadia::GetPlayerCharacter()->GetItemCount(itemCount, repairKit, 0);
					*a_params.retVal = (itemCount > 0);
				}
			};

			class RepairNotNeeded : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					DEBUG("RepairNotNeeded called.");
					SendHUDMessage::ShowHUDMessage("$CAS_NoNeedToRepair", 0, true, true);
				}
			};

			class NoRepairKits : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					DEBUG("NoRepairKits called.");
					SendHUDMessage::ShowHUDMessage("$CAS_NoRepairKits", 0, true, true);
				}
			};

			class RepairFunction : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					if (a_params.retVal)
					{
						DEBUG("RepairFunction called.");
						Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

						std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
						if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
						{
							InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
							const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

							if (inventoryItem)
							{
								if (!Cascadia::GetPlayerCharacter()->IsGodMode())
								{
									TESObjectMISC* repairKit = TESDataHandler::GetSingleton()->LookupForm<TESObjectMISC>(0x1D59F7, "FalloutCascadia.esm");
									TESObjectREFR::RemoveItemData removeItemData{ repairKit, 1 };
									Cascadia::GetPlayerCharacter()->RemoveItem(removeItemData);
								}

								switch (examineMenu->GetCurrentObj()->formType.get())
								{
								case ENUM_FORM_ID::kARMO:
								case ENUM_FORM_ID::kWEAP:
									BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));
									if (stack)
									{
										if (stack->extra->GetHealthPerc() + 0.5f > 1.0f)
										{
											stack->extra->SetHealthPerc(1.0f);
										}
										else
										{
											stack->extra->SetHealthPerc(stack->extra->GetHealthPerc() + 0.5f);
										}

										BGSInventoryItem::CheckStackIDFunctor compareFunction(inventoryUUIEntry->stackIndex.at(0));
										BGSInventoryItem::SetHealthFunctor writeFunction(stack->extra->GetHealthPerc());
										writeFunction.shouldSplitStacks = 0x101;

										Cascadia::GetPlayerCharacter()->FindAndWriteStackDataForInventoryItem(examineMenu->GetCurrentObj(), compareFunction, writeFunction);

										examineMenu->UpdateOptimizedAutoBuildInv();
										selectedIndex = examineMenu->GetSelectedIndex();
										examineMenu->UpdateItemList(selectedIndex);
										examineMenu->uiMovie->Invoke("RefreshList", nullptr, nullptr, 0);

										PipboyDataManager* pipboyDataManager = PipboyDataManager::GetSingleton();
										pipboyDataManager->inventoryData.RepopulateItemCardsOnSection(examineMenu->GetCurrentObj()->formType.get());
									}
									break;
								}
							}
						}
					}
				}
			};

			class NeedsRepair : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					DEBUG("NeedsRepair called.");
					*a_params.retVal = nullptr;
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

									// Temporarily bump condition by 0.001 to avoid wasting a repair kit on effectively non-existant damage.
									stack->extra->SetHealthPerc(tempCurrentPerc + 0.001);
									*a_params.retVal = stack->extra->IsDamaged();
									// Restore condition back to original.
									stack->extra->SetHealthPerc(tempCurrentPerc);
								}
								break;
							}
						}
					}
					else
					{
						DEBUG("Invalid.");
					}
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

						RegisterFunction<NoRepairKits>(&bgsCodeObj, a_view->asMovieRoot, "NoRepairKits");
						RegisterFunction<RepairNotNeeded>(&bgsCodeObj, a_view->asMovieRoot, "RepairNotNeeded");
						RegisterFunction<EligableRepair>(&bgsCodeObj, a_view->asMovieRoot, "EligableRepair");
						RegisterFunction<RepairKitCount>(&bgsCodeObj, a_view->asMovieRoot, "repairKitCount");
						RegisterFunction<HasRepairKits>(&bgsCodeObj, a_view->asMovieRoot, "hasRepairKits");
						RegisterFunction<NeedsRepair>(&bgsCodeObj, a_view->asMovieRoot, "needsRepair");
						RegisterFunction<RepairFunction>(&bgsCodeObj, a_view->asMovieRoot, "CASRepairItem");
					}
					return true;
				}
				return false;
			}
		}
	}
}