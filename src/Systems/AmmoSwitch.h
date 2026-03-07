#pragma once

namespace RE
{
	namespace Cascadia
	{
		namespace AmmoSwitch
		{
			extern bool switchingAmmo;
			extern TESAmmo* ammoToSwitchTo;
			extern BGSObjectInstance* equippedInstance;

			extern BSTArray<BGSKeyword*> keywordsAmmo;
			extern std::unordered_map<BGSKeyword*, BGSListForm*> keywordsAmmoCaliberMap;
			extern std::unordered_map<TESAmmo*, BGSKeyword*> keywordsAmmoOMODMap;
			extern BSTArray<BGSKeyword*> keywordsOMOD;
			extern BGSKeyword* noFormlistWEAP;
			extern BGSKeyword* uniqueFormlistWEAP;
			extern BGSKeyword* omodAPKeyword;
			extern std::vector<std::pair<BGSKeyword*, BGSMod::Attachment::Mod*>> omodFormListVec;
			extern std::unordered_map<BGSKeyword*, BGSMod::Attachment::Mod*> omodKeywordMap;

			bool InitializeAmmoSwitch();
			bool FindAmmoInFormlist(BGSKeyword* keyword, TESAmmo* currentAmmo, PlayerCharacter* playerCharacter);
			bool MaterialSwap(TESAmmo* currentAmmo);

			void DefineAmmoLists();

			void PostLoadGameAmmoFix();
		}
	} 
}