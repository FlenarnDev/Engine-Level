#include "Systems/LockLevels.h"

namespace Cascadia
{
	namespace LockLevels
	{
        void InitializeLockLevelNamesArray()
        {
            GameSettingCollection* settings = GameSettingCollection::GetSingleton();

            static Setting veryEasySetting("sLockLevelNameVeryEasy", "Very Easy");
            g_lockLevelNamesStub[0] = &veryEasySetting;
            g_lockLevelNamesStub[1] = settings->GetSetting("sLockLevelNameEasy");
            g_lockLevelNamesStub[2] = settings->GetSetting("sLockLevelNameAverage");
            g_lockLevelNamesStub[3] = settings->GetSetting("sLockLevelNameHard");
            g_lockLevelNamesStub[4] = settings->GetSetting("sLockLevelNameVeryHard");
            g_lockLevelNamesStub[5] = settings->GetSetting("sLockLevelNameImpossible");
            g_lockLevelNamesStub[6] = settings->GetSetting("sLockLevelNameInaccessible");
            g_lockLevelNamesStub[7] = settings->GetSetting("sLockLevelNameTerminal");
            g_lockLevelNamesStub[8] = settings->GetSetting("sLockLevelNameBarred");
            g_lockLevelNamesStub[9] = settings->GetSetting("sLockLevelNameChained");
        }
	}
}