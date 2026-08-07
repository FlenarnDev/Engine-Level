#pragma once

#define PipboyInventoryObjects RE::PipboyDataManager::GetSingleton()->inventoryData.sortedItems

#define GetTableItem(Index, Name)\
    (PipboyInventoryObjects[Index]->table.Find(&BSFixedString(Name)))

namespace InventoryUtils
{
    std::uint32_t GetPipboyInventoryObjectCount();
    std::string GetInventoryDisplayName(std::uint32_t index);
    std::uint32_t GetHandleIDByIndex(std::uint32_t index);
    std::uint32_t GetStackIDByIndex(std::uint32_t index);
    RE::TESForm* GetInventoryFormByHandleID(std::uint32_t HandleID);
    RE::TESForm* GetInventoryFormByIndex(std::uint32_t index);
    std::uint32_t GetIndexByInventoryItem(const RE::BGSInventoryItem* item);
    const RE::BGSInventoryItem* GetInventoryItemByHandleID(std::uint32_t HandleID);
    const RE::BGSInventoryItem* GetInventoryItemByIndex(std::uint32_t index);
    const RE::BGSInventoryItem* GetInventoryItemWorkbenchByIndex();
    const RE::BGSInventoryItem* GetCurrentEquippedWeapon(RE::Actor* a_actor);
    const RE::BGSInventoryItem::Stack* GetStackByStackID(const RE::BGSInventoryItem* Item, int StackID);
    const RE::BGSInventoryItem* GetInventoryItemByForm(RE::TESForm* a_form);
    RE::ExtraDataList* GetExtraDataListByStackID(const RE::BGSInventoryItem* Item, int StackID);
    RE::ExtraDataList* GetExtraDataListByIndex(std::uint32_t index);

    RE::TESObjectWEAP::InstanceData* GetWeaponInstanceData(RE::ExtraDataList* myExtraDataList);

	RE::TESObjectARMO::InstanceData* GetArmorInstanceData(RE::ExtraDataList* myExtraDataList);
}