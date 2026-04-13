#pragma once

using namespace RE;

namespace Cascadia
{
	namespace NPCFileUtils
	{
		struct BodyMorphRegions
		{
			float head;
			float upperTorso;
			float arms;
			float lowerTorso;
			float legs;
		};

		struct FacialBoneRegion
		{
			std::uint32_t id;
			NiPoint3 position;
			NiPoint3 rotation;
			NiPoint3 scale;
		};

		struct MorphSlider
		{
			std::uint32_t id;
			float value;
		};

		struct TintEntry
		{
			std::uint32_t color;
			std::uint32_t id;
			std::string name;
			std::uint32_t swatch;
			std::uint8_t value;                        // Divide by 100 for NPCFile{};
			BGSCharacterTint::Template::Entry* entry;  // Only use if for randomization.
		};

		struct TintIDSet
		{
			std::uint16_t id;
			std::string name;
			std::string category;
		};

		struct NPCFile
		{
			BodyMorphRegions bodyMorphRegionsA;
			float facialBoneMorphIntensity;
			std::vector<FacialBoneRegion> facialBoneRegionsA;
			std::vector<MorphSlider> morphSlidersA;
			std::vector<TintEntry> tintEntriesA;
		};

		BodyMorphRegions GetBodyMorphRegionsFromNPC(TESNPC* npc);
		std::vector<MorphSlider> GetMorphSliderValuesFromNPC(TESNPC* npc);
		std::vector<FacialBoneRegion> GetFacialBoneRegionsFromNPC(TESNPC* npc);
		std::vector<TintIDSet> GetTintIDsFromRace(TESRace* race, std::uint32_t gender);
		std::vector<TintEntry> GetTintEntriesFromNPC(TESNPC* npc, std::uint32_t gender);
	}

	namespace NPCFileDefaults
	{
		NPCFileUtils::BodyMorphRegions DefaultBodyMorphRegions(std::uint32_t gender);
		std::vector<NPCFileUtils::FacialBoneRegion> DefaultFacialBoneRegions(std::uint32_t gender);
		std::stringstream DefaultMorphSliders();
		std::vector<NPCFileUtils::TintEntry> DefaultTintEntries();
	}

	namespace NPCFileExport
	{
		void ExportNPCFile(NPCFileUtils::NPCFile npcFile, const char* fileName);
		std::string GenerateBodyMorphRegionsOutput(NPCFileUtils::NPCFile npcFile);
		std::string GenerateComplexionOutput(TESNPC* npc);
		std::string GenerateFacialBoneMorphIntensity(NPCFileUtils::NPCFile npcFile);
		std::string GenerateFacialBoneRegions(NPCFileUtils::NPCFile npcFile);
		std::string GenerateHairColor(TESNPC* npc, bool bHairColor);
		std::string GenerateHeadPart(TESNPC* npc, BGSHeadPart::HeadPartType type);
		std::string GenerateMorphSliders(NPCFileUtils::NPCFile npcFile);
		std::string GenerateRace(TESNPC* npc);
		std::string GenerateSex(TESNPC* npc);
		std::string GenerateTintEntries(NPCFileUtils::NPCFile npcFile);
		std::string GenerateWeight(TESNPC* npc);
	}
}