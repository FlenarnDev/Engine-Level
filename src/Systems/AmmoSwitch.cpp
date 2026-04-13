#include "AmmoSwitch.h"

namespace Cascadia
{
	namespace AmmoSwitch
	{
		bool switchingAmmo = false;
		TESAmmo* ammoToSwitchTo;
		BGSObjectInstance* equippedInstance;

		BSTArray<BGSKeyword*> keywordsAmmo;
		std::unordered_map<BGSKeyword*, BGSListForm*> keywordsAmmoCaliberMap;
		std::unordered_map<TESAmmo*, BGSKeyword*> keywordsAmmoOMODMap;

		BSTArray<BGSKeyword*> keywordsOMOD;
		
		BGSKeyword* noFormlistWEAP;
		BGSKeyword* uniqueFormlistWEAP;
		BGSKeyword* materialChange;
		BGSKeyword* omodAPKeyword;

		std::vector<std::pair<BGSKeyword*, BGSMod::Attachment::Mod*>> omodFormListVec;
		std::unordered_map<BGSKeyword*, BGSMod::Attachment::Mod*> omodKeywordMap;

		bool InitializeAmmoSwitch() {
				// Return true to make sure proper menu audio is played if ammo switch is successfully initiated, otherwise false return.
				equippedInstance = new BGSObjectInstance(nullptr, nullptr);
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
				playerCharacter->GetEquippedItem(equippedInstance, BGSEquipIndex{0});

				if (equippedInstance->object && equippedInstance->object->formType == ENUM_FORM_ID::kWEAP &&
				    static_cast<TESObjectWEAP*>(equippedInstance->object)->weaponData.type == WEAPON_TYPE::kGun) {
				    TESObjectWEAP* weapon = static_cast<TESObjectWEAP*>(equippedInstance->object);
				    TESObjectWEAP::InstanceData* instanceDataWEAP = (TESObjectWEAP::InstanceData*)(equippedInstance->instanceData.get());

				    const char* standardListPrefix = "CAS_AmmoSwitch_Standard_";
				    const char* uniqueListPrefix = "CAS_AmmoSwitch_Unique_";

				    // Junkjet and other very unique weapons in terms of ammunition.
				    if (weapon->HasKeyword(noFormlistWEAP)) {
				        REX::DEBUG("'AmmoSwitch::InitializeAmmoSwitch' - weapon '{}' is flagged for no ammo switching.", weapon->GetFormEditorID());
				        return false;
				    }

				    TESAmmo* currentAmmo = instanceDataWEAP->ammo;
				    if (currentAmmo == nullptr) {
				        // Just in case.
				        return false;
				    }

				    const bool isUnique = weapon->HasKeyword(uniqueFormlistWEAP);
				    const char* prefix = isUnique ? uniqueListPrefix : standardListPrefix;
				    auto getKeywordCount = [isUnique, weapon, currentAmmo]() -> std::uint32_t {
				        return isUnique ? weapon->GetNumKeywords() : currentAmmo->GetNumKeywords();
				    };
				    auto getKeywordAt = [isUnique, weapon, currentAmmo](std::uint32_t i) -> std::optional<BGSKeyword*> {
				        return isUnique ? weapon->GetKeywordAt(i) : currentAmmo->GetKeywordAt(i);
				    };

				    std::uint32_t keywordCount = getKeywordCount();
				    if (keywordCount > 0) {
				        bool foundKeyword = false;
				        for (std::uint32_t i = 0; i < keywordCount; ++i) {
				            std::optional<BGSKeyword*> bgsKeyword = getKeywordAt(i);
				            if (bgsKeyword.has_value()) {
				                const char* formEditorID = bgsKeyword.value()->GetFormEditorID();
				                if (strncmp(formEditorID, prefix, strlen(prefix)) == 0) {
				                    foundKeyword = true;
				                    if (FindAmmoInFormlist(bgsKeyword.value(), currentAmmo, playerCharacter)) {
										if (ammoToSwitchTo->HasKeyword(materialChange))
										{
											MaterialSwap(currentAmmo);
										}
				                        break;
				                    }
				                }
				            }
				        }
				        if (!foundKeyword) {
				            REX::DEBUG("'AmmoSwitch::InitializeAmmoSwitch' - {} flagged for ammo switch but no matching keyword found.", isUnique ? "weapon" : "ammo");
				            return false;
				        }
				    } else {
				        return false;
				    }

				    if (playerCharacter->NotifyAnimationGraphImpl("reloadStart")) { // Fails if no weapon is drawn, amongst other things.
				        switchingAmmo = true;
				    } else {
				        return false;
				    }
				    return true;
				}
				return false;
			}

		bool FindAmmoInFormlist(BGSKeyword* keyword, TESAmmo* currentAmmo, PlayerCharacter* playerCharacter) {
		    auto mapEntry = keywordsAmmoCaliberMap.find(keyword);
		    if (mapEntry != keywordsAmmoCaliberMap.end()) {
		        BGSListForm* formList = mapEntry->second;
		        std::uint32_t formListSize = formList->arrayOfForms.size();
		        if (formListSize != 0) {
		            std::uint32_t index = 0;
		            for (; index < formListSize; ++index) {
		                if (formList->arrayOfForms.begin()[index] == (TESForm*)currentAmmo) {
		                    REX::DEBUG("'AmmoSwitch::FindAmmoInFormlist' - current ammo position in found formlist: {}.", index);
		                    break;
		                }
		            }
		
		            for (std::uint32_t i = 1; i < formListSize; i++) {
		                std::uint32_t currentIndex = (index + i) % formListSize;
		                TESBoundObject* a_object = (TESBoundObject*)formList->arrayOfForms[currentIndex];
		                if (playerCharacter->GetInventoryObjectCount(a_object) != 0 && currentAmmo != (TESAmmo*)a_object) {
		                    ammoToSwitchTo = (TESAmmo*)a_object;
		                    return true;
		                }
		            }
		        }
		    } else {
		        REX::CRITICAL("'AmmoSwitch::FindAmmoInFormlist' - keyword not found in 'keywordFormlistMap'.");
		    }
		    return false;
		}

		bool MaterialSwap(TESAmmo* currentAmmo)
			{
				REX::DEBUG("'AmmoSwitch::MaterialSwap' init.");

				if (ammoToSwitchTo->swapForm)
				{
					//REX::DEBUG("'AmmoSwitch::MaterialSwap', ammunition: {} does not have a swapForm.", ammoToSwitchTo->GetFormEditorID());

					BGSMaterialSwap* materialSwap = ammoToSwitchTo->swapForm;
					BSTHashMap<BSFixedString, BGSMaterialSwap::Entry> swapMap = materialSwap->swapMap;
					if (swapMap.size() != 1)
					{
						REX::WARN("'AmmoSwitch::MaterialSwap' - ammo: {} has too many entries in swapMap: {}", ammoToSwitchTo->GetFormEditorID(), swapMap.size());
						return false;
					}
					PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

					TESFileContainer sourceFiles = materialSwap->sourceFiles;


					TESObjectWEAP::InstanceData* instanceDataWEAP = (TESObjectWEAP::InstanceData*)(equippedInstance->instanceData.get());
					 
					BSTArray<BGSMaterialSwap*>* test = instanceDataWEAP->materialSwaps;
					if (test)
					{
						REX::DEBUG("WEAPON HAS MATERIAL SWAP ARRAY!");
					}
					else
					{
						REX::DEBUG("non existant, so sad...");
						BSTArray<BGSMaterialSwap*>* test = new BSTArray<BGSMaterialSwap*>;
						instanceDataWEAP->materialSwaps = test;
						REX::DEBUG("tried establishing new array for it?");
						instanceDataWEAP->materialSwaps->push_back(materialSwap);
						
					}

					BGSInventoryItem* inventoryItem = nullptr;
					TESObjectWEAP* tesWEAP = (TESObjectWEAP*)equippedInstance->object;
					TESFormID weaponFormID = tesWEAP->GetFormID();
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


						ExtraDataList* extraDataList = inventoryItem->stackData->extra.get();
						BGSObjectInstanceExtra* objectModData = (BGSObjectInstanceExtra*)extraDataList->GetByType(EXTRA_DATA_TYPE::kObjectInstance);
						if (objectModData)
						{
							REX::DEBUG("'AmmoSwitch::MaterialSwap' - OMOD count on '{}' is: {}.", tesWEAP->GetFormEditorID(), objectModData->GetNumMods(false));
						}
						auto test = equippedInstance->instanceData.get();
					}
				}
				else
				{
					// Assumed to be the 'default' ammunition (at the very least refer to .nif for original material) of this type.
					// TODO!
				}
				return true;
			}

		void DefineAmmoLists()
		{
			TESDataHandler* dataHandler = TESDataHandler::GetSingleton();
			BSTArray<TESForm*> ammoEntries = dataHandler->formArrays[std::to_underlying(ENUM_FORM_ID::kAMMO)];
			BSTArray<TESForm*> keywordEntries = dataHandler->formArrays[std::to_underlying(ENUM_FORM_ID::kKYWD)];
			BSTArray<TESForm*> omodEntries = dataHandler->formArrays[std::to_underlying(ENUM_FORM_ID::kOMOD)];
			noFormlistWEAP = dataHandler->LookupForm<BGSKeyword>(0x2D9AB8, "FalloutCascadia.esm");
			uniqueFormlistWEAP = dataHandler->LookupForm<BGSKeyword>(0x2D9AB9, "FalloutCascadia.esm");
			omodAPKeyword = dataHandler->LookupForm<BGSKeyword>(0x2F32F2, "FalloutCascadia.esm");
			materialChange = dataHandler->LookupForm<BGSKeyword>(0x000001, "CAS_AmmoSwitch_Extension.esp");
			
			const char* standardListPrefix = "CAS_AmmoSwitch_Standard_";
			const char* uniqueListPrefix = "CAS_AmmoSwitch_Unique_";
			const char* omodPrefix = "CAS_AmmoSwitch_OMOD_";

			// Identify all keywords.
			for (TESForm* tesForm : keywordEntries)
			{
				const char* formEditorID = tesForm->GetFormEditorID();

				if (strncmp(formEditorID, standardListPrefix, strlen(standardListPrefix)) == 0 || strncmp(formEditorID, uniqueListPrefix, strlen(uniqueListPrefix)) == 0)
				{
					REX::DEBUG("'AmmoSwitch::DefineAmmoLists' - matching keyword: {}, added to 'keywordsAmmo'.", formEditorID);
					keywordsAmmo.push_back((BGSKeyword*)tesForm);
					keywordsAmmoCaliberMap[(BGSKeyword*)tesForm] = new BGSListForm;
				}

				if (strncmp(formEditorID, omodPrefix, strlen(omodPrefix)) == 0)
				{
					REX::DEBUG("'AmmoSwitch::DefineAmmoLists' - matching keyword: {}, added to 'keywordsOMOD'.", formEditorID);
					keywordsOMOD.push_back((BGSKeyword*)tesForm);
				}
			}

			// Prep OMOD lists.
			for (TESForm* tesForm : omodEntries)
			{
				BGSMod::Attachment::Mod* omod = static_cast<BGSMod::Attachment::Mod*>(tesForm);
				BGSKeyword* currentKeyword = BGSKeyword::GetTypedKeywordByIndex(KeywordType::kAttachPoint, omod->attachPoint.keywordIndex);
				if (currentKeyword == omodAPKeyword)
				{
					// Currently we assume the 'filterKeyword' fetched here is populated with a valid OMOD keyword. 
					BGSKeyword* filterKeyword = BGSKeyword::GetTypedKeywordByIndex(KeywordType::kInstantiationFilter, omod->filterKeywords.array->keywordIndex);
					if (filterKeyword)
					{
						REX::DEBUG("Filter keyword {} on OMOD: {}", filterKeyword->GetFormEditorID(), omod->GetFormEditorID());
						omodKeywordMap[filterKeyword] = omod;
					}
				}
			}

			// Ammo iteration time
			for (TESForm* tesForm : ammoEntries)
			{
				TESAmmo* tesAMMO = static_cast<TESAmmo*>(tesForm);

				std::uint32_t keywordCount = tesAMMO->GetNumKeywords();

				if (keywordCount > 0)
				{
					for (std::uint32_t i = 0; i < keywordCount; ++i)
					{
						std::optional<BGSKeyword*> bgsKeyword = tesAMMO->GetKeywordAt(i);
						if (bgsKeyword.has_value())
						{
							const char* formEditorID = bgsKeyword.value()->GetFormEditorID();

							// Caliber mapping
							if (strncmp(formEditorID, standardListPrefix, strlen(standardListPrefix)) == 0 || strncmp(formEditorID, uniqueListPrefix, strlen(uniqueListPrefix)) == 0)
							{
								REX::DEBUG("'AmmoSwitch::DefineAmmoLists' - found caliber keyword: {}, on ammo: {}.", bgsKeyword.value()->GetFormEditorID(), tesForm->GetFormEditorID());
								auto mapEntry = keywordsAmmoCaliberMap.find(bgsKeyword.value());
								if (mapEntry != keywordsAmmoCaliberMap.end())
								{
									BGSListForm* formList = mapEntry->second;
									formList->arrayOfForms.push_back(tesForm);
								}
								else
								{
									REX::CRITICAL("'AmmoSwitch::DefineAmmoLists' - keyword '{}' not found in 'keywordFormlistMap'.", formEditorID);
								}	
							} 

							// OMOD mapping [ammo,keyword]
							else if (strncmp(formEditorID, omodPrefix, strlen(omodPrefix)) == 0)
							{
								REX::DEBUG("'AmmoSwitch::DefineAmmoLists' - found OMOD keyword: {}, on ammo: {}.", bgsKeyword.value()->GetFormEditorID(), tesForm->GetFormEditorID());
								keywordsAmmoOMODMap[(TESAmmo*)tesForm] = (BGSKeyword*)bgsKeyword.value();
							}
						}
					}
				}
			}
			
			REX::DEBUG("'AmmoSwitch::DefineAmmoLists' - 'keywordFormlistMap' size: {}", keywordsAmmoCaliberMap.size());
		}

		void PostLoadGameAmmoFix()
		{
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			BGSObjectInstance* equippedInstance = new BGSObjectInstance(nullptr, nullptr);
			playerCharacter->GetEquippedItem(equippedInstance, BGSEquipIndex{ 0 });
			if (equippedInstance->object && equippedInstance->object->formType == ENUM_FORM_ID::kWEAP && static_cast<TESObjectWEAP*>(equippedInstance->object)->weaponData.type == WEAPON_TYPE::kGun)
			{
				TESAmmo* currentAmmoInPlayerMemory = playerCharacter->GetCurrentAmmo(BGSEquipIndex{ 0 });
				TESObjectWEAP::InstanceData* instanceDataWEAP = (TESObjectWEAP::InstanceData*)(equippedInstance->instanceData.get());
				TESAmmo* currentAmmoInWeaponInstance = instanceDataWEAP->ammo;

				if (currentAmmoInPlayerMemory != currentAmmoInWeaponInstance)
				{
					REX::DEBUG("'AmmoSwitch::PostLoadGameAmmoFix' - adjusted weapon instance ammo from: {}, to: {}", currentAmmoInWeaponInstance->GetFormEditorID(), currentAmmoInPlayerMemory->GetFormEditorID());
					instanceDataWEAP->ammo = currentAmmoInPlayerMemory;
					PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardOnSection(ENUM_FORM_ID::kWEAP);
				}
			}
		}
	}
}