#include "InventoryHelpers.h"

namespace InventoryUtils
{
	using namespace RE;

	std::uint32_t GetPipboyInventoryObjectCount()
	{
		return PipboyInventoryObjects.size();

		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		// return player->inventoryList->data.size();
	}

	std::string GetInventoryDisplayName(std::uint32_t index)
	{
		PipboyPrimitiveValue<BSFixedString>* pipboyObject = static_cast<PipboyPrimitiveValue<BSFixedString>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("text"))->second);

		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		// auto a = player->inventoryList->data.at(index).GetDisplayFullName()
		// return player->inventoryList->data.at(index).GetDisplayFullName;

		return pipboyObject->value.c_str();
	}

	std::uint32_t GetHandleIDByIndex(std::uint32_t index)
	{
		PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);

		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		// auto a = player->inventoryList->data.at(index).GetDisplayFullName()

		return pipboyObject->m_value;
	}

	std::uint32_t GetStackIDByIndex(std::uint32_t index)
	{

		PipboyArray* StackIDs = static_cast<PipboyArray*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("StackID"))->second);
		std::uint32_t Result = 0;

		for (int i = 0; i < StackIDs->elements.size(); i++)
		{
			PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(StackIDs->elements.at(i));
			Result = pipboyObject->m_value;
		}

		return Result;
	}


	TESForm* GetInventoryFormByHandleID(std::uint32_t HandleID)
	{
		return BGSInventoryInterface::GetSingleton()->RequestInventoryItem(HandleID)->object;
	}


	TESForm* GetInventoryFormByIndex(std::uint32_t index)
	{
		PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);
		std::uint32_t HandleID = pipboyObject->m_value;

		// RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();

		// return player->inventoryList->data.at(index).object;

		return GetInventoryFormByHandleID(HandleID);
	}


	const BGSInventoryItem* GetInventoryItemByHandleID(std::uint32_t HandleID)
	{
		return RE::BGSInventoryInterface::GetSingleton()->RequestInventoryItem(HandleID);
	}


	const BGSInventoryItem* GetInventoryItemByIndex(std::uint32_t index)
	{
		// auto player = RE::PlayerCharacter::GetSingleton();
		if (PipboyInventoryObjects.size() <= index)
			return nullptr;

		PipboyPrimitiveValue<std::uint32_t>* pipboyObject = static_cast<PipboyPrimitiveValue<std::uint32_t>*>(PipboyInventoryObjects.at(index)->memberMap.find(BSFixedString("HandleID"))->second);
		std::uint32_t HandleID = pipboyObject->m_value;
		return GetInventoryItemByHandleID(HandleID);
		// return &player->inventoryList->data.at(index);
	}

	const RE::BGSInventoryItem* GetInventoryItemWorkbenchByIndex()
	{
		auto ui = RE::UI::GetSingleton();
		if (!ui->GetMenuOpen<RE::ExamineMenu>() || ui->GetMenuOpen<RE::PowerArmorModMenu>()) {
			return nullptr;
		}

		auto examineMenu = ui->GetMenu<RE::ExamineMenu>();
		std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();

		if (selectedIndex >= examineMenu->invInterface.stackedEntries.size())
			selectedIndex = 0;

		if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size()) {
			InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
			return BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);
		}

		return nullptr;
	}

	const RE::BGSInventoryItem* GetCurrentEquippedWeapon(RE::Actor* a_actor)
	{
		if (!a_actor || !a_actor->inventoryList)
			return nullptr;

		a_actor->inventoryList->rwLock.lock_read();
		for (RE::BGSInventoryItem& inventoryItem : a_actor->inventoryList->data) {
			if (!inventoryItem.IsEquipped(0))
				continue;

			RE::TESObjectWEAP* weapon = static_cast<RE::TESObjectWEAP*>(inventoryItem.object);
			if (inventoryItem.object && weapon->formType == RE::ENUM_FORM_ID::kWEAP) {
				a_actor->inventoryList->rwLock.unlock_read();
				return &inventoryItem;
			}
		}
		a_actor->inventoryList->rwLock.unlock_read();


		return nullptr;
	}

	std::uint32_t GetIndexByInventoryItem(const BGSInventoryItem* item)
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
	}

	const BGSInventoryItem::Stack* GetStackByStackID(const BGSInventoryItem* Item, int StackID)
	{
		BGSInventoryItem::Stack* traverse = Item->stackData.get();
		if (!traverse)
			return nullptr;

		while (StackID != 0) {
			traverse = traverse->nextStack.get();
			if (!traverse)
				return nullptr;
			StackID--;
		}

		return traverse;
	}

	const RE::BGSInventoryItem* GetInventoryItemByForm(RE::TESForm* a_form)
	{
		if (!a_form)
			return nullptr;

		auto a_actor = RE::PlayerCharacter::GetSingleton();
		a_actor->inventoryList->rwLock.lock_read();
		for (RE::BGSInventoryItem& inventoryItem : a_actor->inventoryList->data) {
			if ((std::uint32_t)inventoryItem.object == (std::uint32_t)a_form) {
				a_actor->inventoryList->rwLock.unlock_read();
				return &inventoryItem;
			}
		}
		a_actor->inventoryList->rwLock.unlock_read();
		return nullptr;
	}

	ExtraDataList* GetExtraDataListByStackID(const BGSInventoryItem* Item, int StackID)
	{
		if (!Item)
			return nullptr;

		const BGSInventoryItem::Stack* stack = GetStackByStackID(Item, StackID);
		return stack->extra.get() ? stack->extra.get() : nullptr;
	}

	ExtraDataList* GetExtraDataListByIndex(std::uint32_t index)
	{
		const BGSInventoryItem* Item = GetInventoryItemByIndex(index);
		std::uint32_t              StackID = GetStackIDByIndex(index);

		if (!Item)
			return nullptr;

		const BGSInventoryItem::Stack* stack = GetStackByStackID(Item, StackID);
		return stack->extra.get() ? stack->extra.get() : nullptr;
	}
}