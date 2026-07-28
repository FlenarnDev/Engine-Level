#pragma once

#include "Systems/Dialogue.h"
#include "Shared/SharedDeclarations.h"
#include "Shared/SharedFunctions.h"
#include "Systems/AmmoSwitch.h"
#include "Systems/Skills.h"
#include "Systems/LockLevels.h"

#define DETECTION_TYPE DETECTION_TYPE_WINDOWS
#include "detourXS/detourxs.h"
#undef DETECTION_TYPE

#undef min
#undef max
#undef GetForm

namespace Cascadia
{
	namespace Hooks
	{
		
		void Install();
		/*
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
		void RegisterAIFormulasComputePickpocketSuccess();

		
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
			RegisterAIFormulasComputePickpocketSuccess();
		}
		*/
		void RegisterAllHooks();

		template<typename T>
		static void RegisterDetourFunction(DetourXS& detour, const REL::ID functionID, const LPVOID& detourFunctionAddress, REL::Relocation<T>& OriginalFunction, std::string_view functionName) {
			REL::Relocation<T> functionLocation{ functionID };
			if (detour.Create(reinterpret_cast<void*>(functionLocation.address()), detourFunctionAddress)) {
				REX::INFO("Installed '{}' hook", functionName);
				OriginalFunction = reinterpret_cast<uintptr_t>(detour.GetTrampoline());
			}
			else {
				REX::CRITICAL(std::format("Failed to hook '{}'. Exiting...", functionName).c_str());
			}
		}

		/*
		template<typename T>
		static void RegisterDetourFunction(DetourXS& detour, const REL::ID functionID, const LPVOID& detourFunctionAddress, std::string_view functionName) {
			REL::Relocation<T> functionLocation{ functionID };
			if (detour.Create(reinterpret_cast<void*>(functionLocation.address()), detourFunctionAddress)) {
				REX::INFO("Installed '{}' hook", functionName);
				//OriginalFunction = reinterpret_cast<uintptr_t>(detour.GetTrampoline());
			}
			else {
				REX::CRITICAL(std::format("Failed to hook '{}'. Exiting...", functionName).c_str());
			}
		}
		*/
	}
}
