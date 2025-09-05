#pragma once

#include "Menus/LevelUpMenu.h"

namespace ObScript
{
	class TestLoadingMenu
	{
	public:
		static void Install()
		{
			const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
			const auto it = std::find_if(
				functions.begin(),
				functions.end(),
				[&](auto&& a_elem)
				{
					return _stricmp(a_elem.functionName, "TestLoadingMenu") == 0;
				});
			if (it != functions.end())
			{
				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
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
			const RE::SCRIPT_PARAMETER* a_parameters,
			const char* a_compiledParams,
			RE::TESObjectREFR* a_refObject,
			RE::TESObjectREFR* a_container,
			RE::Script* a_script,
			RE::ScriptLocals* a_scriptLocals,
			float&,
			std::uint32_t& a_offset)
		{
			RE::LoadingMenu::StartTestingLoadMenu();

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
		static constexpr auto SHORT_NAME = "TLM"sv;
	};
}