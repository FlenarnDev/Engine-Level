#pragma once

#include "Shared/SharedFunctions.h"

namespace Cascadia
{
	void DefineItemDegradationFormsFromGame();

	std::map<TESAmmo*, float> ammoDegradationMap;
}