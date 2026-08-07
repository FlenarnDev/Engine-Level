#pragma once

#include "Patches/LoadEditorIDs.h"
#include "Patches/Sprint.h"
#include "Patches/TESObjectREFR.h"
#include "Patches/MoonRotation.h"
#include "Patches/CameraFrustum.h"

namespace Cascadia
{
	namespace Patches
	{
		void Install()
		{
			CameraFrustum::Install();
			LoadEditorIDs::Install();
			TESObjectREFR_ConsoleName::Install();
			SprintPatch::Install();
			MoonRotation::Install();
		}
	}
}