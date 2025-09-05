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

namespace ObScript
{
	void Install()
	{
		ObScript::BetaComment::Install();
		ObScript::GetBaseObject::Install();
		ObScript::GetDefaultObject::Install();
		//ObScript::Help::Install();
		ObScript::SavePlayerFace::Install();
		ObScript::TestLoadingMenu::Install();
		ObScript::ToggleArmorDegradation::Install();
		ObScript::ToggleMarkers::Install();
		ObScript::ToggleWeaponDegradation::Install();
	}
}
