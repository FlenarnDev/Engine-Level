#pragma once

#include "Systems/Dialogue.h"
#include "Shared/SharedDeclarations.h"
#include "Shared/SharedFunctions.h"
#include "Shared/InventoryHelpers.h"
#include "Systems/AmmoSwitch.h"
#include "Systems/Skills.h"
#include "Systems/LockLevels.h"

#define DETECTION_TYPE DETECTION_TYPE_WINDOWS
#include <SlimDetours.h>
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
		static bool RegisterDetourFunction(const REL::ID functionID, const LPVOID& detourFunctionAddress, REL::Relocation<T>& OriginalFunction, std::string_view functionName) {
			REL::Relocation<T> functionLocation{ functionID };
			OriginalFunction = functionLocation.address();
			HRESULT hr = SlimDetoursAttach(reinterpret_cast<PVOID*>(&OriginalFunction), detourFunctionAddress);
			if (SUCCEEDED(hr)) {
				REX::INFO("Installed '{}' hook", functionName);
				return true;
			}
			REX::CRITICAL("Failed to hook '{}' (hr={:#010x})", functionName, static_cast<unsigned long>(hr));
			return false;
		}
	}
}
