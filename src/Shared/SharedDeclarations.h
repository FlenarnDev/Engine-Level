#pragma once

namespace RE
{
	namespace Cascadia
	{
		namespace Shared
		{
			extern BGSKeyword* noDegradation;
			extern TESLoadScreen* forcedLoadScreen;
			void InitializeSharedVariables();
			void InitializeSharedGameVariables();
			extern bool noArmorDegradation;
			extern bool noWeaponDegradation;
		}
	}
}