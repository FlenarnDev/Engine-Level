#include "InventoryHelpers.h"

namespace Cascadia
{
	namespace InventoryUtils
	{
		std::uint32_t GetPipboyInventoryObjectCount()
		{
			return PipboyInventoryObjects.size();
		}

		/**std::string GetInventoryDisplayName(std::uint32_t index)
		{
			PipboyPrimitiveValue<BSFixedString>* pipboyObject = static_cast<PipboyPrimitiveValue<BSFixedString>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("text"))->second);

			// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
			// auto a = player->inventoryList->data.at(index).GetDisplayFullName()
			// return player->inventoryList->data.at(index).GetDisplayFullName;

			return pipboyObject->value.c_str();
		}*/

		/**std::uint32_t GetHandleIDByIndex(std::uint32_t index)
		{
			PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);

			// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
			// auto a = player->inventoryList->data.at(index).GetDisplayFullName()

			return pipboyObject->value;
		}*/

		/**std::uint32_t GetStackIDByIndex(std::uint32_t index)
		{

			PipboyArray* StackIDs = static_cast<PipboyArray*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("StackID"))->second);
			std::uint32_t Result = 0;

			for (int i = 0; i < StackIDs->elements.size(); i++)
			{
				PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(StackIDs->elements.at(i));
				Result = pipboyObject->value;
			}

			return Result;
		}*/


		TESForm* GetInventoryFormByHandleID(std::uint32_t a_handleID)
		{
			return BGSInventoryInterface::GetSingleton()->RequestInventoryItem(a_handleID)->object;
		}


		/**TESForm* GetInventoryFormByIndex(std::uint32_t index)
		{
			PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);
			std::uint32_t HandleID = pipboyObject->value;

			// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

			// return player->inventoryList->data.at(index).object;

			return GetInventoryFormByHandleID(HandleID);
		}*/


		const BGSInventoryItem* GetInventoryItemByHandleID(std::uint32_t a_handleID)
		{
			return RE::BGSInventoryInterface::GetSingleton()->RequestInventoryItem(a_handleID);
		}


		/**const BGSInventoryItem* GetInventoryItemByIndex(std::uint32_t index)
		{
			// auto player = RE::PlayerCharacter::GetSingleton();
			if (PipboyInventoryObjects.size() <= index)
				return nullptr;

			PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);
			std::uint32_t HandleID = pipboyObject->value;
			return GetInventoryItemByHandleID(HandleID);
			// return &player->inventoryList->data.at(index);
		}*/

		const BGSInventoryItem* GetInventoryItemWorkbenchByIndex()
		{
			auto ui = UI::GetSingleton();
			if (!ui->GetMenuOpen<ExamineMenu>() || ui->GetMenuOpen<PowerArmorModMenu>()) {
				return nullptr;
			}

			auto examineMenu = ui->GetMenu<ExamineMenu>();
			std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();

			if (selectedIndex >= examineMenu->invInterface.stackedEntries.size()) {
				selectedIndex = 0;
			}

			if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size()) {
				InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
				return BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);
			}

			return nullptr;
		}

		const RE::BGSInventoryItem* GetCurrentEquippedWeapon(Actor* a_actor)
		{
			if (!a_actor || !a_actor->inventoryList)
				return nullptr;

			a_actor->inventoryList->rwLock.lock_read();
			for (BGSInventoryItem& inventoryItem : a_actor->inventoryList->data) {
				if (!inventoryItem.IsEquipped(0))
					continue;

				TESObjectWEAP* weapon = static_cast<TESObjectWEAP*>(inventoryItem.object);
				if (inventoryItem.object && weapon->formType == ENUM_FORM_ID::kWEAP) {
					a_actor->inventoryList->rwLock.unlock_read();
					return &inventoryItem;
				}
			}
			a_actor->inventoryList->rwLock.unlock_read();


			return nullptr;
		}

		/**std::uint32_t GetIndexByInventoryItem(const BGSInventoryItem* item)
		{
			std::uint32_t inventoryCount = GetPipboyInventoryObjectCount();

			for (std::uint32_t i = 0; i < inventoryCount; i++)
			{
				const BGSInventoryItem* iter = GetInventoryItemByIndex(i);

				if (item == iter)
				{
					return i;
				}
			}

			return -1;
		}*/

		const BGSInventoryItem::Stack* GetStackByStackID(const BGSInventoryItem* a_item, std::uint32_t a_stackID)
		{
			BGSInventoryItem::Stack* traverse = a_item->stackData.get();
			if (!traverse) {
				return nullptr;
			}
				
			while (a_stackID != 0) {
				traverse = traverse->nextStack.get();
				if (!traverse) {
					return nullptr;
				}
					
				a_stackID--;
			}

			return traverse;
		}

		const BGSInventoryItem* GetInventoryItemByForm(TESForm* a_form)
		{
			if (!a_form) {
				return nullptr;
			}

			PlayerCharacter* a_actor = PlayerCharacter::GetSingleton();
			a_actor->inventoryList->rwLock.lock_read();
			for (BGSInventoryItem& inventoryItem : a_actor->inventoryList->data) {
				if ((std::uint32_t)inventoryItem.object == (std::uint32_t)a_form) {
					a_actor->inventoryList->rwLock.unlock_read();
					return &inventoryItem;
				}
			}
			a_actor->inventoryList->rwLock.unlock_read();
			return nullptr;
		}

		ExtraDataList* GetExtraDataListByStackID(const BGSInventoryItem* a_item, std::uint32_t a_stackID)
		{
			if (!a_item) {
				return nullptr;
			}

			const BGSInventoryItem::Stack* stack = GetStackByStackID(a_item, a_stackID);
			return stack->extra.get() ? stack->extra.get() : nullptr;
		}

		/**ExtraDataList* GetExtraDataListByIndex(std::uint32_t index)
		{
			const BGSInventoryItem* Item = GetInventoryItemByIndex(index);
			std::uint32_t              StackID = GetStackIDByIndex(index);

			if (!Item)
				return nullptr;

			const BGSInventoryItem::Stack* stack = GetStackByStackID(Item, StackID);
			return stack->extra.get() ? stack->extra.get() : nullptr;
		}*/

		TESObjectWEAP::InstanceData* GetWeaponInstanceData(ExtraDataList* a_extraDataList)
		{
			TESObjectWEAP::InstanceData* result = nullptr;
			TBO_InstanceData* myInstanceData = nullptr;

			a_extraDataList->extraRWLock.lock_read();

			if (a_extraDataList)
			{
				BSExtraData* myExtraData = a_extraDataList->HasType<ExtraInstanceData>() ? a_extraDataList->GetByType<ExtraInstanceData>() : nullptr;

				if (myExtraData)
				{
					ExtraInstanceData* myExtraInstanceData = static_cast<ExtraInstanceData*>(myExtraData);
					if (myExtraInstanceData)
					{
						myInstanceData = myExtraInstanceData->data.get();
					}
				}
			}

			a_extraDataList->extraRWLock.unlock_read();
			if (!myInstanceData) {
				return nullptr;
			}

			result = static_cast<TESObjectWEAP::InstanceData*>(myInstanceData);
			return result;
		}

		TESObjectARMO::InstanceData* GetArmorInstanceData(ExtraDataList* a_extraDataList)
		{
			TESObjectARMO::InstanceData* result = nullptr;
			TBO_InstanceData* myInstanceData = nullptr;
			a_extraDataList->extraRWLock.lock_read();
			if (a_extraDataList)
			{
				BSExtraData* myExtraData = a_extraDataList->HasType<ExtraInstanceData>() ? a_extraDataList->GetByType<ExtraInstanceData>() : nullptr;

				if (myExtraData)
				{
					ExtraInstanceData* myExtraInstanceData = static_cast<ExtraInstanceData*>(myExtraData);
					if (myExtraInstanceData)
					{
						myInstanceData = myExtraInstanceData->data.get();
					}
				}
			}
			a_extraDataList->extraRWLock.unlock_read();
			if (!myInstanceData) {
				return nullptr;
			}

			result = static_cast<TESObjectARMO::InstanceData*>(myInstanceData);
			return result;
		}
	}
}