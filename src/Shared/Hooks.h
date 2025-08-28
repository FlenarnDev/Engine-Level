#pragma once

#include "Systems/Dialogue.h"
#include "Shared/SharedDeclarations.h"
#include "Shared/SharedFunctions.h"
#include "Systems/AmmoSwitch.h"
#include "Systems/Skills.h"

namespace RE
{
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
			}
		}
	}
}
