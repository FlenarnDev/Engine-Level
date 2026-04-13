#pragma once

#include "Menus/LevelUpMenu.h"

namespace Cascadia
{
	class LevelIncreaseEventWatcher :
		public BSTEventSink<LevelIncrease::Event>
	{
		virtual BSEventNotifyControl ProcessEvent(const LevelIncrease::Event& a_event, BSTEventSource<LevelIncrease::Event>*) override
		{
			LevelUpMenu::HandleLevelUp();
			return BSEventNotifyControl::kContinue;
		}
	};

	void RegisterLevelIncreaseEventSink()
	{
		LevelIncreaseEventWatcher* levelIncreaseEvent = new LevelIncreaseEventWatcher();
		LevelIncrease::GetEventSource()->RegisterSink(levelIncreaseEvent);
		REX::DEBUG("Registered 'LevelIncrease::Event' sink.");
	}
}