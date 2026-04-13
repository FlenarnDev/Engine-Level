#pragma once

namespace Cascadia
{
	class CellFullyLoadedListener : 
		public BSTEventSink<TESCellFullyLoadedEvent>
	{
	public:
		virtual BSEventNotifyControl ProcessEvent(const TESCellFullyLoadedEvent& a_event, BSTEventSource<TESCellFullyLoadedEvent>*) override
		{
			if (a_event.cell)
			{
				a_event.cell->UpdateAllDecals();
			}

			return BSEventNotifyControl::kContinue;
		}
	};

	void RegisterForCellFullyLoaded() {
		CellFullyLoadedListener* fullyLoadedCellEvent = new CellFullyLoadedListener();
		TESCellFullyLoadedEvent::GetEventSource()->RegisterSink(fullyLoadedCellEvent);
		REX::DEBUG("Registered 'TESCellFullLoadedEvent' sink.");
	}
}