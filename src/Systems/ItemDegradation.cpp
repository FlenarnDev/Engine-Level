#include "ItemDegradation.h"

namespace Cascadia
{
	std::map<TESAmmo*, float> ammoDegradationMap;

	void DefineItemDegradationFormsFromGame()
	{
		REX::INFO("Item Degradation: Linking degradation forms.");
		TESDataHandler* dataHandler = TESDataHandler::GetSingleton();

		// Ammo types with respective degradation values
		TESAmmo* ammo10mm = dataHandler->LookupForm<TESAmmo>(0x01F276, RETAIL_ESM01);
		TESAmmo* ammo2mmEC = dataHandler->LookupForm<TESAmmo>(0x18ABDF, RETAIL_ESM01);
		TESAmmo* ammo308 = dataHandler->LookupForm<TESAmmo>(0x01F66B, RETAIL_ESM01);
		TESAmmo* ammo38 = dataHandler->LookupForm<TESAmmo>(0x04CE87, RETAIL_ESM01);
		TESAmmo* ammo44 = dataHandler->LookupForm<TESAmmo>(0x09221C, RETAIL_ESM01);
		TESAmmo* ammo45 = dataHandler->LookupForm<TESAmmo>(0x01F66A, RETAIL_ESM01);

		ammoDegradationMap[ammo10mm] = 0.005f;
		ammoDegradationMap[ammo2mmEC] = 0.04f;
		ammoDegradationMap[ammo308] = 0.0133f;
		ammoDegradationMap[ammo38] = 0.003f;
		ammoDegradationMap[ammo44] = 0.011f;
		ammoDegradationMap[ammo45] = 0.0035f;

		REX::INFO("Item Degradation: Finished linking degradation forms.");
	}
}