#pragma once

namespace RE
{
	namespace Cascadia
	{
		namespace Shared
		{
			extern BGSKeyword* noDegradation;
			void InitializeSharedVariables();
			void InitializeSharedGameVariables();
			extern bool noArmorDegradation;
			extern bool noWeaponDegradation;
		}
	}
}