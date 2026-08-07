#pragma once

#include "Shared/SharedDeclarations.h"
#include "Shared/SharedFunctions.h"

namespace Cascadia
{
	void DefineItemDegradationFormsFromGame();

	extern std::map<TESAmmo*, float> ammoDegradationMap;
}