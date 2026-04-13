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
					REX::DEBUG("Found tagged skills data.");
					std::uint32_t taggedSkillsCount = 0;
					a_intfc->ReadRecordData(&taggedSkillsCount, sizeof(uint32_t));

					for (std::uint32_t i = 0; i < taggedSkillsCount; i++)
					{
						std::uint32_t oldFormID = 0;
						std::uint32_t newFormID = 0;

						a_intfc->ReadRecordData(&oldFormID, sizeof(uint32_t));
						newFormID = a_intfc->ResolveFormID(oldFormID).value_or(0);

						if (!newFormID != 0)
						{
							taggedSkills.insert(newFormID);
						}
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
	}
}