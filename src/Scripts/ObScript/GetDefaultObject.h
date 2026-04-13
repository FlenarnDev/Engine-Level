#pragma once

using namespace RE;

namespace Cascadia
{
	namespace ObScript
	{
		class GetDefaultObject
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
						return _stricmp(a_elem.functionName, "DumpSoundMap") == 0;
					});

				if (it != functions.end())
				{
					static std::array params{
						SCRIPT_PARAMETER{"String", SCRIPT_PARAM_TYPE::kChar, false},
					};

					*it = SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
					it->helpString = HelpString().data();
					it->referenceFunction = false;
					it->paramCount = static_cast<std::uint16_t>(params.size());
					it->parameters = params.data();
					it->executeFunction = Execute;

					REX::DEBUG("Registered 'GetDefaultObject' command.");
				}
				else
				{
					REX::DEBUG("Failed to register 'GetDefaultObject' command.");
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
				char dfobName[512]{ '\0' };
				Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					dfobName);

				if (dfobName[0] == '\0')
				{
					return true;
				}

				if (auto form = TESForm::GetFormByEditorID(dfobName))
				{
					if (auto dfob = form->As<RE::BGSDefaultObject>(); dfob)
					{
						auto result = std::format(
							"GetDefaultObject ({:s}) >> 0x{:08X}"sv,
							dfob->formEditorID.c_str(),
							dfob->form ? dfob->form->formID : 0);
						ConsoleLog::GetSingleton()->PrintLine(result.data());
						return true;
					}
				}

				auto result = std::format("GetDefaultObject ({:s}) >> Does not exist."sv, dfobName);
				ConsoleLog::GetSingleton()->PrintLine(result.data());
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Print the FormID contained by a DefaultObject Form. "sv;
						buf += "[GetDefaultObject \"InventoryWeight_DO\"]"sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "GetDefaultObject"sv;
			static constexpr auto SHORT_NAME = "gdo"sv;
		};
	}
}