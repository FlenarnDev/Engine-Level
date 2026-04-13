#pragma once

#include "Systems/Dialogue.h"
#include "Shared/SharedDeclarations.h"
#include "Shared/SharedFunctions.h"
#include "Systems/AmmoSwitch.h"
#include "Systems/Skills.h"

namespace Cascadia
{
	namespace Hooks
	{
		void Install();
		void RegisterCalcTargetedLimbDamage();
		void RegisterAddItemHook();
		void RegisterSetHealthPercHook();
		void RegisterGetInventoryValueHook();
		void RegisterShowBuildFailureMessage();
		void RegisterGetBuildConfirmQuestion();
		void RegisterQCurrentModChoiceData();
		void RegisterExamineMenuBuildConfirmed();
		void RegisterTESObjectWEAPFire();
		void RegisterCombatFormulasCalcWeaponDamage();
		void RegisterGetEquippedArmorDamageResistance();
		void RegisterIUUIIUtilsAddItemCardInfoEntry();
		void RegisterPipboyInventoryDataBaseAddItemsCardInfoEntry();
		void RegisterIUUIIUtilsPopulateItemCardInfo_Helper();
		void RegisterPipboyInventoryUtilsFillResistTypeInfo();
		void RegisterActorUnequipObject();
		void RegisterLoadingMenuPopulateLoadScreens();
		void RegisterGamePlayFormulasCanPickLockGateCheck();
		void RegisterGamePlayFormulasCanHackGateCheck();
		void RegisternsHUDTypesNotificationData_ctor();
		void RegisterActorSPECIALModifiedCallback();
		void RegisterREFR_LOCKNumericValueToEnum();
		void RegisterREFR_LOCKIsInaccessible();
		void RegisterGamePlayFormulasGetLockXPReward();
		void RegisterActorProcessGetActorLightLevel();
		void RegisterActorCalculateDetectionFormula();

		void RegisterHooks()
		{
			RegisterActorUnequipObject();
			RegisterCalcTargetedLimbDamage();
			RegisterAddItemHook();
			RegisterSetHealthPercHook();
			RegisterGetInventoryValueHook();
			RegisterShowBuildFailureMessage();
			RegisterGetBuildConfirmQuestion();
			RegisterQCurrentModChoiceData();
			RegisterExamineMenuBuildConfirmed();
			RegisterTESObjectWEAPFire();
			RegisterCombatFormulasCalcWeaponDamage();
			RegisterGetEquippedArmorDamageResistance();
			RegisterIUUIIUtilsAddItemCardInfoEntry();
			RegisterPipboyInventoryDataBaseAddItemsCardInfoEntry();
			RegisterIUUIIUtilsPopulateItemCardInfo_Helper();
			RegisterPipboyInventoryUtilsFillResistTypeInfo();
			RegisterLoadingMenuPopulateLoadScreens();
			RegisterGamePlayFormulasCanPickLockGateCheck();
			RegisterGamePlayFormulasCanHackGateCheck();
			RegisternsHUDTypesNotificationData_ctor();
			RegisterActorSPECIALModifiedCallback();
			RegisterREFR_LOCKNumericValueToEnum();
			RegisterREFR_LOCKIsInaccessible();
			RegisterGamePlayFormulasGetLockXPReward();
			RegisterActorProcessGetActorLightLevel();
			RegisterActorCalculateDetectionFormula();
		}
	}
}
