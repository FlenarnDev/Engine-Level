#pragma once

#include "Shared/SharedDeclarations.h"

namespace RE
{
	namespace Cascadia
	{
		;


		static Setting* pLockLevelNamesExtended[10] = { nullptr };

		void InitializeLockLevelNamesArray();

		// Extended lock level enum with 'Very Easy' support.
		enum class LOCK_LEVEL_EXTENDED : std::int32_t
		{
			kVeryEasy = 0x0,
			kEasy = 0x1,
			kAverage = 0x2,
			kHard = 0x3,
			kVeryHard = 0x4,
			kRequiresKey = 0x5,
			kInaccessible = 0x6,
			kTerminal = 0x7,
			kBarred = 0x8,
			kChained = 0x9,
			kCount,
			kUnlocked = static_cast<std::underlying_type_t<LOCK_LEVEL_EXTENDED>>(-1),
		};

		inline int GetLockLevelMaxEasy()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxEasy");
			return setting ? setting->GetInt() : 25;
		}

		inline int GetLockLevelMaxAverage()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxAverage");
			return setting ? setting->GetInt() : 50;
		}

		inline int GetLockLevelMaxHard()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxHard");
			return setting ? setting->GetInt() : 75;
		}

		inline int GetLockLevelMaxVeryHard()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxVeryHard");
			return setting ? setting->GetInt() : 100;
		}

		inline int GetLockLevelMaxBarred()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxBarred");
			return setting ? setting->GetInt() : 251;
		}

		inline int GetLockLevelMaxChained()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxChained");
			return setting ? setting->GetInt() : 254;
		}

		inline int GetLockLevelMaxTerminal()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxTerminal");
			return setting ? setting->GetInt() : 252;
		}

		inline int GetLockLevelMaxInaccessible()
		{
			static Setting* setting = GameSettingCollection::GetSingleton()->GetSetting("iLockLevelMaxInaccessible");
			return setting ? setting->GetInt() : 253;
		}
	}
}