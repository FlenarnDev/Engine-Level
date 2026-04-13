#include "Shared/NPCFileUtils.h"
#include "Shared/SharedFunctions.h"

namespace Cascadia
{
	namespace NPCFileUtils
	{
		BodyMorphRegions GetBodyMorphRegionsFromNPC(TESNPC* npc)
		{
			if (!npc->morphSliderValues || npc->morphSliderValues->size() == 0)
			{
				return NPCFileDefaults::DefaultBodyMorphRegions(std::to_underlying(npc->GetSex()));
			}

			BodyMorphRegions result{};

			auto headIter = npc->morphSliderValues->find(0);
			if (headIter != npc->morphSliderValues->end())
			{
				result.head = headIter->second;
			}

			auto upperTorsoIter = npc->morphSliderValues->find(1);
			if (upperTorsoIter != npc->morphSliderValues->end())
			{
				result.upperTorso = upperTorsoIter->second;
			}

			auto armsIter = npc->morphSliderValues->find(2);
			if (armsIter != npc->morphSliderValues->end())
			{
				result.arms = armsIter->second;
			}

			auto lowerTorsoIter = npc->morphSliderValues->find(3);
			if (lowerTorsoIter != npc->morphSliderValues->end())
			{
				result.lowerTorso = lowerTorsoIter->second;
			}

			auto legsIter = npc->morphSliderValues->find(4);
			if (legsIter != npc->morphSliderValues->end())
			{
				result.legs = legsIter->second;
			}

			if (result.head == 0.0f && result.upperTorso == 0.0f && result.arms == 0.0f && result.lowerTorso == 0.0f && result.legs == 0.0f)
			{
				return NPCFileDefaults::DefaultBodyMorphRegions(std::to_underlying(npc->GetSex()));
			}

			return result;
		}

		std::vector<MorphSlider> GetMorphSliderValuesFromNPC(TESNPC* npc)
		{
			std::vector<MorphSlider> result;

			if (!npc->morphSliderValues)
			{
				return result;
			}

			for (auto it = npc->morphSliderValues->cbegin(), end = npc->morphSliderValues->cend(); it != end; ++it)
			{
				MorphSlider currentSlider{};

				currentSlider.id = it->first;
				currentSlider.value = it->second;
				result.push_back(currentSlider);
			}

			return result;
		}

		std::vector<FacialBoneRegion> GetFacialBoneRegionsFromNPC(TESNPC* npc)
		{
			std::vector<FacialBoneRegion> result;

			if (!npc->facialBoneRegionSliderValues)
			{
				return result;
			}

			std::uint32_t i = 0;

			for (auto it = npc->facialBoneRegionSliderValues->cbegin(), end = npc->facialBoneRegionSliderValues->cend(); it != end; ++it)
			{
				FacialBoneRegion currentRegion{};
				currentRegion.id = it->first;

				currentRegion.position = it->second.position;
				currentRegion.rotation = it->second.rotation;
				currentRegion.scale = it->second.scale;

				i++;
				result.push_back(currentRegion);
			}
			return result;
		}

		std::vector<TintIDSet> GetTintIDsFromRace(TESRace* race, std::uint32_t gender)
		{
			std::vector<TintIDSet> result;

			for (std::uint32_t x = 0; x < race->faceRelatedData[gender]->tintingTemplate[0].groups.size(); x++)
			{
				const char* category = race->faceRelatedData[gender]->tintingTemplate[0].groups[x]->name.c_str();

				for (std::uint32_t y = 0; y < race->faceRelatedData[gender]->tintingTemplate[0].groups[x]->entries.size(); y++)
				{
					TintIDSet currentSet{};

					currentSet.category = category;
					currentSet.id = race->faceRelatedData[gender]->tintingTemplate[0].groups[x]->entries[y]->uniqueID;
					currentSet.name = race->faceRelatedData[gender]->tintingTemplate[0].groups[x]->entries[y]->name.c_str();

					result.push_back(currentSet);
				}
			}

			return result;
		}

		std::vector<TintEntry> GetTintEntriesFromNPC(TESNPC* npc, std::uint32_t gender)
		{
			std::vector<TintEntry> result;

			if (!npc->tintingData)
			{
				return result;
			}

			TESRace* race = npc->GetFormRace();
			std::vector<TintIDSet> tintIDSet = GetTintIDsFromRace(race, gender);

			for (std::uint32_t i = 0; i < npc->tintingData->entriesA.size(); i++)
			{
				BGSCharacterTint::Entry* entry = npc->tintingData->entriesA[i];
				TintEntry currentTintEntry{};

				if (entry->GetType() == BGSCharacterTint::EntryType::kPalette)
				{
					BGSCharacterTint::PaletteEntry* palette = static_cast<BGSCharacterTint::PaletteEntry*>(entry);

					currentTintEntry.color = palette->tintingColor;
					currentTintEntry.swatch = palette->swatchID;
				}

				currentTintEntry.id = entry->idLink;

				for (std::uint32_t j = 0; j < tintIDSet.size(); j++)
				{
					if (currentTintEntry.id == tintIDSet[j].id)
					{
						currentTintEntry.name = tintIDSet[j].name;
						break;
					}
				}

				currentTintEntry.value = entry->tingingValue;

				result.push_back(currentTintEntry);
			}

			return result;
		}
	}

	namespace NPCFileDefaults
	{
		NPCFileUtils::BodyMorphRegions DefaultBodyMorphRegions(std::uint32_t gender)
		{
			NPCFileUtils::BodyMorphRegions result{};

			switch (gender)
			{
			case 1:
				result.head = 0.41f;
				break;
			default:
				result.lowerTorso = -0.16f;
				result.legs = -0.47f;
				break;
			}

			return result;
		}

		std::vector<NPCFileUtils::FacialBoneRegion> DefaultFacialBoneRegions(std::uint32_t gender)
		{
			std::vector<NPCFileUtils::FacialBoneRegion> regions;

			switch (gender)
			{
			case 1:
			{
				// Female
				NPCFileUtils::FacialBoneRegion femaleRegions[32];

				femaleRegions[0].id = 100000;
				femaleRegions[1].id = 100001;
				femaleRegions[2].id = 100002;
				femaleRegions[3].id = 100003;
				femaleRegions[4].id = 100004;
				femaleRegions[5].id = 100005;
				femaleRegions[6].id = 100006;
				femaleRegions[7].id = 100007;
				femaleRegions[8].id = 100008;
				femaleRegions[9].id = 100009;
				femaleRegions[10].id = 100010;
				femaleRegions[11].id = 100011;
				femaleRegions[12].id = 100012;
				femaleRegions[13].id = 100013;
				femaleRegions[14].id = 100014;
				femaleRegions[15].id = 100015;
				femaleRegions[16].id = 100016;
				femaleRegions[17].id = 100027;
				femaleRegions[18].id = 100017;
				femaleRegions[19].id = 100018;
				femaleRegions[20].id = 100019;
				femaleRegions[21].id = 100020;
				femaleRegions[22].id = 100021;
				femaleRegions[23].id = 100022;
				femaleRegions[24].id = 100023;
				femaleRegions[25].id = 100024;
				femaleRegions[26].id = 100025;
				femaleRegions[27].id = 100026;
				femaleRegions[28].id = 58;
				femaleRegions[29].id = 59;
				femaleRegions[30].id = 10062;
				femaleRegions[31].id = 10063;

				for (std::uint32_t i = 0; i < 32; i++)
				{
					regions.push_back(femaleRegions[i]);
				}
			}
			default:
			{
				// Male
				NPCFileUtils::FacialBoneRegion maleRegions[32];

				maleRegions[0].id = 0;
				maleRegions[1].id = 1;
				maleRegions[2].id = 2;
				maleRegions[3].id = 3;
				maleRegions[4].id = 4;
				maleRegions[5].id = 5;
				maleRegions[6].id = 6;
				maleRegions[7].id = 7;
				maleRegions[8].id = 8;
				maleRegions[9].id = 9;
				maleRegions[10].id = 10;
				maleRegions[11].id = 927646266;
				maleRegions[12].id = 921646267;
				maleRegions[13].id = 921646268;
				maleRegions[14].id = 921646269;
				maleRegions[15].id = 921646270;
				maleRegions[16].id = 921646271;
				maleRegions[17].id = 921646272;
				maleRegions[18].id = 921646273;
				maleRegions[19].id = 921646274;
				maleRegions[20].id = 921646275;
				maleRegions[21].id = 921646276;
				maleRegions[22].id = 921646277;
				maleRegions[23].id = 921646278;
				maleRegions[24].id = 921646279;
				maleRegions[25].id = 25;
				maleRegions[26].id = 26;
				maleRegions[27].id = 27;
				maleRegions[28].id = 60;
				maleRegions[29].id = 61;
				maleRegions[30].id = 62;
				maleRegions[31].id = 63;

				for (std::uint32_t i = 0; i < 32; i++)
				{
					regions.push_back(maleRegions[i]);
				}
			}
			}

			return regions;
		}

		std::stringstream DefaultMorphSliders()
		{
			std::stringstream result;

			result << "   \"MorphSlidersA\" : null," << std::endl;

			return result;
		}

		std::vector<NPCFileUtils::TintEntry> DefaultTintEntries()
		{
			std::vector<NPCFileUtils::TintEntry> tints;

			NPCFileUtils::TintEntry entry1;
			entry1.id = 1228;
			entry1.name = "Average";
			entry1.value = 1;

			NPCFileUtils::TintEntry entry2;

			entry2.color = 6439796;
			entry2.id = 1558;
			entry2.name = "Lip Color";
			entry2.swatch = 1658;
			entry2.value = 0.52;

			NPCFileUtils::TintEntry entry3;

			entry3.id = 1377;
			entry3.name = "Forehead Blemishes 4";
			entry3.value = 0.33;

			tints.push_back(entry1);
			tints.push_back(entry2);
			tints.push_back(entry3);

			return tints;
		}
	}

	namespace NPCFileExport
	{
		void ExportNPCFile(NPCFileUtils::NPCFile npcFile, const char* fileName)
		{
			Actor* a_actor = PlayerCharacter::GetSingleton();
			TESNPC* npc = a_actor->GetNPC();

			char outputFile[REX::W32::MAX_PATH] = "";
			strcat_s(outputFile, fileName);
			strcat_s(outputFile, ".npc");

			std::ofstream output(outputFile, std::ios_base::binary | std::ios_base::out);

			std::span<BGSHeadPart*> headParts = npc->GetHeadParts();

			output << "{" << std::endl;
			output << GenerateBodyMorphRegionsOutput(npcFile);
			output << GenerateComplexionOutput(npc);
			output << GenerateFacialBoneMorphIntensity(npcFile);
			output << GenerateFacialBoneRegions(npcFile);
			output << GenerateHairColor(npc, false);
			output << GenerateHairColor(npc, true);
			output << GenerateHeadPart(npc, RE::BGSHeadPart::HeadPartType::kEyebrows);
			output << GenerateHeadPart(npc, RE::BGSHeadPart::HeadPartType::kEyes);
			output << GenerateHeadPart(npc, RE::BGSHeadPart::HeadPartType::kFace);
			output << GenerateHeadPart(npc, RE::BGSHeadPart::HeadPartType::kFacialHair);
			output << GenerateHeadPart(npc, RE::BGSHeadPart::HeadPartType::kHair);
			output << GenerateHeadPart(npc, RE::BGSHeadPart::HeadPartType::kHeadRear);
			output << GenerateHeadPart(npc, RE::BGSHeadPart::HeadPartType::kTeeth);
			output << GenerateMorphSliders(npcFile);
			output << GenerateRace(npc);
			output << GenerateSex(npc);
			output << GenerateTintEntries(npcFile);
			output << GenerateWeight(npc);
			output << "}";
			output << std::endl;

			output.close();
		}

		std::string GenerateBodyMorphRegionsOutput(NPCFileUtils::NPCFile npcFile)
		{
			NPCFileUtils::BodyMorphRegions regions = npcFile.bodyMorphRegionsA;

			std::stringstream result;

			result << "   \"BodyMorphRegionsA\" : [ ";

			result << std::setprecision(17) << (regions.head);
			result << ", ";
			result << std::setprecision(17) << (regions.upperTorso);
			result << ", ";
			result << std::setprecision(17) << (regions.arms);
			result << ", ";
			result << std::setprecision(17) << (regions.lowerTorso);
			result << ", ";
			result << std::setprecision(17) << (regions.legs);
			result << " ]," << std::endl;

			return result.str();
		}

		std::string GenerateComplexionOutput(TESNPC* npc)
		{
			std::stringstream result;

			result << "   \"Complexion\" : \"";

			if (npc->headRelatedData->faceDetails)
			{
				result << npc->headRelatedData->faceDetails->GetFormEditorID();
			}
			else
			{
				result << "";
			}

			result << "\"," << std::endl;
			;

			return result.str();
		}

		std::string GenerateFacialBoneMorphIntensity(NPCFileUtils::NPCFile npcFile)
		{
			float morphIntensity = npcFile.facialBoneMorphIntensity;

			std::stringstream result;

			result << "   \"FacialBoneMorphIntensity\" : ";
			result << std::setprecision(17) << (morphIntensity);
			result << "," << std::endl;

			return result.str();
		}

		std::string GenerateFacialBoneRegions(NPCFileUtils::NPCFile npcFile)
		{
			std::vector<NPCFileUtils::FacialBoneRegion> regions = npcFile.facialBoneRegionsA;
			std::stringstream result;

			if (regions.size() == 0)
			{
				regions = NPCFileDefaults::DefaultFacialBoneRegions(1);
			}

			result << "   \"FacialBoneRegionsA\" : [" << std::endl;

			for (std::uint32_t i = 0; i < regions.size(); i++)
			{
				NPCFileUtils::FacialBoneRegion region = regions[i];

				result << "      {" << std::endl;

				result << "         \"ID\" : ";
				result << std::setprecision(17) << (region.id);
				result << "," << std::endl;

				result << "         \"Position\" : {" << std::endl;
				result << "            \"x\" : ";
				result << std::setprecision(17) << (region.position.x);
				result << "," << std::endl;
				result << "            \"y\" : ";
				result << std::setprecision(17) << (region.position.y);
				result << "," << std::endl;
				result << "            \"z\" : ";
				result << std::setprecision(17) << (region.position.z) << std::endl;
				result << "         }," << std::endl;

				result << "         \"Rotation\" : {" << std::endl;
				result << "            \"x\" : ";
				result << std::setprecision(17) << (region.rotation.x);
				result << "," << std::endl;
				result << "            \"y\" : ";
				result << std::setprecision(17) << (region.rotation.y);
				result << "," << std::endl;
				result << "            \"z\" : ";
				result << std::setprecision(17) << (region.rotation.z) << std::endl;
				result << "         }," << std::endl;

				result << "         \"Scale\" : {" << std::endl;
				result << "            \"x\" : ";
				result << std::setprecision(17) << (region.scale.x);
				result << "," << std::endl;
				result << "            \"y\" : ";
				result << std::setprecision(17) << (region.scale.y);
				result << "," << std::endl;
				result << "            \"z\" : ";
				result << std::setprecision(17) << (region.scale.z) << std::endl;
				result << "         }" << std::endl;

				if (i == (regions.size() - 1))
				{
					result << "      }" << std::endl;
				}
				else
				{
					result << "      }," << std::endl;
				}
			}

			result << "   ]," << std::endl;

			return result.str();
		}

		std::string GenerateHairColor(TESNPC* npc, bool bHairColor)
		{
			std::stringstream result;

			if (bHairColor)
			{
				result << "   \"HairColor\" : \"";
				if (npc->headRelatedData->hairColor)
				{
					result << npc->headRelatedData->hairColor->GetFormEditorID();
				}
			}
			else
			{
				result << "   \"FacialHairColor\" : \"";
				if (npc->headRelatedData->facialHairColor)
				{
					result << npc->headRelatedData->facialHairColor->GetFormEditorID();
				}
			}
			result << "\"," << std::endl;

			return result.str();
		}

		std::string GenerateHeadPart(TESNPC* npc, BGSHeadPart::HeadPartType type)
		{
			std::span<BGSHeadPart*> headParts = npc->GetHeadParts();
			std::int8_t numHeadParts = npc->numHeadParts;

			const char* partName;

			switch (type)
			{
			case BGSHeadPart::HeadPartType::kEyebrows:
				partName = "HeadPartEyebrows";
				break;
			case BGSHeadPart::HeadPartType::kEyes:
				partName = "HeadPartEyes";
				break;
			case BGSHeadPart::HeadPartType::kFace:
				partName = "HeadPartFace";
				break;
			case BGSHeadPart::HeadPartType::kFacialHair:
				partName = "HeadPartFacial_Hair";
				break;
			case BGSHeadPart::HeadPartType::kHair:
				partName = "HeadPartHair";
				break;
			case BGSHeadPart::HeadPartType::kHeadRear:
				partName = "HeadPartHead_Rear";
				break;
			case BGSHeadPart::HeadPartType::kTeeth:
				partName = "HeadPartTeeth";
				break;
			}

			std::stringstream result;

			result << "   \"" << partName << "\" : \"";

			for (std::int8_t i = 0; i < numHeadParts; i++)
			{
				BGSHeadPart* currentHeadPart = headParts[i];

				if (currentHeadPart->type.get() == type)
				{
					result << currentHeadPart->formEditorID.c_str();
				}
			}

			result << "\"," << std::endl;

			return result.str();
		}

		std::string GenerateMorphSliders(NPCFileUtils::NPCFile npcFile)
		{
			std::vector<NPCFileUtils::MorphSlider> sliders = npcFile.morphSlidersA;

			std::stringstream result;

			if (sliders.size() == 0)
			{
				result = NPCFileDefaults::DefaultMorphSliders();
				return result.str();
			}

			result << "   \"MorphSlidersA\" : [" << std::endl;

			for (std::uint32_t i = 0; i < sliders.size(); i++)
			{
				NPCFileUtils::MorphSlider slider = sliders[i];

				result << "      {" << std::endl;
				result << "         \"ID\" : ";
				result << std::setprecision(17) << (slider.id);
				result << "," << std::endl;
				result << "         \"Value\" : ";
				result << std::setprecision(17) << (slider.value) << std::endl;

				if (i == (sliders.size() - 1))
				{
					result << "      }" << std::endl;
				}
				else
				{
					result << "      }," << std::endl;
				}
			}

			result << "   ]," << std::endl;

			return result.str();
		}

		std::string GenerateRace(TESNPC* npc)
		{
			TESRace* race = npc->GetFormRace();

			std::stringstream result;

			result << "   \"Race\" : \"";
			result << race->formEditorID.c_str();
			result << "\"," << std::endl;

			return result.str();
		}

		std::string GenerateSex(TESNPC* npc)
		{
			std::uint32_t sex = std::to_underlying(npc->GetSex());

			std::stringstream result;

			result << "   \"Sex\" : \"";

			if (sex == 1)
			{
				result << "Female";
			}
			else
			{
				result << "Male";
			}

			result << "\"," << std::endl;

			return result.str();
		}

		std::string GenerateTintEntries(NPCFileUtils::NPCFile npcFile)
		{
			std::vector<NPCFileUtils::TintEntry> tints = npcFile.tintEntriesA;

			std::stringstream result;

			if (tints.size() == 0)
			{
				tints = NPCFileDefaults::DefaultTintEntries();
			}

			result << "   \"TintEntriesA\" : [" << std::endl;

			for (int i = 0; i < tints.size(); i++)
			{
				NPCFileUtils::TintEntry tint = tints[i];

				result << "      {" << std::endl;

				if (tint.swatch > 0)
				{
					result << "         \"Color\" : ";
					result << std::to_string(tint.color);
					result << "," << std::endl;
				}

				result << "         \"ID\" : ";
				result << std::to_string(tint.id);
				result << "," << std::endl;

				result << "         \"Name\" : \"";
				result << tint.name;
				result << "\"," << std::endl;

				if (tint.swatch > 0)
				{
					result << "         \"Swatch\" : ";
					result << std::to_string(tint.swatch);
					result << "," << std::endl;
				}

				result << "         \"Value\" : ";
				result << std::setprecision(17) << (Shared::ConvertPercentageToFloat(tint.value)) << std::endl;

				if (i == (tints.size() - 1))
				{
					result << "      }" << std::endl;
				}
				else
				{
					result << "      }," << std::endl;
				}
			}

			result << "   ]," << std::endl;

			return result.str();
		}

		std::string GenerateWeight(TESNPC* npc)
		{
			std::stringstream result;

			result << "   \"Weight\" : {" << std::endl;
			result << "      \"x\" : ";
			result << std::setprecision(17) << (npc->morphWeight.x);
			result << "," << std::endl;
			result << "      \"y\" : ";
			result << std::setprecision(17) << (npc->morphWeight.y);
			result << "," << std::endl;
			result << "      \"z\" : ";
			result << std::setprecision(17) << (npc->morphWeight.z) << std::endl;
			result << "   }" << std::endl;

			return result.str();
		}
	}
}