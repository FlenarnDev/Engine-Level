#pragma once

#include "Menus/ExamineMenu.h"
#include "Menus/DialogueMenu.h"
#include "Menus/LevelUpMenu.h"
#include "Menus/PipboyTabs.h"

namespace Cascadia
{
	class MenuOpenCloseEventWatcher :
		public BSTEventSink<MenuOpenCloseEvent>
	{
		virtual BSEventNotifyControl ProcessEvent(const MenuOpenCloseEvent& a_event, BSTEventSource<MenuOpenCloseEvent>*) override
		{
			if (a_event.menuName == BSFixedString("DialogueMenu"))
			{
				if (a_event.opening)
				{
					DialogueMenu::savedSubtitlePosition = DialogueMenu::GetSubtitlePosition();
				}
				else
				{
					DialogueMenu::SetSubtitlePosition(DialogueMenu::savedSubtitlePosition.first, DialogueMenu::savedSubtitlePosition.second);
				}
			}
			else if (a_event.menuName == BSFixedString("CASLevelUpMenu"))
			{
				LevelUp::Controller::GetSingleton().OnMenuOpenClose(a_event.opening);
			}
			else if (a_event.menuName == BSFixedString("PipboyMenu"))
			{
				if (a_event.opening)
				{
					PipboyTabs::RefreshSkillsTabIfOpen();
				}
			}

			REX::DEBUG("Menu: {}, opening: {}", a_event.menuName, a_event.opening);

			return BSEventNotifyControl::kContinue;
		}
	};

	void RegisterMenuOpenCloseEventSink()
	{
		MenuOpenCloseEventWatcher* menuOpenCloseEvent = new MenuOpenCloseEventWatcher();
		UI::GetSingleton()->GetEventSource<MenuOpenCloseEvent>()->RegisterSink(menuOpenCloseEvent);
		REX::DEBUG("Registered 'MenuOpenCloseEvent' sink.");
	}
}
