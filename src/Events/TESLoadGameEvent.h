#pragma once

#include "Menus/LevelUpMenu.h"

namespace Cascadia
{
	class TESLoadGameEventWatcher :
		public BSTEventSink<TESLoadGameEvent>
	{
		virtual BSEventNotifyControl ProcessEvent(const TESLoadGameEvent& a_event, BSTEventSource<TESLoadGameEvent>*) override
		{
			REX::DEBUG("'TESLoadGameEvent' fired!");
			LevelUpMenu::CheckForLevelUp();

			return BSEventNotifyControl::kContinue;
		}
	};

	void RegisterTESLoadGameEventSink()
	{
		TESLoadGameEventWatcher* tesLoadGameEvent = new TESLoadGameEventWatcher();
		TESLoadGameEvent::GetEventSource()->RegisterSink(tesLoadGameEvent);
		REX::DEBUG("Registered 'TESLoadGameEvent' sink.");
	}
}