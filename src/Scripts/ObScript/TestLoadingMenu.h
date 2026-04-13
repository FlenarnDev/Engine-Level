#pragma once

#include "Shared/SharedDeclarations.h"

namespace Cascadia
{
	namespace ObScript
	{
		class TestLoadingMenu
		{
		public:
			static void Install()
			{
				const auto functions = SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem)
					{
						return _stricmp(a_elem.functionName, "TestLoadingMenu") == 0;
					});
				if (it != functions.end())
				{
					static std::array params{
						SCRIPT_PARAMETER{"String", SCRIPT_PARAM_TYPE::kChar, false},
					};

					*it = SCRIPT_FUNCTION{ LONG_NAME.data(), "", it->output };
					it->helpString = HelpString().data();
					it->referenceFunction = false;
					it->paramCount = static_cast<std::uint16_t>(params.size());
					it->parameters = params.data();
					it->executeFunction = Execute;
					REX::DEBUG("Registered 'TestLoadingMenu' command.");
				}
				else
				{
					REX::WARN("Failed to register 'TestLoadingMenu' command.");
				}
			}
		private:
			static bool Execute(
				const SCRIPT_PARAMETER* a_parameters,
				const char* a_compiledParams,
				TESObjectREFR* a_refObject,
				TESObjectREFR* a_container,
				Script* a_script,
				ScriptLocals* a_scriptLocals,
				float&,
				std::uint32_t& a_offset)
			{
				char loadScreenName[512]{ '\0' };
				Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					loadScreenName);

				if (loadScreenName[0] == '\0')
				{
					return true;
				}

				if (auto form = RE::TESForm::GetFormByEditorID(loadScreenName))
				{
					if (auto loadscreen = form->As<RE::TESLoadScreen>(); loadscreen)
					{
						Shared::forcedLoadScreen = loadscreen;
						LoadingMenu::StartTestingLoadMenu();
						return true;
					}
				}
				auto result = std::format("TestLoadingMenu ({:s}) >> Does not exist."sv, loadScreenName);
				ConsoleLog::GetSingleton()->PrintLine(result.data());
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Test Loading Menu"sv;
						return buf;
					}();
				return help;
			}
			static constexpr auto LONG_NAME = "TestLoadingMenu"sv;
		};
	}
}