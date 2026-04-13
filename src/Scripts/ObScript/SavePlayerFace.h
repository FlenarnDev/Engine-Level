#pragma once

#include "Shared/NPCFileUtils.h"
#include "Shared/SharedFunctions.h"

namespace Cascadia
{
	namespace ObScript
	{
		class SavePlayerFace
		{
		public:
			static void Install()
			{
				const auto functions = SCRIPT_FUNCTION::GetScriptFunctions();
				const auto it = std::find_if(
					functions.begin(),
					functions.end(),
					[&](auto&& a_elem)
					{
						return _stricmp(a_elem.functionName, "SavePCFace") == 0;
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

					REX::DEBUG("Registered 'SavePCFace' command.");
				}
				else
				{
					REX::WARN("WARNING: Failed to register 'SavePCFace' command.");
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
				std::array<char, 0x200> rawComment{ '\0' };
				auto result = Script::ParseParameters(
					a_parameters,
					a_compiledParams,
					a_offset,
					a_refObject,
					a_container,
					a_script,
					a_scriptLocals,
					rawComment.data());

				if (!result)
				{
					return true;
				}

				if (rawComment[0] == '\0')
				{
					return true;
				}

				Actor* a_actor = PlayerCharacter::GetSingleton();
				TESNPC* npc = a_actor->GetNPC();

				NPCFileUtils::NPCFile npcFile{};

				npcFile.bodyMorphRegionsA = NPCFileUtils::GetBodyMorphRegionsFromNPC(npc);  // Body Morph Regions
				npcFile.facialBoneMorphIntensity = npc->GetFacialBoneMorphIntensity();  // Facial Bone Morph Intensity
				npcFile.facialBoneRegionsA = NPCFileUtils::GetFacialBoneRegionsFromNPC(npc);  // Facial Bone Regions
				npcFile.morphSlidersA = NPCFileUtils::GetMorphSliderValuesFromNPC(npc);  //Morph Sliders
				npcFile.tintEntriesA = NPCFileUtils::GetTintEntriesFromNPC(npc, std::to_underlying(npc->GetSex()));  // Tint Entries

				NPCFileExport::ExportNPCFile(npcFile, rawComment.data());

				return true;
			}

			[[nodiscard]] static const std::string& HelpString()
			{
				static auto help = []()
					{
						std::string buf;
						buf += "Exports the Player's face as "sv;
						buf += "a .npc file for use in importing"sv;
						buf += " into the Creation Kit."sv;
						return buf;
					}();
				return help;
			}

			static constexpr auto LONG_NAME = "SavePCFace"sv;
			static constexpr auto SHORT_NAME = "spf"sv;

			inline static char _delim{ '\t' };

			inline static std::ofstream _file;
		};
	}
}