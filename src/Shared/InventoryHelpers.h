#pragma once

#define PipboyInventoryObjects RE::PipboyDataManager::GetSingleton()->inventoryData.sortedItems

#define GetTableItem(Index, Name)\
    (PipboyInventoryObjects[Index]->table.Find(&BSFixedString(Name)))

using namespace RE;

namespace Cascadia
{
	namespace InventoryUtils
	{
		std::uint32_t GetPipboyInventoryObjectCount();
		//std::string GetInventoryDisplayName(std::uint32_t index);
		//std::uint32_t GetHandleIDByIndex(std::uint32_t index);
		//std::uint32_t GetStackIDByIndex(std::uint32_t index);
		TESForm* GetInventoryFormByHandleID(std::uint32_t a_handleID);
		//RE::TESForm* GetInventoryFormByIndex(std::uint32_t index);
		//std::uint32_t GetIndexByInventoryItem(const RE::BGSInventoryItem* item);
		const RE::BGSInventoryItem* GetInventoryItemByHandleID(std::uint32_t a_handleID);
		//const RE::BGSInventoryItem* GetInventoryItemByIndex(std::uint32_t index);
		const RE::BGSInventoryItem* GetInventoryItemWorkbenchByIndex();
		const RE::BGSInventoryItem* GetCurrentEquippedWeapon(Actor* a_actor);
		//const RE::BGSInventoryItem::Stack* GetStackByStackID(const RE::BGSInventoryItem* Item, int StackID);
		const RE::BGSInventoryItem* GetInventoryItemByForm(TESForm* a_form);
		//RE::ExtraDataList* GetExtraDataListByStackID(const RE::BGSInventoryItem* Item, int StackID);
		//RE::ExtraDataList* GetExtraDataListByIndex(std::uint32_t index);

		TESObjectWEAP::InstanceData* GetWeaponInstanceData(ExtraDataList* a_extraDataList);

		TESObjectARMO::InstanceData* GetArmorInstanceData(ExtraDataList* a_extraDataList);
	}
}