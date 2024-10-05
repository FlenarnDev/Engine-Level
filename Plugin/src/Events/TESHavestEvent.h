#pragma once

namespace RE
{
	namespace Cascadia
	{
		class TESHarvestEventWatcher :
			public BSTEventSink<TESHarvestEvent::ItemHarvested>
		{
			virtual BSEventNotifyControl ProcessEvent(const TESHarvestEvent::ItemHarvested& a_event, BSTEventSource<TESHarvestEvent::ItemHarvested>*) override
			{
				if (a_event.harvestedBy == PlayerCharacter::GetSingleton())
				{
					if (a_event.itemHarvested->formType == ENUM_FORM_ID::kMISC || a_event.itemHarvested->formType == ENUM_FORM_ID::kLVLI)
					{
						DEBUG("Harvest event sent by player, harvested REFR: {}, harvest object: {}.", a_event.referenceHarvested->GetFormEditorID(), a_event.itemHarvested->GetFormEditorID());
						const char* itemHarvestedEDID = a_event.itemHarvested->GetFormEditorID();
						GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
						const char* addedMessage = gameSettingCollection->GetSetting("sAddItemtoInventory")->GetString().data();
						if (a_event.itemHarvested->formType == ENUM_FORM_ID::kMISC)
						{
							TESObjectMISC* miscItemHarvested = (TESObjectMISC*)(a_event.itemHarvested);
							const char* itemHarvestedName = miscItemHarvested->GetFullName();
							std::string message = std::string(itemHarvestedName) + " " + addedMessage;
							SendHUDMessage::ShowHUDMessage(message.c_str(), nullptr, true, true);
						}
						else
						{
							if (strcmp(itemHarvestedEDID, "CAS_LL_WoodPile_Random") == 0)
							{
								// TODO - figure out a way to aquire FullName of the result from the LVLI roll.
								std::string message = "Firewood";
								message += " ";
								message += addedMessage;
								SendHUDMessage::ShowHUDMessage(message.c_str(), nullptr, true, true);
							}
						}
					}
				}

				return BSEventNotifyControl::kContinue;
			}
		};

		void RegisterTESHarvestEventSink()
		{
			TESHarvestEventWatcher* tesHarvestEvent = new TESHarvestEventWatcher();
			TESHarvestEvent::ItemHarvested::GetEventSource()->RegisterSink(tesHarvestEvent);
			INFO("Registered 'TESHarvestEvent::ItemHarvested' sink.");
		}
	}
}