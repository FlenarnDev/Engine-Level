#include "Shared/SharedDeclarations.h"

namespace RE
{
	namespace Cascadia
	{
		namespace Shared
		{
			BGSKeyword* noDegradation;
			TESLoadScreen* forcedLoadScreen;
			bool noArmorDegradation;
			bool noWeaponDegradation;

			void InitializeSharedGameVariables()
			{
				TESDataHandler* dataHandler = TESDataHandler::GetSingleton();
				noDegradation = dataHandler->LookupForm<BGSKeyword>(0x2BD72E, "FalloutCascadia.esm");
			}

			void InitializeSharedVariables()
			{
				noArmorDegradation = false;
				noWeaponDegradation = false;
			}
		}
	}
}