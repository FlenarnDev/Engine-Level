#pragma once

#include "Shared/SharedDeclarations.h"

namespace Cascadia
{
	namespace ObScript
	{
		class ToggleArmorDegradation
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
						return _stricmp(a_elem.functionName, "CaptureMessage") == 0;
					});

				if (it != functions.end())
				{
					*it = SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
					it->helpString = HelpString().data();
					it->referenceFunction = false;
					it->executeFunction = Execute;

					REX::DEBUG("Registered 'ToggleArmorDegradation' command.");
				}
				else
				{
					REX::WARN("WARNING: Failed to register 'ToggleArmorDegradation' command.");
				}
			}

		private:
			static bool Execute(
				const SCRIPT_PARAMETER*,
				const char*,
				TESObjectREFR*,
				TESObjectREFR*,
				Script*,
				ScriptLocals*,
				float&,
				std::uint32_t&)
			{
				Shared::noArmorDegradation = !Shared::noArmorDegradation;
				if (Shared::noArmorDegradation)
				{
					ConsoleLog::GetSingleton()->PrintLine("Armor degradation disabled.");
				}
				else
				{
					ConsoleLog::GetSingleton()->PrintLine("Armor degradation enabled.");
				}
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Toggle armor degradation"sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "ToggleArmorDegradation"sv;
			static constexpr auto SHORT_NAME = "tad"sv;
		};
	}

}