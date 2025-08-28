#pragma once

#include "Patches/LoadEditorIDs.h"
#include "Patches/Sprint.h"
#include "Patches/TESObjectREFR.h"

namespace RE
{
	namespace Cascadia
	{
		namespace Patches
		{
			void Install()
			{
				LoadEditorIDs::Install();
				TESObjectREFR_ConsoleName::Install();
				SprintPatch::Install();
			}
		}
	}
}
