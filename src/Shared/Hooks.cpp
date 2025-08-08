#include "Shared/Hooks.h"
#include "detourXS/detourxs.h"
#undef min
#undef max
#undef GetForm

namespace RE
{
	namespace Cascadia
	{
		extern std::map<TESAmmo*, float> ammoDegradationMap;

		namespace Hooks
		{
			#define NOP_BYTES(size) std::vector<std::uint8_t>(size, 0x90)

			void HookPipboyDataPopulateItemCardInfo(PipboyInventoryData* a_pipboyInventoryData, const BGSInventoryItem* a_inventoryItem, const BGSInventoryItem::Stack* a_stack, PipboyObject* a_data)
			{
				a_pipboyInventoryData->PopulateItemCardInfo(a_inventoryItem, a_stack, a_data);

				if (a_stack->extra->HasType(EXTRA_DATA_TYPE::kHealth))
				{
					PipboyArray* pipboyArray = a_data->GetMember<PipboyArray*>(BSFixedString("itemCardInfoList"));
					const BSFixedStringCS cnd = BSFixedStringCS("CND");
					a_pipboyInventoryData->AddItemCardInfoEntry(&cnd, a_stack->extra->GetHealthPerc() * 100.0f, pipboyArray);

					std::uint32_t cndIndexPosition = a_inventoryItem->object->GetFormType() == ENUM_FORM_ID::kWEAP ? 1 : 0;

					PipboyValue* cndEntry = std::move(pipboyArray->elements.back());
					for (std::size_t i = pipboyArray->elements.size() - 1; i > cndIndexPosition; --i)
					{
						pipboyArray->elements[i] = std::move(pipboyArray->elements[i - 1]);
					}
					pipboyArray->elements[cndIndexPosition] = std::move(cndEntry);
				}
				return;
			}

			std::int64_t HookActorUtilsArmorRatingVisitorBaseOperator(ActorUtils::ArmorRatingVisitorBase* a_this, BGSInventoryItem* a_item, std::uint32_t a_stackID)
			{
				std::int64_t result = a_this->operator()(a_item, a_stackID);

				BGSInventoryItem::Stack* stack = nullptr;

				// Retail logic followed here, don't ask.
				if (a_stackID == -1)
				{
					BGSInventoryItem::Stack* pointer = a_item->stackData.get();
					while (pointer)
					{
						// BGSInventoryItem::Stack::Flag::kSlotMask = 7
						if (!a_this->checkEquipped || (stack ==	 nullptr && (pointer->flags.underlying() & 7) != 0))
						{
							stack = pointer;
						}

						if (stack)
						{
							break;
						}

						pointer = pointer->nextStack.get();
					}
				}
				else
				{
					stack = a_item->GetStackByID(a_stackID);
				}

				if (stack && stack->extra && stack->extra->HasType(EXTRA_DATA_TYPE::kHealth))
				{
					float condition = stack->extra->GetHealthPerc();
					a_this->rating = a_this->rating * (0.66f + std::min((0.34f * condition) / 0.5f, 0.34f));
				}

				return result;
			}

			bool HookFavoritesManagerUseQuickkeyItem(FavoritesManager* a_this, std::uint32_t a_quickkeyIndex)
			{
				REX::DEBUG("'FavoritesManager::UseQuickkeyItem' - quickkey index: {}.", a_quickkeyIndex);
				if (a_quickkeyIndex == 1)
				{
					return AmmoSwitch::InitializeAmmoSwitch();
				}
				else
				{
					return a_this->UseQuickkeyItem(a_quickkeyIndex);
				}
			}

			void HookWorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu()
			{
				GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
				SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sCannotBuildMessage")->GetString().data(), nullptr, true, true);
				return;
			}

			void Install()
			{
				auto& trampoline = REL::GetTrampoline();

				// GetInfoForPlayerDialogueOptionHook - { ID 2196817 + 0x40A }
				typedef TESTopicInfo(GetCurrentTopicInfo_Player_Sig)(BGSSceneActionPlayerDialogue* apPlayerDialogue, BGSScene* apParentScene, TESObjectREFR* apTarget, std::uint32_t aeType);
				REL::Relocation<GetCurrentTopicInfo_Player_Sig> GetCurrentTopicInfo_Player_Location{ REL::ID(2196817), 0x40A };
				trampoline.write_call<5>(GetCurrentTopicInfo_Player_Location.address(), &RE::GetCurrentTopicInfo_Player_Hook);

				// GetInfoForNPCResponseDialogOptionHook - { ID 2196817 + 0x7A1 }
				typedef TESTopicInfo(GetCurrentTopicInfo_NPC_Sig)(BGSSceneActionPlayerDialogue* apPlayerDialogue, BGSScene* apParentScene, TESObjectREFR* apTarget, std::uint32_t aeType);
				REL::Relocation<GetCurrentTopicInfo_NPC_Sig> GetCurrentTopicInfo_NPC_Location{ REL::ID(2196817), 0x7A1 };
				trampoline.write_call<5>(GetCurrentTopicInfo_NPC_Location.address(), &RE::GetCurrentTopicInfo_NPC_Hook);

				// GetNPCResponseInfoForOptionHook - { ID 2196800 + 0x388 }
				typedef TESTopicInfo(GetCurrentTopicInfo_NPCAction_Sig)(BGSSceneActionNPCResponseDialogue* apNPCDialogue, BGSScene* apParentScene);
				REL::Relocation<GetCurrentTopicInfo_NPCAction_Sig> GetCurrentTopicInfo_NPCAction_Location{ REL::ID(2196800), 0x388 };
				trampoline.write_call<5>(GetCurrentTopicInfo_NPCAction_Location.address(), &RE::GetCurrentTopicInfo_NPCAction_Hook);

				// PopulateItemCardInfo - { ID 2225264 + 0x651 }
				typedef void(PopulateItemCardInfo1_Sig)(PipboyInventoryData* a_pipboyInventoryData, const BGSInventoryItem* a_inventoryItem, const BGSInventoryItem::Stack* a_stack, PipboyObject* a_data);
				REL::Relocation<PopulateItemCardInfo1_Sig> PopulateItemCardInfo1_Location{ REL::ID(2225264), 0x651 };
				trampoline.write_jmp<5>(PopulateItemCardInfo1_Location.address(), &HookPipboyDataPopulateItemCardInfo);

				// PopulateItemCardInfo - { ID 2225279 + 0x40F }
				typedef void(PopulateItemCardInfo2_Sig)(PipboyInventoryData* a_pipboyInventoryData, const BGSInventoryItem* a_inventoryItem, const BGSInventoryItem::Stack* a_stack, PipboyObject* a_data);
				REL::Relocation<PopulateItemCardInfo2_Sig> PopulateItemCardInfo2_Location{ ID::PipboyInventoryData::RepopulateItemCardsOnSection, 0x40F };
				trampoline.write_call<5>(PopulateItemCardInfo2_Location.address(), &HookPipboyDataPopulateItemCardInfo);

				// InventoryUserUIUtils::PopulateItemCardInfo_Helper - { ID 2222625 + 0x1226 }
				REL::Relocation<std::uintptr_t> InventoryUserUIUtilsPopulateItemCardInfo_Helper_NOP{ REL::ID(2222625), 0x1226 };
				InventoryUserUIUtilsPopulateItemCardInfo_Helper_NOP.write_fill(REL::NOP, 5);

				// PipboyInventoryData::PopulateItemCardInfo - { 2225266 + 0x8CD }
				REL::Relocation<std::uintptr_t> PipboyInventoryDataPopulateItemCardInfo_NOP{ ID::PipboyInventoryData::PopulateItemCardInfo, 0x8CD };
				PipboyInventoryDataPopulateItemCardInfo_NOP.write_fill(REL::NOP, 5);

				// HUDExperienceMeter::UpdateDisplayObject - { 2220382 + 0x221 }
				REL::Relocation<std::uintptr_t> HUDExperienceMeterUpdateDisplayObject_NOP_1{ REL::ID(2220382), 0x221 };
				HUDExperienceMeterUpdateDisplayObject_NOP_1.write_fill(REL::NOP, 5);

				// HUDExperienceMeter::UpdateDisplayObject - { 2220382 + 0x26B }
				REL::Relocation<std::uintptr_t> HUDExperienceMeterUpdateDisplayObject_NOP_2{ REL::ID(2220382), 0x26B };
				HUDExperienceMeterUpdateDisplayObject_NOP_2.write_fill(REL::NOP, 5);

				// Actor::GetDesirability - { 2229946 + 0x56 } - .984
				REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_1{ REL::ID(2229946), 0x56 };
				trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_1.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

				// Actor::CalcArmorRating - { 2230008 + 0xFC } - .984
				REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_2{ REL::ID(2230008), 0xFC };
				trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_2.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

				// Actor::CalcArmorRating - { 2230009 + 0x2B } - .984
				REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_3{ REL::ID(2230009), 0x2B };
				trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_3.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

				// sub_140BFBE70 - { 2230010 + 0x13C } - .984 - TODO: Further RE func needed, inlined in .980? 
				REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_4{ REL::ID(2230010), 0x13C };
				trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_4.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

				// CombatBehaviourFindObject::EvaluateArmor { 2241004 + 0x4ED } - .984
				REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_5{ REL::ID(2241004), 0x4ED };
				trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_5.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

				// CombatBehaviourFindObject::EvaluateArmor { 2241004 + 0x579 }
				REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_6{ REL::ID(2241004), 0x579 };
				trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_6.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

				// FavoritesManager::UseQuickkeyItem - { ID 2248740 + 0x5B }
				REL::Relocation<std::uintptr_t> FavoritesManagerUseQuickkeyItem_1{ REL::ID(2248740), 0x5B };
				trampoline.write_call<5>(FavoritesManagerUseQuickkeyItem_1.address(), &HookFavoritesManagerUseQuickkeyItem);

				// FavoritesManager::UseQuickkeyItem - { ID 2248766 + 0x68 }
				REL::Relocation<std::uintptr_t> FavoritesManagerUseQuickkeyItem_2{ REL::ID(2248766), 0x68 };
				trampoline.write_call<5>(FavoritesManagerUseQuickkeyItem_2.address(), &HookFavoritesManagerUseQuickkeyItem);

				// WorkbenchMenuBase::ShowBuildFailureMessage { ID 2224322 + 0x2C5 } - .980
				REL::Relocation<std::uintptr_t> WorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu{ REL::ID(2224322), 0x2C5 };
				trampoline.write_jmp<5>(WorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu.address(), &HookWorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu);
			}

			DetourXS hook_ShowBuildFailureMessage;
			typedef void(ShowBuildFailureMessageSig)(WorkbenchMenuBase*);

			void HookWorkbenchMenuBaseShowBuildFailureMessage(WorkbenchMenuBase* a_this)
			{
				if (a_this->repairing)
				{
					Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

					std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
					if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
					{
						InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
						const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

						if (inventoryItem)
						{
							BSTArray<BSTTuple<TESForm*, BGSTypedFormValuePair::SharedVal>>* requiredItems = a_this->QCurrentModChoiceData()->recipe->requiredItems;
							if (requiredItems)
							{
								RE::RepairFailureCallback* repairFailureCallback = new RE::RepairFailureCallback(examineMenu.get());
								RE::ExamineConfirmMenu::InitDataRepairFailure* initDataRepair = new RE::ExamineConfirmMenu::InitDataRepairFailure(requiredItems);

								BSTHashMap<TESBoundObject*, std::uint32_t> availableComponents;

								BSTTuple<TESBoundObject*, std::uint32_t> insertObject;

								std::uint32_t size = requiredItems->size();
								if (size)
								{
									for (std::uint32_t i = 0; i < size; ++i) {
										const auto& tuple = requiredItems->at(i);
										TESForm* form = tuple.first;
										const auto& value = tuple.second;

										if (!form) continue;

										if (!form->IsBoundObject()) continue;

										TESObjectREFR* sharedContainerREF = examineMenu.get()->sharedContainerRef.get();
										BGSInventoryList* inventoryList = sharedContainerREF->inventoryList;

										if (inventoryList)
										{
											TESBoundObject* object = reinterpret_cast<TESBoundObject*>(form);

											//REX::DEBUG("'sharedContainerREF' has inventory list.");
											std::uint32_t availableCount = GetAvailableComponentCount(inventoryList, form);
											insertObject = { object, availableCount };
										}
										else
										{
											BaseFormComponent* container = sharedContainerREF->HasContainer();
											if (container)
											{
												// TODO - handling needed here.
												REX::DEBUG("'sharedContainerREF' has container - MISSING HANDLING");
											}
										}

										availableComponents.insert(insertObject);										
									}
									initDataRepair->availableComponents = availableComponents;
								}

								examineMenu->ShowConfirmMenu(initDataRepair, repairFailureCallback);
							}
							a_this->repairing = false;
						}
					}
				}
				else
				{
					GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
					SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sCannotBuildMessage")->GetString().data(), nullptr, true, true);
				}
			}

			DetourXS hook_GetBuildConfirmQuestion;
			typedef void(GetBuildConfirmQuestionSig)(RE::ExamineMenu*, char*, std::uint32_t);

			void HookExamineMenuGetBuildConfirmQuestion(RE::ExamineMenu* a_this, char* a_buffer, std::uint32_t a_bufferLength)
			{
				const WorkbenchMenuBase::ModChoiceData* modChoiceData;
				const char* fullName;
				const char* type;

				if (a_this->QCurrentModChoiceData()->recipe)
				{
					modChoiceData = a_this->QCurrentModChoiceData();
					if (a_this->repairing)
					{
						type = "$$Repair";
						std::uint32_t selectedIndex = a_this->GetSelectedIndex();
						if (!a_this->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < a_this->invInterface.stackedEntries.size())
						{
							InventoryUserUIInterfaceEntry* inventoryUUIEntry = (a_this->invInterface.stackedEntries.data() + selectedIndex);
							const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

							fullName = inventoryItem->GetDisplayFullName(inventoryUUIEntry->stackIndex.at(0));
						}
					}
					else
					{
						type = "$$Make";
						fullName = TESFullName::GetFullName(*modChoiceData->recipe->createdItem).data();
					}
					snprintf(a_buffer, a_bufferLength, "%s %s?", type, fullName);
				}
			}

			DetourXS hook_SetHealthPerc;
			typedef void(SetHealthPercSig)(ExtraDataList*, float);
			REL::Relocation<SetHealthPercSig> SetHealthPercOriginal;

			// Required to allow the game to store a value of 1.0, by default it would set it to '-1.0'
			// This causes issues in the way we handle initialization of the health data.
			// Here we simply, run the original function, then if 'a_health' was '1.0', we manually set it 
			// again to '1.0', reverting the original functions result of '-1.0'.
			void HookExtraDataListSetHealthPerc(ExtraDataList* a_this, float a_health)
			{
				SetHealthPercOriginal(a_this, a_health);
				if (a_health == 1.0f && a_this->GetHealthPerc() != 1.0f)
				{
					a_this->AddExtra(new ExtraHealth(1.0f));
				}
			}

			DetourXS hook_QCurrentModChoiceData;
			typedef const WorkbenchMenuBase::ModChoiceData* (QCurrentModChoiceDataSig)(WorkbenchMenuBase*);

			const WorkbenchMenuBase::ModChoiceData* HookWorkbenchMenuBaseQCurrentModChoiceData(WorkbenchMenuBase* a_this)
			{
				if (a_this->repairing)
				{
					// TODO: Merge this into singular function returning dynamic repair cost.
					if (typeid(*a_this) == typeid(PowerArmorModMenu))
					{
						PowerArmorModMenu* powerArmorModMenu = dynamic_cast<PowerArmorModMenu*>(a_this);
						return &powerArmorModMenu->repairData;
					}
					else
					{
						std::uint32_t modChoiceIndex = a_this->modChoiceIndex;
						if (modChoiceIndex >= a_this->modChoiceArray.size())
						{
							return 0;
						}
						else
						{
							WorkbenchMenuBase::ModChoiceData* currentModChoiceData = (a_this->modChoiceArray.data() + modChoiceIndex);

							// Remove any possible required perks, as we don't take that into account when repairing.
							if (currentModChoiceData->requiredPerks.size() > 0)
							{
								currentModChoiceData->requiredPerks.clear();
							}

							// Remove any possible conditions on the recipe, as we don't take that into account when repairing.
							currentModChoiceData->recipe->conditions.ClearAllConditionItems();

							return (currentModChoiceData);
						}
					}
				}
				else // Retail logic come into play here.
				{
					std::uint32_t modChoiceIndex = a_this->modChoiceIndex;
					if (modChoiceIndex >= a_this->modChoiceArray.size())
					{
						return 0;
					}
					else
					{
						return (a_this->modChoiceArray.data() + modChoiceIndex);
					}
				}
			}

			DetourXS hook_GetInventoryValue;
			typedef std::int64_t(GetInventoryValueSig)(TESBoundObject*, const ExtraDataList*);
			REL::Relocation<GetInventoryValueSig> GetInventoryValueOriginal;

			std::int64_t HookBGSInventoryItemUtilsGetInventoryValue(TESBoundObject* a_baseObj, const ExtraDataList* a_extra)
			{
				if (!a_extra->HasType(EXTRA_DATA_TYPE::kHealth))
				{
					return GetInventoryValueOriginal(a_baseObj, a_extra);
				}

				ExtraDataList* non_const_a_extra = const_cast<ExtraDataList*>(a_extra);

				std::int64_t newValue;

				// Clamp to 5% value at minimum.
				if (non_const_a_extra->GetHealthPerc() < 0.05f)
				{
					newValue = std::int64_t(GetInventoryValueOriginal(a_baseObj, a_extra) * 0.05f * std::sqrt(0.05f));
				}
				else
				{
					newValue = std::int64_t(GetInventoryValueOriginal(a_baseObj, a_extra) * non_const_a_extra->GetHealthPerc() * std::sqrt(non_const_a_extra->GetHealthPerc()));
				}
				return newValue;
			}

			DetourXS hook_AddItem;
			typedef void(AddItemSig)(BGSInventoryList*, TESBoundObject*, const BGSInventoryItem::Stack*, std::uint32_t*, std::uint32_t*);
			REL::Relocation<AddItemSig> AddItemOriginal;

			// All health initialization occurs here, this triggers both when a NPC is spawned with objects, added to inventories through
			// a command or any other way, items also stay uninitialized until picked up saving a 'float' per object from player saves until
			// they're picked up and serialized.
			void HookBGSInventoryListAddItem(BGSInventoryList* a_this, TESBoundObject* a_boundObject, const BGSInventoryItem::Stack* a_stack, std::uint32_t* a_oldCount, std::uint32_t* a_newCount)
			{
				ENUM_FORM_ID formType = a_boundObject->GetFormType();
				if (a_boundObject)
				{
					if (formType == ENUM_FORM_ID::kWEAP || formType == ENUM_FORM_ID::kARMO)
					{
						std::uint32_t iterCount = 0;
						for (const BGSInventoryItem::Stack* traverse = a_stack; traverse; traverse->nextStack)
						{
							if (!traverse || !traverse->extra)
							{
								break;
							}

							if (formType == ENUM_FORM_ID::kWEAP)
							{
								TESObjectWEAP* tempREFR = static_cast<TESObjectWEAP*>(a_boundObject);
								if (tempREFR->weaponData.type == WEAPON_TYPE::kGrenade || tempREFR->weaponData.type == WEAPON_TYPE::kMine)
								{
									REX::DEBUG("BGSInventoryList::AddItem: REFR grenade/mine weapon type.");
									break;
								}

								// Set to '1.0' when initializing if the 'noDegradation' keyword is on the object.
								if (tempREFR->HasKeyword(Shared::noDegradation))
								{
									REX::DEBUG("'CAS_NoDegradation' keyword found on weapon: {}.", tempREFR->GetFormEditorID());
									break;
								}
							}

							if (formType == ENUM_FORM_ID::kARMO)
							{
								TESObjectARMO* tempREFR = static_cast<TESObjectARMO*>(a_boundObject);
								// Set to '1.0' when initializing if the 'noDegradation' keyword is on the object.

								if (tempREFR->armorData.rating == 0 && !tempREFR->armorData.damageTypes)
								{
									break;
								}

								if (tempREFR->HasKeyword(Shared::noDegradation))
								{
									REX::DEBUG("'CAS_NoDegradation' keyword found on armor: {}.", tempREFR->GetFormEditorID());
									break;
								}
							}

							// GetHealthPerc returns -1.0 if it can't find the 'kHealth' type.
							if (traverse->extra->GetHealthPerc() < 0.0f)
							{
								traverse->extra->SetHealthPerc(BSRandom::Float(0.45f, 0.85f));
								break;
							}

							iterCount++;

							if (iterCount > traverse->count - 1)
							{
								break;
							}
						}
					}
				}
				AddItemOriginal(a_this, a_boundObject, a_stack, a_oldCount, a_newCount);
			}

			DetourXS hook_ExamineMenuBuildConfirmed;
			typedef void(ExamineMenuBuildConfirmedSig)(ExamineMenu*, bool);
			REL::Relocation<ExamineMenuBuildConfirmedSig> ExamineMenuBuildConfirmedOriginal;

			void HookExamineMenuBuildConfirmed(ExamineMenu* a_this, bool a_ownerIsWorkbench)
			{
				if (a_this->repairing)
				{
					std::uint32_t selectedIndex = a_this->GetSelectedIndex();
					if (!a_this->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < a_this->invInterface.stackedEntries.size())
					{
						InventoryUserUIInterfaceEntry* inventoryUUIEntry = (a_this->invInterface.stackedEntries.data() + selectedIndex);
						const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

						if (inventoryItem)
						{
							switch (a_this->GetCurrentObj()->formType.get())
							{
							case ENUM_FORM_ID::kARMO:
							case ENUM_FORM_ID::kWEAP:
								BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));

								if (stack)
								{
									stack->extra->SetHealthPerc(1.0f);

									BGSInventoryItem::CheckStackIDFunctor compareFunction(inventoryUUIEntry->stackIndex.at(0));
									BGSInventoryItem::SetHealthFunctor writeFunction(stack->extra->GetHealthPerc());
									writeFunction.shouldSplitStacks = 0x101;

									PlayerCharacter::GetSingleton()->FindAndWriteStackDataForInventoryItem(a_this->GetCurrentObj(), compareFunction, writeFunction);

									BGSDefaultObject* craftingSound = TESDataHandler::GetSingleton()->LookupForm<BGSDefaultObject>(0x112622, "Fallout4.esm");

									a_this->ConsumeSelectedItems(true, craftingSound->GetForm<BGSSoundDescriptorForm>());
									a_this->UpdateOptimizedAutoBuildInv();
									selectedIndex = a_this->GetSelectedIndex();
									a_this->UpdateItemList(selectedIndex);
									a_this->uiMovie->Invoke("RefreshList", nullptr, nullptr, 0);
									a_this->CreateModdedInventoryItem();
									a_this->UpdateItemCard(false);
									a_this->repairing = false;
									a_this->uiMovie->Invoke("UpdateButtons", nullptr, nullptr, 0); // TODO - update custom buttons for this functionality.
								}
							}
						}
					}
				}
				else
				{
					ExamineMenuBuildConfirmedOriginal(a_this, a_ownerIsWorkbench);
				}
			}

			DetourXS hook_TESObjectWEAPFire;
			typedef void(TESObjectWEAPFireSig)(const BGSObjectInstanceT<TESObjectWEAP>*, TESObjectREFR*, BGSEquipIndex, TESAmmo*, AlchemyItem*);
			REL::Relocation<TESObjectWEAPFireSig> TESObjectWEAPFireOriginal;

			void HookTESObjectWEAPFire(const BGSObjectInstanceT<TESObjectWEAP>* a_weapon, TESObjectREFR* a_source, BGSEquipIndex a_equipIndex, TESAmmo* a_ammo, AlchemyItem* a_poison)
			{
				// Make sure 'a_source' is valid.
				if (!a_source)
				{
					return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
				}

				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

				// Only extend logic if the source is the player.
				if (a_source != playerCharacter)
				{
					return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
				}

				// God Mod - no degradation.
				if (playerCharacter->IsGodMode())
				{
					return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
				}

				TESObjectWEAP* weapon = (TESObjectWEAP*)a_weapon->object;

				// No degradation to mines/grenades.
				if (weapon->weaponData.type == WEAPON_TYPE::kGrenade || weapon->weaponData.type == WEAPON_TYPE::kMine)
				{
					return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
				}

				// No degradation on objects with 'noDegradation' keyword.
				if (weapon->HasKeyword(Shared::noDegradation))
				{
					return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
				}

				TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);

				BGSInventoryItem* inventoryItem = nullptr;;
				TESFormID weaponFormID = a_weapon->object->GetFormID();
				for (BGSInventoryItem& item : playerCharacter->inventoryList->data)
				{
					if (item.object->GetFormID() == weaponFormID)
					{
						inventoryItem = &item;
						break;
					}
				}

				if (inventoryItem)
				{
					EquippedItem& equippedWeapon = playerCharacter->currentProcess->middleHigh->equippedItems[0];

					// If ammo is mapped to a degradation value, override default value of 1%.
					float conditionReduction = 0.01f;
					TESObjectWEAP::InstanceData* data = (TESObjectWEAP::InstanceData*)(a_weapon->instanceData.get());

					TESAmmo* ammoInstance = data->ammo;

					auto it = ammoDegradationMap.find(ammoInstance);
					if (it != ammoDegradationMap.end())
					{
						conditionReduction = it->second;
					}

					if (conditionReduction == 0.01f)
					{
						REX::WARN("TESObjectWEAP::Fire - Ammo type '{}' is not declared in 'ammoDegradationMap'.", ammoInstance->GetFormEditorID());
					}

					// TODO: Rework these numbers after gameplay tests.
					std::uint32_t flags = data->flags.underlying();
					if (flags & std::uint32_t(WEAPON_FLAGS::kAutomatic))
					{
						conditionReduction *= 0.5f;
					}
					else if (flags & std::uint32_t(WEAPON_FLAGS::kBoltAction))
					{
						conditionReduction *= 2.0f;
					}

					// Reduces damage to weapon depending on players relevant weapon skill level.
					// Linear reduction from 0 - 100, 100 resulting in 20% less damage to weapon.
					float gunsSkillValue = 0.0f;
					ActorValueInfo* gunSkill = data->skill;

					if (gunSkill)
					{
						REX::DEBUG("Weapon '{}' set to skill: {}.", a_weapon->object->GetFormEditorID(), gunSkill->GetFormEditorID());
						gunsSkillValue = playerCharacter->GetActorValue(*gunSkill);
					}
					else {
						REX::DEBUG("Weapon '{}' is missing skill value - please correct this in the weapon record in XEdit.", a_weapon->object->GetFormEditorID());
					}
					
					float reductionPercentFromSkill = (gunsSkillValue / 100.0f) * 0.2;
					conditionReduction *= (1.0f - reductionPercentFromSkill);

					ExtraDataList* extraDataList = inventoryItem->stackData->extra.get();

					float currentHealth = extraDataList->GetHealthPerc();
					float newHealth = std::max(currentHealth - conditionReduction, 0.0f);

					if (newHealth == 0.0f)
					{
						ActorEquipManager::GetSingleton()->UnequipItem(playerCharacter, &equippedWeapon, false);
						GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
						SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sWeaponBreak")->GetString().data(), "UIWorkshopModeItemScrapGeneric", true, true);
					}
					extraDataList->SetHealthPerc(newHealth);
					PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardsOnSection(ENUM_FORM_ID::kWEAP);
				}
				return;
			}

			DetourXS hook_CombatFormulasCalcWeaponDamage;
			typedef float(CombatFormulasCalcWeaponDamageSig)(const TESForm*, const TESObjectWEAP::InstanceData*, const TESAmmo*, float, float);
			REL::Relocation<CombatFormulasCalcWeaponDamageSig> CombatFormulasCalcWeaponDamageOriginal;

			float HookCombatFormulasCalcWeaponDamage(const TESForm* a_actorForm, const TESObjectWEAP::InstanceData* a_weapon, const TESAmmo* a_ammo, float a_condition, float a_damageMultiplier)
			{
				float retailDamage = CombatFormulasCalcWeaponDamageOriginal(a_actorForm, a_weapon, a_ammo, a_condition, a_damageMultiplier);
				if (a_condition != -1.0f)
				{
					retailDamage = retailDamage * (0.5f + std::min((0.5f * a_condition) / 0.75, 0.5));
				}
				return retailDamage;
			}

			DetourXS hook_GetEquippedArmorDamageResistance;
			typedef float(GetEquippedArmorDamageResistanceSig)(Actor*, const ActorValueInfo*);
			REL::Relocation<GetEquippedArmorDamageResistanceSig> GetEquippedArmorDamageResistanceOriginal;

			float HookGetEquippedDamageResistance(Actor* a_actor, const ActorValueInfo* a_info)
			{
				float retailDamageResistance = GetEquippedArmorDamageResistanceOriginal(a_actor, a_info);

				if (a_actor != PlayerCharacter::GetSingleton())
				{
					return retailDamageResistance;
				}
				else
				{
					BGSInventoryList* inventoryList = a_actor->inventoryList;
					inventoryList->rwLock.lock_read();
					std::uint32_t inventoryListSize = inventoryList->data.size();

					for (BGSInventoryItem& item : inventoryList->data)
					{
						if (item.IsEquipped(0))
						{
							TESObjectARMO* armor = static_cast<TESObjectARMO*>(item.object);
							if (item.object && armor->formType == ENUM_FORM_ID::kARMO && armor->Protects(a_info, false))
							{
								ExtraDataList* extraDataList = item.stackData->extra.get();
								REX::DEBUG("Hit item: {}", item.object->GetFormEditorID());
								REX::DEBUG("Item condition: {}", extraDataList->GetHealthPerc());
								REX::DEBUG("Damage resistance prior to condition modifier: {}", retailDamageResistance);
								float conditionModifier = 0.66f + std::min((0.34f * extraDataList->GetHealthPerc()) / 0.5f, 0.34f);
								retailDamageResistance *= conditionModifier;
								REX::DEBUG("Damage resistance after condition modifier: {}", retailDamageResistance);
								break;
							}
						}
					}

					inventoryList->rwLock.unlock_read();
					return retailDamageResistance;
				}
			}

			DetourXS hook_IUUIIUtilsAddItemCardInfoEntry;
			typedef void(IUUIIUtilsAddItemCardInfoEntrySig)(Scaleform::GFx::Value&, Scaleform::GFx::Value&, const BSFixedStringCS&, Scaleform::GFx::Value&, float, float, float);
			REL::Relocation<IUUIIUtilsAddItemCardInfoEntrySig> IUUIIUtilsAddItemCardInfoEntryOriginal;

			void HookIUUIIUtilsAddItemCardInfoEntry(Scaleform::GFx::Value& a_array, Scaleform::GFx::Value& a_newEntry, const BSFixedStringCS& a_textID, Scaleform::GFx::Value& a_value, float a_difference, float a_totalDamage, float a_compareDamage)
			{
				IUUIIUtilsAddItemCardInfoEntryOriginal(a_array, a_newEntry, a_textID, a_value, a_difference, a_totalDamage, a_compareDamage);

				if (std::string(a_textID.c_str()) == "CND")
				{
					bool showAsPercent = true;
					std::uint32_t precision = 1;

					SetMemberAS3(a_newEntry, "showAsPercent", showAsPercent);
					SetMemberAS3(a_newEntry, "precision", precision);
				}
			}

			DetourXS hook_PipboyInventoryDataBaseAddItemCardInfoEntry;
			typedef PipboyObject* (PipboyInventoryDataBaseAddItemCardInfoEntrySig)(PipboyInventoryData*, const BSFixedStringCS*, PipboyArray*);
			REL::Relocation<PipboyInventoryDataBaseAddItemCardInfoEntrySig> PipboyInventoryDataBaseAddItemCardInfoEntryOriginal;

			PipboyObject* HookPipboyInventoryDataBaseAddItemCardInfoEntry(PipboyInventoryData* a_inventory, const BSFixedStringCS* a_textID, PipboyArray* a_array)
			{
				PipboyObject* result = PipboyInventoryDataBaseAddItemCardInfoEntryOriginal(a_inventory, a_textID, a_array);

				if (std::string(a_textID->c_str()) == "CND")
				{
					PipboyPrimitiveValue<bool>* showAsPercent = new PipboyPrimitiveValue<bool>(true, result);
					PipboyPrimitiveValue<std::uint32_t>* precision = new PipboyPrimitiveValue<std::uint32_t>(1, result);

					const BSFixedString showAsPercentString = BSFixedString("showAsPercent");
					const BSFixedString precisionString = BSFixedString("precision");

					result->AddMember(&showAsPercentString, showAsPercent);
					result->AddMember(&precisionString, precision);
				}

				return result;
			}

			DetourXS hook_IUUIIUtilsPopulateItemCardInfo_Helper;
			typedef void(IUUIIUtilsPopulateItemCardInfo_HelperSig)(Scaleform::GFx::Value*, const BGSInventoryItem*, std::uint32_t, BSScrapArray<BSTTuple<BGSInventoryItem const*, std::uint32_t>>, bool);
			REL::Relocation<IUUIIUtilsPopulateItemCardInfo_HelperSig> IUUIIUtilsPopulateItemCardInfo_HelperOriginal;

			void HookIUUIIUtilsPopulateItemCardInfo_Helper(Scaleform::GFx::Value* a_menuObj, const BGSInventoryItem* a_item, std::uint32_t a_itemStackID, BSScrapArray<BSTTuple<BGSInventoryItem const*, std::uint32_t>> a_comparisonItems, bool a_forceArmorCompare)
			{
				IUUIIUtilsPopulateItemCardInfo_HelperOriginal(a_menuObj, a_item, a_itemStackID, a_comparisonItems, a_forceArmorCompare);

				if (!UI::GetSingleton()->GetMenuOpen("CookingMenu") && a_item->stackData->extra->HasType(EXTRA_DATA_TYPE::kHealth))
				{
					Scaleform::GFx::Value defaultArray = a_menuObj->HasMember("ItemCardInfoList") ? GetMemberAS3(*a_menuObj, "ItemCardInfoList") : *a_menuObj;

					float condition = a_item->stackData->extra->GetHealthPerc();
					InventoryUserUIUtils::AddItemCardInfoEntry(defaultArray, "CND", condition * 100.0f);

					std::uint32_t cndIndexPosition = a_item->object->GetFormType() == ENUM_FORM_ID::kWEAP ? 1 : 0;

					Scaleform::GFx::Value cndEntry;
					defaultArray.GetElement(defaultArray.GetArraySize() - 1, &cndEntry);
					for (std::uint32_t i = defaultArray.GetArraySize() - 1; i > cndIndexPosition; --i)
					{
						Scaleform::GFx::Value tempEntry;
						defaultArray.GetElement(i - 1, &tempEntry);
						defaultArray.SetElement(i, tempEntry);
					}

					defaultArray.SetElement(cndIndexPosition, cndEntry);
				}
				return;
			}

			DetourXS hook_PipboyInventoryUtilsFillResistTypeInfo;
			typedef void(PipboyInventoryUtilsFillResistTypeInfoSig)(const BGSInventoryItem*, const BGSInventoryItem::Stack*, BSScrapArray<BSTTuple<std::uint32_t, float>>*, float);
			REL::Relocation<PipboyInventoryUtilsFillResistTypeInfoSig> PipboyInventoryUtilsFillResistTypeInfo_Original;

			void HookPipboyInventoryUtilsFillResistTypeInfo(const BGSInventoryItem* a_item, const BGSInventoryItem::Stack* a_stack, BSScrapArray<BSTTuple<std::uint32_t, float>>* a_resistValuesPerType, float a_scale)
			{
				PipboyInventoryUtilsFillResistTypeInfo_Original(a_item, a_stack, a_resistValuesPerType, a_scale);

				// Return early if need be.
				if (!a_stack->extra || !a_stack->extra->HasType(EXTRA_DATA_TYPE::kHealth))
				{
					return;
				}
				float condition = a_stack->extra->GetHealthPerc();
				for (std::uint32_t type = 0; type < 6; type++)
				{
					float resistance = a_resistValuesPerType->at(type).second;
					if (resistance != 0.0f && type == 0)
					{
						a_resistValuesPerType->at(type).second = resistance * (0.66f + std::min(0.34 * condition / 0.5, 0.34));
					}
				}
			}

			// ========== REGISTERS ==========
			void RegisterGetBuildConfirmQuestion()
			{
				REL::Relocation<GetBuildConfirmQuestionSig> functionLocation{ REL::ID(2223057) };
				if (hook_GetBuildConfirmQuestion.Create(reinterpret_cast<void*>(functionLocation.address()), &HookExamineMenuGetBuildConfirmQuestion))
				{
					REX::DEBUG("Installed 'ExamineMenu::GetBuildConfirmQuestion' hook.");
				}
				else
				{
					REX::CRITICAL("Failed to hook 'ExamineMenu::GetBuildConfirmQuestion', exiting.");
				}
			}

			void RegisterShowBuildFailureMessage()
			{
				REL::Relocation<ShowBuildFailureMessageSig> functionLocation{ REL::ID(2224959) };
				if (hook_ShowBuildFailureMessage.Create(reinterpret_cast<void*>(functionLocation.address()), &HookWorkbenchMenuBaseShowBuildFailureMessage))
				{
					REX::DEBUG("Installed 'WorkbenchMenuBase::ShowBuildFailureMessage' hook.");
				}
				else
				{
					REX::CRITICAL("Failed to hook 'WorkbenchMenuBase::ShowBuildFailureMessage', exiting.");
				}
			}

			void RegisterSetHealthPercHook()
			{
				REL::Relocation<SetHealthPercSig> functionLocation{ ID::ExtraDataList::SetHealthPerc };
				if (hook_SetHealthPerc.Create(reinterpret_cast<void*>(functionLocation.address()), &HookExtraDataListSetHealthPerc))
				{
					REX::DEBUG("Installed 'ExtraDataList::SetHealthPerc' hook.");
					SetHealthPercOriginal = reinterpret_cast<uintptr_t>(hook_SetHealthPerc.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'ExtraDataList::SetHealthPerc', exiting.");
				}
			}

			void RegisterAddItemHook()
			{
				REL::Relocation<AddItemSig> functionLocation{ REL::ID(2194159) };
				if (hook_AddItem.Create(reinterpret_cast<void*>(functionLocation.address()), &HookBGSInventoryListAddItem))
				{
					REX::DEBUG("Installed 'BGSInventoryList::AddItem' hook.");
					AddItemOriginal = reinterpret_cast<uintptr_t>(hook_AddItem.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'BGSInventoryList::AddItem', exiting.");
				}
			}

			void RegisterGetInventoryValueHook()
			{
				REL::Relocation<GetInventoryValueSig> functionLocation{ REL::ID(2194127) };
				if (hook_GetInventoryValue.Create(reinterpret_cast<void*>(functionLocation.address()), &HookBGSInventoryItemUtilsGetInventoryValue))
				{
					REX::DEBUG("Installed 'BGSInventoryItemUtils::GetInventoryValue' hook.");
					GetInventoryValueOriginal = reinterpret_cast<uintptr_t>(hook_GetInventoryValue.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'BGSInventoryItemUtils::GetInventoryValue', exiting.");
				}
			}

			void RegisterQCurrentModChoiceData()
			{
				REL::Relocation<QCurrentModChoiceDataSig> functionLocation{ ID::WorkbenchMenuBase::QCurrentModChoiceData };
				if (hook_QCurrentModChoiceData.Create(reinterpret_cast<void*>(functionLocation.address()), &HookWorkbenchMenuBaseQCurrentModChoiceData))
				{
					REX::DEBUG("Installed 'WorkbenchMenuBase::QCurrentModChoiceData' hook.");
				}
				else
				{
					REX::CRITICAL("Failed to hook 'BGSInventoryItemUtils::GetInventoryValue', exiting.");
				}
			}

			void RegisterExamineMenuBuildConfirmed()
			{
				REL::Relocation<ExamineMenuBuildConfirmedSig> functionLocation{ ID::ExamineMenu::BuildConfirmed };
				if (hook_ExamineMenuBuildConfirmed.Create(reinterpret_cast<void*>(functionLocation.address()), &HookExamineMenuBuildConfirmed))
				{
					REX::DEBUG("Installed 'ExamineMenu::BuildConfirmed' hook.");
					ExamineMenuBuildConfirmedOriginal = reinterpret_cast<uintptr_t>(hook_ExamineMenuBuildConfirmed.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'ExamineMenu::BuildConfirmed', exiting.");
				}
			}

			void RegisterTESObjectWEAPFire()
			{
				REL::Relocation<TESObjectWEAPFireSig> functionLocation{ REL::ID(2198960) };
				if (hook_TESObjectWEAPFire.Create(reinterpret_cast<void*>(functionLocation.address()), &HookTESObjectWEAPFire))
				{
					REX::DEBUG("Installed 'TESObjectWEAP::Fire' hook.");
					TESObjectWEAPFireOriginal = reinterpret_cast<uintptr_t>(hook_TESObjectWEAPFire.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'TESObjectWEAP::Fire', exiting.");
				}
			}

			void RegisterCombatFormulasCalcWeaponDamage()
			{
				REL::Relocation<CombatFormulasCalcWeaponDamageSig> functionLocation{ REL::ID(2209001) };
				if (hook_CombatFormulasCalcWeaponDamage.Create(reinterpret_cast<void*>(functionLocation.address()), &HookCombatFormulasCalcWeaponDamage))
				{
					REX::DEBUG("Installed 'CombatFormulas::CalcWeaponDamage' hook.");
					CombatFormulasCalcWeaponDamageOriginal = reinterpret_cast<uintptr_t>(hook_CombatFormulasCalcWeaponDamage.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'CombatFormulas::CalcWeaponDamage', exiting.");
				}
			}

			void RegisterGetEquippedArmorDamageResistance()
			{
				REL::Relocation<GetEquippedArmorDamageResistanceSig> functionLocation{ ID::ActorUtils::GetEquippedArmorDamageResistance };
				if (hook_GetEquippedArmorDamageResistance.Create(reinterpret_cast<void*>(functionLocation.address()), &HookGetEquippedDamageResistance))
				{
					REX::DEBUG("Installed 'ActorUtils::GetEquippedArmorDamageResistance' hook.");
					GetEquippedArmorDamageResistanceOriginal = reinterpret_cast<uintptr_t>(hook_GetEquippedArmorDamageResistance.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'ActorUtils::GetEquippedArmorDamageResistance', exiting.");
				}
			}

			void RegisterIUUIIUtilsAddItemCardInfoEntry()
			{
				REL::Relocation<IUUIIUtilsAddItemCardInfoEntrySig> functionLocation{ ID::InventoryUserUIUtils::AddItemCardInfoEntry };
				if (hook_IUUIIUtilsAddItemCardInfoEntry.Create(reinterpret_cast<void*>(functionLocation.address()), &HookIUUIIUtilsAddItemCardInfoEntry))
				{
					REX::DEBUG("Installed 'IUUIIUtils::AddItemCardInfoEntry' hook.");
					IUUIIUtilsAddItemCardInfoEntryOriginal = reinterpret_cast<uintptr_t>(hook_IUUIIUtilsAddItemCardInfoEntry.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'IUUIIUtils::AddItemCardInfoEntry', exiting.");
				}
			}

			void RegisterPipboyInventoryDataBaseAddItemsCardInfoEntry()
			{
				REL::Relocation<PipboyInventoryDataBaseAddItemCardInfoEntrySig> functionLocation{ ID::PipboyInventoryData::BaseAddItemCardInfoEntry };
				if (hook_PipboyInventoryDataBaseAddItemCardInfoEntry.Create(reinterpret_cast<void*>(functionLocation.address()), &HookPipboyInventoryDataBaseAddItemCardInfoEntry))
				{
					REX::DEBUG("Installed 'PipboyInventoryData::BaseAddItemCardInfoEntry' hook.");
					PipboyInventoryDataBaseAddItemCardInfoEntryOriginal = reinterpret_cast<uintptr_t>(hook_PipboyInventoryDataBaseAddItemCardInfoEntry.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'PipboyInventoryData::BaseAddItemCardInfoEntry', exiting.");
				}
			}

			void RegisterIUUIIUtilsPopulateItemCardInfo_Helper()
			{
				REL::Relocation<IUUIIUtilsPopulateItemCardInfo_HelperSig> functionLocation{ REL::ID(2222625) };
				if (hook_IUUIIUtilsPopulateItemCardInfo_Helper.Create(reinterpret_cast<void*>(functionLocation.address()), &HookIUUIIUtilsPopulateItemCardInfo_Helper))
				{
					REX::DEBUG("Installed 'IUUIIUtils::PopulateItemCardInfo_Helper' hook.");
					IUUIIUtilsPopulateItemCardInfo_HelperOriginal = reinterpret_cast<uintptr_t>(hook_IUUIIUtilsPopulateItemCardInfo_Helper.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'IUUIIUtils::PopulateItemCardInfo_Helper', exiting.");
				}
			}

			void RegisterPipboyInventoryUtilsFillResistTypeInfo()
			{
				REL::Relocation<PipboyInventoryUtilsFillResistTypeInfoSig> functionLocation{ REL::ID(2225235) };
				if (hook_PipboyInventoryUtilsFillResistTypeInfo.Create(reinterpret_cast<void*>(functionLocation.address()), &HookPipboyInventoryUtilsFillResistTypeInfo))
				{
					REX::DEBUG("Installed 'PipboyInventoryUtils::FillResistTypeInfo' hook.");
					PipboyInventoryUtilsFillResistTypeInfo_Original = reinterpret_cast<uintptr_t>(hook_PipboyInventoryUtilsFillResistTypeInfo.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook 'PipboyInventoryUtils::FillResistTypeInfo', exiting.");
				}
			}

			/**void RegisterActorUtilsArmorRatingVisitorBaseOperator()
			{
				REL::Relocation<ActorUtilsArmorRatingVisitorBaseOperatorSig> functionLocation{ ActorUtils::ArmorRatingVisitorBase::_operator };
				if (hook_ActorUtilsArmorRatingVisitorBaseOperator.Create(reinterpret_cast<void*>(functionLocation.address()), &HookActorUtilsArmorRatingVisitorBaseOperator))
				{
					REX::DEBUG("Installed 'ActorUtils::ArmorRatingVisitorBaseOperator' hook.");
					ActorUtilsArmorRatingVisitorBaseOperator_Original = reinterpret_cast<uintptr_t>(hook_ActorUtilsArmorRatingVisitorBaseOperator.GetTrampoline());
				}
				else
				{
					REX::CRITICAL("Failed to hook: 'ActorUtils::ArmorRatingVisitorBaseOperator', exiting.");
				}
			}*/
		}
	}
}
