#pragma once

#include "Shared/SharedFunctions.h"
#include "Systems/AmmoSwitch.h"

// Thanks to 'Jajari' and 'Pinedog' for assistance with this code.

namespace RE
{
	namespace Cascadia
	{
		using std::unordered_map;
		class BSAnimationGraphEventWatcher
		{
		public:
			typedef BSEventNotifyControl(BSAnimationGraphEventWatcher::* FnProcessEvent)(BSAnimationGraphEvent& a_event, BSTEventSource<BSAnimationGraphEvent>* a_source);

			BSEventNotifyControl ProcessEvent(BSAnimationGraphEvent& a_event, BSTEventSource<BSAnimationGraphEvent>* a_source)
			{
				const BSFixedString ReloadComplete("ReloadComplete");
				const BSFixedString reloadStateExit("reloadStateExit");

				if (a_event.tag == ReloadComplete && AmmoSwitch::switchingAmmo == true)
				{
					REX::DEBUG("'BSEventNotifyControl::ProcessEvent' - 'ReloadComplete'.");

					BGSObjectInstanceT<TESObjectWEAP>* weaponInstance = new BGSObjectInstanceT<TESObjectWEAP>{ static_cast<TESObjectWEAP*>(AmmoSwitch::equippedInstance->object), AmmoSwitch::equippedInstance->instanceData.get() };
					TESObjectWEAP::InstanceData* instanceDataWEAP = (TESObjectWEAP::InstanceData*)(weaponInstance->instanceData.get());

					if (AmmoSwitch::ammoToSwitchTo)
					{
						PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

						BGSInventoryItem* inventoryItem = nullptr;
						TESObjectWEAP* tesWEAP = (TESObjectWEAP*)AmmoSwitch::equippedInstance->object;
						TESFormID weaponFormID = tesWEAP->GetFormID();
						for (BGSInventoryItem& item : playerCharacter->inventoryList->data)
						{
							if (item.object->GetFormID() == weaponFormID)
							{
								inventoryItem = &item;
								break;
							}
						}

						// If we manage to locate currently equipped weapon instance from the stack, all  OMOD magic takes place here.
						if (inventoryItem)
						{
							auto getKeyword = [&](TESAmmo* ammo) -> BGSKeyword*
								{
									auto it = AmmoSwitch::keywordsAmmoOMODMap.find(ammo);
									return (it != AmmoSwitch::keywordsAmmoOMODMap.end()) ? it->second : nullptr;
								};

							auto applyOMOD = [&](BGSKeyword* keyword, bool add)
								{
									if (!keyword)
										return;

									auto it = AmmoSwitch::omodKeywordMap.find(keyword);
									if (it == AmmoSwitch::omodKeywordMap.end())
										return;

									BGSMod::Attachment::Mod* omod = it->second;
									bool result = false;

									BGSInventoryItem::CheckStackIDFunctor compare(0);
									BGSInventoryItem::ModifyModDataFunctor writeData(omod, 0, add, &result);

									playerCharacter->FindAndWriteStackDataForInventoryItem(tesWEAP, compare, writeData);

									REX::DEBUG("{} OMOD: {}", add ? "Added" : "Removed", result);
								};

							applyOMOD(getKeyword(instanceDataWEAP->ammo), false);
							applyOMOD(getKeyword(AmmoSwitch::ammoToSwitchTo), true);
						
							// Ammo logic itself
							instanceDataWEAP->ammo = AmmoSwitch::ammoToSwitchTo;
							playerCharacter->currentProcess->SetCurrentAmmo(BGSEquipIndex{ 0 }, AmmoSwitch::ammoToSwitchTo);
							playerCharacter->SetCurrentAmmoCount(BGSEquipIndex{ 0 }, 0);
							playerCharacter->ReloadWeapon(weaponInstance, BGSEquipIndex{ 0 });
							PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardOnSection(ENUM_FORM_ID::kWEAP);
						}
					}
						
					AmmoSwitch::switchingAmmo = false;
					AmmoSwitch::ammoToSwitchTo = nullptr;
					AmmoSwitch::equippedInstance = nullptr;
				}

				if (a_event.tag == reloadStateExit && AmmoSwitch::switchingAmmo == true)
				{
					REX::DEBUG("'BSEventNotifyControl::ProcessEvent' - 'reloadStateExit'.");
					AmmoSwitch::switchingAmmo = false;
					AmmoSwitch::ammoToSwitchTo = nullptr;
					AmmoSwitch::equippedInstance = nullptr;
				}

				FnProcessEvent fn = fnHash.at(*(uint64_t*)this);
				return fn ? (this->*fn)(a_event, a_source) : BSEventNotifyControl::kContinue;
			}

			void RegisterBSAnimationGraphEventSink()
			{
				std::uint64_t vtable = *(std::uint64_t*)this;
				auto it = fnHash.find(vtable);
				if (it == fnHash.end())
				{
					FnProcessEvent fn = SafeWrite64Function(vtable + 0x8, &BSAnimationGraphEventWatcher::ProcessEvent);
					fnHash.insert(std::pair<std::uint64_t, FnProcessEvent>(vtable, fn));
				}
			}

			static void Install()
			{
				((BSAnimationGraphEventWatcher*)((uint64_t)PlayerCharacter::GetSingleton() + 0x38))->RegisterBSAnimationGraphEventSink();
			}


		protected:
			static std::unordered_map<std::uint64_t, FnProcessEvent> fnHash;
		};
		std::unordered_map<std::uint64_t, BSAnimationGraphEventWatcher::FnProcessEvent> BSAnimationGraphEventWatcher::fnHash;
	}
}