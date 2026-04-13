#pragma once

using namespace RE;

namespace Cascadia
{
	namespace ObScript
	{
		class GetBaseObject
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
						return _stricmp(a_elem.functionName, "IgnoreRenderPass") == 0;
					});

				if (it != functions.end())
				{
					*it = SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
					it->helpString = HelpString().data();
					it->referenceFunction = true;
					it->executeFunction = Execute;

					REX::DEBUG("Registered 'GetBaseObject' command.");
				}
				else
				{
					REX::DEBUG("WARNING: Failed to register 'GetBaseObject' command.");
				}
			}

		private:
			static bool Execute(
				const SCRIPT_PARAMETER*,
				const char*,
				TESObjectREFR* a_refObject,
				TESObjectREFR*,
				Script*,
				ScriptLocals*,
				float&,
				std::uint32_t&)
			{
				if (!a_refObject)
				{
					return true;
				}

				auto base = a_refObject->data.objectReference;
				if (!base)
				{
					return true;
				}

				auto result = std::format("GetBaseObject >> {:08X}"sv, base->formID);
				ConsoleLog::GetSingleton()->PrintLine(result.data());
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Print the FormID of an ObjectReference's base Form."sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "GetBaseObject"sv;
			static constexpr auto SHORT_NAME = "gbo"sv;
		};
	}
}