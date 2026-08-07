#pragma once

#include "Systems/Dialogue.h"
#include "Shared/SharedDeclarations.h"
#include "Shared/SharedFunctions.h"
#include "Shared/InventoryHelpers.h"
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
		
		void RegisterAllHooks();

		template<typename T>
		static void RegisterDetourFunction(DetourXS& detour, const REL::ID functionID, const LPVOID& detourFunctionAddress, REL::Relocation<T>& OriginalFunction, std::string_view functionName) {
			REL::Relocation<T> functionLocation{ functionID };
			if (detour.Create(reinterpret_cast<void*>(functionLocation.address()), detourFunctionAddress)) {
				REX::INFO("Installed '{}' hook", functionName);
				OriginalFunction = reinterpret_cast<uintptr_t>(detour.GetTrampoline());
			}
			else {
				REX::CRITICAL("Failed to hook '{}'. Exiting...", functionName);
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
