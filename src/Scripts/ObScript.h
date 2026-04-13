#pragma once

#include "ObScript/BetaComment.h"
#include "ObScript/GetBaseObject.h"
#include "ObScript/GetDefaultObject.h"
#include "ObScript/Help.h"
#include "ObScript/SavePlayerFace.h"
#include "ObScript/TestLoadingMenu.h"
#include "ObScript/ToggleArmorDegradation.h"
#include "ObScript/ToggleMarkers.h"
#include "ObScript/ToggleWeaponDegradation.h"

// Repurposed retail commands:
// "IgnoreRenderPass" = "GetBaseObject"
// "DumpSoundMap" = "GetDefaultObject"
// "CaptureMessage" = "ToggleArmorDegradation"
// "CaptureMessages" = "ToggleWeaponDegradation"

namespace Cascadia
{
	namespace ObScript
	{
		void Install()
		{
			BetaComment::Install();
			GetBaseObject::Install();
			GetDefaultObject::Install();
			//ObScript::Help::Install();
			SavePlayerFace::Install();
			TestLoadingMenu::Install();
			ToggleArmorDegradation::Install();
			ToggleMarkers::Install();
			ToggleWeaponDegradation::Install();
		}
	}
}