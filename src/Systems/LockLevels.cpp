#include "Systems/LockLevels.h"

namespace RE
{
	namespace Cascadia
	{
        void InitializeLockLevelNamesArray()
        {
            GameSettingCollection* settings = GameSettingCollection::GetSingleton();

            static Setting veryEasySetting("sLockLevelNameVeryEasy", "Very Easy");
            Shared::g_lockLevelNamesStub[0] = &veryEasySetting;
            Shared::g_lockLevelNamesStub[1] = settings->GetSetting("sLockLevelNameEasy");
            Shared::g_lockLevelNamesStub[2] = settings->GetSetting("sLockLevelNameAverage");
            Shared::g_lockLevelNamesStub[3] = settings->GetSetting("sLockLevelNameHard");
            Shared::g_lockLevelNamesStub[4] = settings->GetSetting("sLockLevelNameVeryHard");
            Shared::g_lockLevelNamesStub[5] = settings->GetSetting("sLockLevelNameImpossible");
            Shared::g_lockLevelNamesStub[6] = settings->GetSetting("sLockLevelNameInaccessible");
            Shared::g_lockLevelNamesStub[7] = settings->GetSetting("sLockLevelNameTerminal");
            Shared::g_lockLevelNamesStub[8] = settings->GetSetting("sLockLevelNameBarred");
            Shared::g_lockLevelNamesStub[9] = settings->GetSetting("sLockLevelNameChained");
        }
	}
}