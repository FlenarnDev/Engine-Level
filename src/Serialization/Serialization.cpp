#include "Serialization.h"

namespace Cascadia
{
	namespace Serialization
	{
		auto CASSerialization::GetSingleton() -> CASSerialization*
		{
			static CASSerialization singleton{};
			return std::addressof(singleton);
		}

		/* Level Up Menu Related */
		std::int32_t playerSkillPoints = 0;
		std::set<std::uint32_t> taggedSkills;
		bool playerReadyToLevelUp = false;

		// Cascadia-tracked permanent SPECIAL baseline - see Serialization.h for why this
		// exists separately from the engine's own GetPermanentActorValue.
		std::unordered_map<std::uint32_t, float> permanentSpecialBaseline;

		void CASSerialization::Serialize(const F4SE::SerializationInterface* a_intfc)
		{
			REX::DEBUG("Serialize save data.");
			std::int32_t test = 12;

			a_intfc->WriteRecord('USKP', SerializationVersion, &test, sizeof(int32_t));

			std::uint32_t taggedSkillCount = taggedSkills.size();
			
			a_intfc->OpenRecord('UTSK', SerializationVersion);
			a_intfc->WriteRecordData(&taggedSkillCount, sizeof(uint32_t));
			for (auto& form : taggedSkills)
			{
				std::uint32_t formID = form;
				a_intfc->WriteRecordData(&formID, sizeof(uint32_t));
			}
			REX::DEBUG("Saving tagged skills.");

			a_intfc->WriteRecord('BPRL', SerializationVersion, &playerReadyToLevelUp, sizeof(bool));

			std::uint32_t permanentSpecialCount = static_cast<std::uint32_t>(permanentSpecialBaseline.size());

			a_intfc->OpenRecord('PSPC', SerializationVersion);
			a_intfc->WriteRecordData(&permanentSpecialCount, sizeof(uint32_t));
			for (auto& [formID, value] : permanentSpecialBaseline)
			{
				a_intfc->WriteRecordData(&formID, sizeof(uint32_t));
				a_intfc->WriteRecordData(&value, sizeof(float));
			}
			REX::DEBUG("Saving permanent SPECIAL baseline.");
		}

		void CASSerialization::Deserialize(const F4SE::SerializationInterface* a_intfc)
		{
			REX::DEBUG("Deserialize save data.");
			std::uint32_t type;
			std::uint32_t version;
			std::uint32_t length;

			while (a_intfc->GetNextRecordInfo(type, version, length))
			{
				switch (type) {
				case 'BPRL':
					REX::DEBUG("Found PlayerReadyToLevelUp data.");
					a_intfc->ReadRecordData(&playerReadyToLevelUp, sizeof(bool));
					break;

				case 'USKP':
					REX::DEBUG("Found playerSkillPoints data.");
					a_intfc->ReadRecordData(&playerSkillPoints, sizeof(int32_t));
					REX::DEBUG("{}", playerSkillPoints);
					break;

				case 'UTSK':
				{
					REX::DEBUG("Found tagged skills data.");
					std::uint32_t taggedSkillsCount = 0;
					a_intfc->ReadRecordData(&taggedSkillsCount, sizeof(uint32_t));

					for (std::uint32_t i = 0; i < taggedSkillsCount; i++)
					{
						std::uint32_t oldFormID = 0;
						std::uint32_t newFormID = 0;

						a_intfc->ReadRecordData(&oldFormID, sizeof(uint32_t));
						newFormID = a_intfc->ResolveFormID(oldFormID).value_or(0);

						if (newFormID != 0)
						{
							taggedSkills.insert(newFormID);
						}
					}
					break;
				}

				case 'PSPC':
				{
					REX::DEBUG("Found permanent SPECIAL baseline data.");
					std::uint32_t permanentSpecialCount = 0;
					a_intfc->ReadRecordData(&permanentSpecialCount, sizeof(uint32_t));

					for (std::uint32_t i = 0; i < permanentSpecialCount; i++)
					{
						std::uint32_t oldFormID = 0;
						float value = 0.0f;

						a_intfc->ReadRecordData(&oldFormID, sizeof(uint32_t));
						a_intfc->ReadRecordData(&value, sizeof(float));

						std::uint32_t newFormID = a_intfc->ResolveFormID(oldFormID).value_or(0);
						if (newFormID != 0)
						{
							permanentSpecialBaseline[newFormID] = value;
						}
					}
					break;
				}
				}
			}
		}

		void CASSerialization::Revert()
		{
			REX::DEBUG("Revert serialization data.");
			playerSkillPoints = 0;
			playerReadyToLevelUp = false;
			taggedSkills.clear();
			permanentSpecialBaseline.clear();
		}

		void RevertCallback([[maybe_unused]] const F4SE::SerializationInterface* a_intfc)
		{
			CASSerialization::GetSingleton()->Revert();
		}

		void SaveCallback(const F4SE::SerializationInterface* a_intfc)
		{
			CASSerialization::GetSingleton()->Serialize(a_intfc);
		}

		void LoadCallback(const F4SE::SerializationInterface* a_intfc)
		{
			CASSerialization::GetSingleton()->Deserialize(a_intfc);
		}

		std::uint32_t GetSkillPoints()
		{
			return playerSkillPoints;
		}

		void SetSkillPoints(std::uint32_t setValue)
		{
			playerSkillPoints = setValue;
		}

		void ModSkillPoints(std::uint32_t modValue)
		{
			playerSkillPoints += modValue;
		}

		bool IsReadyToLevelUp()
		{
			return playerReadyToLevelUp;
		}

		void SetReadyToLevelUp(bool ready)
		{
			playerReadyToLevelUp = ready;
		}

		void SetSkillTagged(TESFormID skillFormid)
		{
			taggedSkills.insert(skillFormid);
		}

		void RemoveSkillTagged(TESFormID skillFormid)
		{
			taggedSkills.erase(skillFormid);
		}
		
		bool IsSkillTagged(TESFormID skillFormID)
		{
			return taggedSkills.find(skillFormID) != taggedSkills.end();
		}

		std::uint32_t GetSkillsTagged()
		{
			return taggedSkills.size();
		}

		bool HasPermanentSpecial(TESFormID specialFormID)
		{
			return permanentSpecialBaseline.find(specialFormID) != permanentSpecialBaseline.end();
		}

		float GetPermanentSpecial(TESFormID specialFormID)
		{
			auto it = permanentSpecialBaseline.find(specialFormID);
			return it != permanentSpecialBaseline.end() ? it->second : 0.0f;
		}

		void SetPermanentSpecial(TESFormID specialFormID, float value)
		{
			permanentSpecialBaseline[specialFormID] = value;
		}
	}
}