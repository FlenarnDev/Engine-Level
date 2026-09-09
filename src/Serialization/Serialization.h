#pragma once

#include "F4SE/Interfaces.h"
#include "Shared/SharedFunctions.h"

namespace Cascadia
{
	namespace Serialization
	{
		constexpr std::int32_t ID = 'CAS';

		class CASSerialization
		{
		public:
			~CASSerialization() = default;

			static auto GetSingleton() -> CASSerialization*;

			void Serialize(const F4SE::SerializationInterface* a_intfc);

			void Deserialize(const F4SE::SerializationInterface* a_intfc);

			void Revert();
		private:
			static constexpr std::uint32_t SerializationVersion = 1;
			static constexpr std::uint32_t SerializationType = 'CAS';
		};

		void RevertCallback(const F4SE::SerializationInterface* a_intfc);
		void SaveCallback(const F4SE::SerializationInterface* a_intfc);
		void LoadCallback(const F4SE::SerializationInterface* a_intfc);

		// Skill Point functions
		std::uint32_t GetSkillPoints();
		void SetSkillPoints(std::uint32_t setValue);
		void ModSkillPoints(std::uint32_t modValue);

		//	Level Up Functions
		bool IsReadyToLevelUp();
		void SetReadyToLevelUp(bool bReady);

		// Tagged skills
		void SetSkillTagged(TESFormID skillFormid);
		void RemoveSkillTagged(TESFormID skillFormid);
		bool IsSkillTagged(TESFormID skillFormID);
		std::uint32_t GetSkillsTagged();

		// Cascadia-tracked permanent SPECIAL baseline. This exists because the engine's
		// own permanent/temporary/damage modifier classification for a SPECIAL AV can't
		// be trusted to match what a magic effect's author intended (verified: an effect
		// meant to be a temporary buff can land its magnitude in the engine's permanent
		// modifier slot) - so GetPermanentActorValue on a SPECIAL isn't safe to use as a
		// "this SPECIAL genuinely wasn't temporarily buffed" reference. This baseline is
		// only ever moved by Cascadia's own code (see Skills::ModPermanentSpecial),
		// independent of whatever slot external effects use.
		bool HasPermanentSpecial(TESFormID specialFormID);
		float GetPermanentSpecial(TESFormID specialFormID);
		void SetPermanentSpecial(TESFormID specialFormID, float value);
	}
}

