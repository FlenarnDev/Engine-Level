#pragma once

#include "Shared/SharedFunctions.h"

namespace Cascadia
{
	void DefineItemDegradationFormsFromGame();

	extern std::map<TESAmmo*, float> ammoDegradationMap;
}