#pragma once

#include "Systems/Dialogue.h"

namespace Cascadia
{
	namespace DialogueMenu
	{
		std::pair<float, float> savedSubtitlePosition;

		class SetWheelZoomEnabled : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.argCount < 1) return;
				if (!a_params.args[0].IsBoolean()) return;

				bool enabled = a_params.args[0].GetBoolean();

				BSFixedString menuString("DialogueMenu");
				if (UI::GetSingleton()->GetMenuOpen(menuString))
				{
					Scaleform::Ptr<RE::DialogueMenu> dialogueMenu = UI::GetSingleton()->GetMenu<RE::DialogueMenu>();
					BSTSmartPointer<BSInputEnableLayer> menuInputLayer = dialogueMenu.get()->inputLayer;

					BSInputEnableManager::GetSingleton()->EnableUserEvent(menuInputLayer->layerID, UserEvents::USER_EVENT_FLAG::kWheelZoom, enabled, UserEvents::SENDER_ID::kMenu);
				}
			}
		};

		class SetFavoritesEnabled : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.argCount < 1) return;
				if (!a_params.args[0].IsBoolean()) return;

				bool enabled = a_params.args[0].GetBoolean();

				BSFixedString menuString("DialogueMenu");
				if (UI::GetSingleton()->GetMenuOpen(menuString))
				{
					Scaleform::Ptr<RE::DialogueMenu> dialogueMenu = UI::GetSingleton()->GetMenu<RE::DialogueMenu>();
					BSTSmartPointer<BSInputEnableLayer> menuInputLayer = dialogueMenu.get()->inputLayer;

					BSInputEnableManager::GetSingleton()->EnableOtherEvent(menuInputLayer->layerID, OtherInputEvents::OTHER_EVENT_FLAG::kFavorites, enabled, UserEvents::SENDER_ID::kMenu);
				}
			}
		};

		class SetMovementEnabled : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.argCount < 1) return;
				if (!a_params.args[0].IsBoolean()) return;

				bool enabled = a_params.args[0].GetBoolean();

				BSFixedString menuString("DialogueMenu");
				if (UI::GetSingleton()->GetMenuOpen(menuString))
				{
					Scaleform::Ptr<RE::DialogueMenu> dialogueMenu = UI::GetSingleton()->GetMenu<RE::DialogueMenu>();
					BSTSmartPointer<BSInputEnableLayer> menuInputLayer = dialogueMenu.get()->inputLayer;

					BSInputEnableManager::GetSingleton()->EnableUserEvent(menuInputLayer->layerID, UserEvents::USER_EVENT_FLAG::kMovement, enabled, UserEvents::SENDER_ID::kMenu);
				}
			}
		};

		class GetINISetting : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					*a_params.retVal = nullptr;

					if (a_params.argCount < 1 || !a_params.args[0].IsString())
					{
						return;
					}

					INIPrefSettingCollection* iniSettings = INIPrefSettingCollection::GetSingleton();
					if (iniSettings)
					{
						Setting* setting = iniSettings->GetSetting(a_params.args[0].GetString());
						std::uint32_t value;

						if (setting)
						{
							value = setting->GetInt();
							REX::DEBUG("DialogueMenu::GetINISetting called, requested setting: {:s}, returned: {}", a_params.args[0].GetString(), value);
							*a_params.retVal = value;
						}
						else
						{
							REX::WARN("DialogueMenu::GetINISetting called, requested setting: {:s} which was not found.", a_params.args[0].GetString());
						}
					}
				}
			}
		};

		class GetModSetting : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					*a_params.retVal = nullptr;

					if (a_params.argCount < 1 || !a_params.args[0].IsString())
					{
						return;
					}

					// Here we "cheat" once again, who needs configurable settings anyhow.
					std::string setting = Shared::trim(a_params.args[0].GetString());

					std::uint32_t value = 1;
					if (setting == "fSubtitlesX:DialogueMenu" || setting == "bShowOptionNumbers:DialogueMenu" || setting == "bEnable:Debug")
					{
						value = 0;
					}
					else if (setting == "fSubtitlesY:DialogueMenu")
					{
						value = 40;
					}

					*a_params.retVal = value;
				}
			}
		};

		class GetSubtitlePosition_GFx : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					*a_params.retVal = nullptr;

					BSFixedString menuString("HUDMenu");
					if (UI::GetSingleton()->GetMenuOpen(menuString))
					{
						IMenu* menu = UI::GetSingleton()->GetMenu(menuString).get();
						Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot = menu->uiMovie->asMovieRoot;

						movieRoot->CreateArray(a_params.retVal);

						Scaleform::GFx::Value subtitleX;
						movieRoot->GetVariable(&subtitleX, "root.BottomCenterGroup_mc.SubtitleText_mc.x");
						Scaleform::GFx::Value subtitleY;
						movieRoot->GetVariable(&subtitleY, "root.BottomCenterGroup_mc.SubtitleText_mc.y");

						a_params.retVal->PushBack(subtitleX);
						a_params.retVal->PushBack(subtitleY);
					}
					else
					{
						REX::DEBUG("Unable to retrieve the subtitle position because `HUDMenu` is not open.");
					}
				}
			}
		};

		class GetTargetName : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					const char* result = "";
					if (TESObjectREFR* target = Dialogue::GetCurrentPlayerDialogueTarget())
					{
						result = target->GetDisplayFullName();
						REX::DEBUG("DialogueMenu::GetTargetName called, target name: {:s}", result);
					}
					else
					{
						REX::WARN("DialogueMenu::GetTargetName called, no target name found.");
						result = "Error: No name found.";
					}

					*a_params.retVal = result;
				}
			}
		};

		class GetTargetType : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					std::uint8_t result = 0;
					if (TESObjectREFR* target = Dialogue::GetCurrentPlayerDialogueTarget())
					{
						result = target->formType.underlying();
					}

					*a_params.retVal = result;
				}
			}
		};

		class IsFrameworkActive : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					// We "cheat" here, we always return true since we're a TC, our theoretical impact on the retail game doesn't matter.
					*a_params.retVal = true;
				}
			}
		};

		class SelectDialogueOption_GFx : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				REX::DEBUG("SelectDialogueOption_GFx called.");
				if (a_params.retVal)
				{
					if (a_params.argCount < 1) return;
					if (!a_params.args[0].IsUInt()) return;

					*a_params.retVal = Dialogue::SelectDialogueOption(a_params.args[0].GetUInt());
				}
			}
		};

		class SetSubtitlePosition_GFx : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					if (a_params.argCount < 2) return;
					if (!a_params.args[0].IsNumber()) return;
					if (!a_params.args[1].IsNumber()) return;

					BSFixedString menuString("HUDMenu");
					if (UI::GetSingleton()->GetMenuOpen(menuString))
					{
						IMenu* menu = UI::GetSingleton()->GetMenu(menuString).get();
						Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot = menu->uiMovie->asMovieRoot;

						Scaleform::GFx::Value subtitle;
						movieRoot->GetVariable(&subtitle, "root.BottomCenterGroup_mc.SubtitleText_mc");
						subtitle.SetMember("x", a_params.args[1]);
						subtitle.SetMember("y", a_params.args[1]);

						*a_params.retVal = true;
					}
				}
			}
		};

		class GetDialogueOptions_GFx : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.retVal)
				{
					Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot = a_params.movie->asMovieRoot;

					movieRoot->CreateArray(a_params.retVal);
					if (BGSSceneActionPlayerDialogue* playerDialogue = Dialogue::GetCurrentPlayerDialogueAction())
					{
						std::vector<Dialogue::DialogueOption> options = Dialogue::GetDialogueOptions();
						for (Dialogue::DialogueOption dialogueOption : options)
						{
							Scaleform::GFx::Value optionIDValue, promptValue, responseValue, enabledValue, saidValue, challengeLevelValue, challengeResultValue, linkedToSelfValue, endsScene, isBarterOption, isInventoryOption;

							optionIDValue = dialogueOption.optionID;
							promptValue = dialogueOption.prompText.c_str();
							responseValue = dialogueOption.responseText.c_str();
							enabledValue = dialogueOption.enabled;
							saidValue = dialogueOption.said;
							challengeLevelValue = dialogueOption.challengeLevel;
							challengeResultValue = dialogueOption.challengeResult;
							linkedToSelfValue = dialogueOption.linkedToSelf;
							endsScene = dialogueOption.endsScene;
							isBarterOption = dialogueOption.isBarterOption;
							isInventoryOption = dialogueOption.isInventoryOption;

							Scaleform::GFx::Value dialogueValue;
							movieRoot->CreateObject(&dialogueValue);
							dialogueValue.SetMember("optionID", optionIDValue);
							dialogueValue.SetMember("prompt", promptValue);
							dialogueValue.SetMember("response", responseValue);
							dialogueValue.SetMember("enabled", enabledValue);
							dialogueValue.SetMember("said", saidValue);
							dialogueValue.SetMember("challengeLevel", challengeLevelValue);
							dialogueValue.SetMember("challengeResult", challengeResultValue);
							dialogueValue.SetMember("linkedToSelf", linkedToSelfValue);
							dialogueValue.SetMember("endsScene", endsScene);
							dialogueValue.SetMember("isBarterOption", isBarterOption);
							dialogueValue.SetMember("isInventoryOption", isInventoryOption);

							a_params.retVal->PushBack(dialogueValue);
						}
					}
					else
					{
						REX::WARN("DialogueMenu::GetDialogueOptions - Player dialogue not currently available. No dialogue will be retrieved.");
					}
				}
			}
		};

		class SetXDIResult : public Scaleform::GFx::FunctionHandler
		{
		public:
			virtual void Call(const Params& a_params)
			{
				if (a_params.argCount < 1) return;
				if (!a_params.args[0].IsNumber()) return;

				float value = a_params.args[0].GetNumber();
			}
		};

		// =================
		// General Functions
		// =================

		std::pair<float, float> GetSubtitlePosition()
		{
			std::pair<float, float> position;

			BSFixedString menuString("HUDMenu");
			if (UI::GetSingleton()->GetMenuOpen(menuString))
			{
				IMenu* menu = UI::GetSingleton()->GetMenu(menuString).get();
				Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot = menu->uiMovie->asMovieRoot;

				Scaleform::GFx::Value subtitleX;
				movieRoot->GetVariable(&subtitleX, "root.BottomCenterGroup_mc.SubtitleText_mc.x");
				Scaleform::GFx::Value subtitleY;
				movieRoot->GetVariable(&subtitleY, "root.BottomCenterGroup_mc.SubtitleText_mc.y");

				position.first = subtitleX.GetNumber();
				position.second = subtitleY.GetNumber();
			}
			else
			{
				REX::WARN("WARNING: Unable to retrieve the subtitle position because 'HUDMenu' is not open.");
			}

			return position;
		}

		bool SetSubtitlePosition(double x, double y)
		{
			BSFixedString menuString("HUDMenu");
			if (UI::GetSingleton()->GetMenuOpen(menuString))
			{
				IMenu* menu = UI::GetSingleton()->GetMenu(menuString).get();
				Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot = menu->uiMovie->asMovieRoot;

				Scaleform::GFx::Value subtitle;
				movieRoot->GetVariable(&subtitle, "root.BottomCenterGroup_mc.SubtitleText_mc");
				subtitle.SetMember("x", Scaleform::GFx::Value(x));
				subtitle.SetMember("y", Scaleform::GFx::Value(y));
				return true;
			}

			return false;
		}

		// =================

		bool RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value)
		{
			Scaleform::GFx::Value currentSWFPath;

			// Register native code handlers.
			if (a_view->asMovieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url"))
			{
				if (_stricmp(currentSWFPath.GetString(), "Interface/DialogueMenu.swf") == 0)
				{
					Scaleform::GFx::Value bgsCodeObj;
					a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.Menu_mc.BGSCodeObj");

					Shared::RegisterFunction<GetINISetting>(&bgsCodeObj, a_view->asMovieRoot, "GetINISetting");
					Shared::RegisterFunction<GetModSetting>(&bgsCodeObj, a_view->asMovieRoot, "GetModSetting");

					Shared::RegisterFunction<SetMovementEnabled>(&bgsCodeObj, a_view->asMovieRoot, "SetMovementEnabled");

					Shared::RegisterFunction<GetSubtitlePosition_GFx>(&bgsCodeObj, a_view->asMovieRoot, "GetSubtitlePosition");
					Shared::RegisterFunction<SetSubtitlePosition_GFx>(&bgsCodeObj, a_view->asMovieRoot, "SetSubtitlePosition");

					Shared::RegisterFunction<GetTargetName>(&bgsCodeObj, a_view->asMovieRoot, "GetTargetName");
					Shared::RegisterFunction<GetTargetType>(&bgsCodeObj, a_view->asMovieRoot, "GetTargetType");

					Shared::RegisterFunction<IsFrameworkActive>(&bgsCodeObj, a_view->asMovieRoot, "IsFrameworkActive");
					Shared::RegisterFunction<SelectDialogueOption_GFx>(&bgsCodeObj, a_view->asMovieRoot, "SelectDialogueOption");
					Shared::RegisterFunction<GetDialogueOptions_GFx>(&bgsCodeObj, a_view->asMovieRoot, "GetDialogueOptions");

					Shared::RegisterFunction<SetWheelZoomEnabled>(&bgsCodeObj, a_view->asMovieRoot, "SetWheelZoomEnabled");
					Shared::RegisterFunction<SetFavoritesEnabled>(&bgsCodeObj, a_view->asMovieRoot, "SetFavoritesEnabled");

					Shared::RegisterFunction<SetXDIResult>(&bgsCodeObj, a_view->asMovieRoot, "SetXDIResult");

					a_view->asMovieRoot->Invoke("root.XDI_Init", nullptr, nullptr, 0);
				}
				return true;
			}
			return false;
		}
	}
}
