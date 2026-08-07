#pragma once

namespace Cascadia
{
	class CameraFarDistanceListener :
		public BSTEventSink<TESCellFullyLoadedEvent>
	{
	public:
		virtual BSEventNotifyControl ProcessEvent(const TESCellFullyLoadedEvent&, BSTEventSource<TESCellFullyLoadedEvent>*) override
		{
			if (applied)
			{
				return BSEventNotifyControl::kContinue;
			}

			SceneGraph* sceneGraph = RE::Main::GetWorldRootNode();
			if (!sceneGraph)
			{
				return BSEventNotifyControl::kContinue;  // world not up yet, try again next cell load
			}

			sceneGraph->customFarDistance = 1000000.0f; // cba to fuck with it, this worked for our worldspace
			sceneGraph->customFarDistanceActive = true;
			applied = true;

			return BSEventNotifyControl::kContinue;
		}

	private:
		bool applied = false;
	};

	void RegisterForCameraFarDistanceOverride()
	{
		CameraFarDistanceListener* farDistanceListener = new CameraFarDistanceListener();
		TESCellFullyLoadedEvent::GetEventSource()->RegisterSink(farDistanceListener);
		REX::DEBUG("Registered 'CameraFarDistanceOverride' sink.");
	}
}
