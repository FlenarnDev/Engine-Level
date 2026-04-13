#pragma once

#include "Shared/SharedFunctions.h"

namespace Cascadia
{
	namespace ExamineConfirmMenu
	{
		class OnEscapePress : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				REX::DEBUG("this.BGSCodeObj.OnEscapePress");
				Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();
				if (examineMenu)
				{
					REX::DEBUG("Examine menu!");
					examineMenu->repairing = false;
				}
			}
		};

		bool RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value)
		{
			Scaleform::GFx::Value currentSWFPath;
			// Register native code handlers.
			if (a_view->asMovieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url"))
			{
				if (_stricmp(currentSWFPath.GetString(), "Interface/ExamineConfirmMenu.swf") == 0)
				{
					Scaleform::GFx::Value bgsCodeObj;
					a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.Menu_mc.BGSCodeObj");

					Shared::RegisterFunction<OnEscapePress>(&bgsCodeObj, a_view->asMovieRoot, "OnEscapePress");
				}
				return true;
			}
			return false;
		}
	}
}