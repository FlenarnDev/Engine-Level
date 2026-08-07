#pragma once

#include "../../Systems/Skills.h"

using namespace RE;

namespace Cascadia
{
	namespace ObScript
	{
		#define LOG_TO_CONSOLE(a) if (auto* console = RE::ConsoleLog::GetSingleton()) console->AddString(a);

		class ModCASValueCommand {
		public:
			static void Install() {
				const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem) {
						return _stricmp(a_elem.functionName, "ShowPivot") == 0;
					});

				if (it == functions.end()) {
					REX::WARN("Failed to reigster console command: 'ShowPivot'");
					return;
				}

				static std::array params{
					RE::SCRIPT_PARAMETER{"skillName", RE::SCRIPT_PARAM_TYPE::kChar, true},
					RE::SCRIPT_PARAMETER{"KarmaAmount", RE::SCRIPT_PARAM_TYPE::kInt, true},
				};

				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->paramCount = static_cast<std::uint16_t>(params.size());
				it->parameters = params.data();
				it->executeFunction = Execute;

				REX::INFO("Registered 'ShowPivot' console command");
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
				int karmaModAmount = 0;
				std::array<char, 0x200> skillName = { '\0' };

				auto paramsParsed = RE::Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					skillName.data(),
					&karmaModAmount
				);

				if (!paramsParsed || skillName[0] == '\0' || karmaModAmount == 0) {
					LOG_TO_CONSOLE("Mods the player's skill.\nModCASValue SKILL 100");
					// CW_SkillsPapyrus::DEBUG_LogSkillsToConsole_Papyrus(std::monostate(), RE::PlayerCharacter::GetSingleton());
					return true;
				}

				auto vm = RE::GameVM::GetSingleton()->GetVM();
				// PA_Globals.Karma->value += karmaModAmount;

				std::string nameS = skillName.data();

				auto skillAV = Skills::GetSkillByName(nameS);
				if (skillAV) {
					auto player = RE::PlayerCharacter::GetSingleton();
					player->SetBaseActorValue(*skillAV, karmaModAmount);

					// ModPermanentSkillValue(player, skillAV, karmaModAmount);
					//auto s = std::format("'{}' new value for player: {}", GetBaseAVValue()).c_str();
					//LOG_TO_CONSOLE();
				}
				else {
					auto s = std::format("Skill '{}' is not a valid CASSkill.", nameS).c_str();
					LOG_TO_CONSOLE(s);
				}


				// LOG_TO_CONSOLE(std::format("Old Karma: {}, New Karma: {}", oldKarma, PA_Globals.Karma->value).c_str());
				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Mods the player's skill/karma.\nModCASValue 100 OR ModCASValue -100"sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "ModCASValue"sv;
			static constexpr auto SHORT_NAME = "mcwv"sv;
		};
	}
}