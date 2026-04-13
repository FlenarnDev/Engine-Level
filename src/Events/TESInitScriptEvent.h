#pragma once

namespace Cascadia
{
	class TESInitScriptEventWatcher :
		public BSTEventSink<TESInitScriptEvent>
	{
		virtual BSEventNotifyControl ProcessEvent(const TESInitScriptEvent& a_event, BSTEventSource<TESInitScriptEvent>*) override
		{
			return BSEventNotifyControl::kContinue;
		}
	};

	void RegisterTESInitScriptEventSink()
	{
		TESInitScriptEventWatcher* tesInitScriptEvent = new TESInitScriptEventWatcher();
		TESInitScriptEvent::GetEventSource()->RegisterSink(tesInitScriptEvent);
		REX::DEBUG("Registered 'TESInitScriptEvent' sink.");
	}
}