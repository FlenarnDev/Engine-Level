#include "ItemDegradation.h"

namespace RE
{
	namespace Cascadia
	{

		void DefineItemDegradationFormsFromGame()
		{
			REX::INFO("Item Degradation: Linking degradation forms.");
			TESDataHandler* dataHandler = TESDataHandler::GetSingleton();

			// Ammo types with respective degradation values
			TESAmmo* ammo10mm = dataHandler->LookupForm<TESAmmo>(0x01F276, "Fallout4.esm");
			TESAmmo* ammo2mmEC = dataHandler->LookupForm<TESAmmo>(0x18ABDF, "Fallout4.esm");
			TESAmmo* ammo308 = dataHandler->LookupForm<TESAmmo>(0x01F66B, "Fallout4.esm");
			TESAmmo* ammo38 = dataHandler->LookupForm<TESAmmo>(0x04CE87, "Fallout4.esm");
			TESAmmo* ammo44 = dataHandler->LookupForm<TESAmmo>(0x09221C, "Fallout4.esm");
			TESAmmo* ammo45 = dataHandler->LookupForm<TESAmmo>(0x01F66A, "Fallout4.esm");

			ammoDegradationMap[ammo10mm] = 0.005f;
			ammoDegradationMap[ammo2mmEC] = 0.04f;
			ammoDegradationMap[ammo308] = 0.0133f;
			ammoDegradationMap[ammo38] = 0.003f;
			ammoDegradationMap[ammo44] = 0.011f;
			ammoDegradationMap[ammo45] = 0.0035f;

			REX::INFO("Item Degradation: Finished linking degradation forms.");
		}
	}
}