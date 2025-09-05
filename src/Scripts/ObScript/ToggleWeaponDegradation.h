#pragma once

#include "Shared/SharedDeclarations.h"

namespace ObScript
{
	class ToggleWeaponDegradation
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
					return _stricmp(a_elem.functionName, "CaptureMessages") == 0;
				});

			if (it != functions.end())
			{
				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->executeFunction = Execute;

				REX::DEBUG("Registered 'ToggleWeaponDegradation' command.");
			}
			else
			{
				REX::WARN("WARNING: Failed to register 'ToggleWeaponDegradation' command.");
			}
		}

	private:
		static bool Execute(
			const RE::SCRIPT_PARAMETER*,
			const char*,
			RE::TESObjectREFR*,
			RE::TESObjectREFR*,
			RE::Script*,
			RE::ScriptLocals*,
			float&,
			std::uint32_t&)
		{
			RE::Cascadia::Shared::noWeaponDegradation = !RE::Cascadia::Shared::noWeaponDegradation;
			if (RE::Cascadia::Shared::noWeaponDegradation)
			{
				RE::ConsoleLog::GetSingleton()->PrintLine("Weapon degradation disabled.");
			}
			else
			{
				RE::ConsoleLog::GetSingleton()->PrintLine("Weapon degradation enabled.");
			}
			return true;
		}

		[[nodiscard]] static const std::string& HelpString()
		{
			static auto help = []()
				{
					std::string buf;
					buf += "Toggle weapon degradation"sv;
					return buf;
				}();
			return help;
		}

		static constexpr auto LONG_NAME = "ToggleWeaponDegradation"sv;
		static constexpr auto SHORT_NAME = "twd"sv;
	};
}
