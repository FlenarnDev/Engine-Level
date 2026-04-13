#include "Shared/PerkHelpers.h"

namespace Cascadia
{
	namespace PerkHelpers
	{
		AvailablePerk GetAvailablePerk(BGSPerk* a_perk)
		{
			AvailablePerk result;
			
			result.level = 0;
			result.perk = nullptr;
			result.numRanks = a_perk->data.numRanks;

			BGSPerk* temporaryPerk;

			bool gotResult = false;
			bool learned = false;

			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

			temporaryPerk = a_perk;

			if (a_perk->data.numRanks > 1 && (a_perk->nextPerk == a_perk || a_perk->nextPerk == nullptr))
			{
				std::uint32_t currentPerkRank = playerCharacter->GetPerkRank(temporaryPerk);
				if (currentPerkRank < a_perk->data.numRanks)
				{
					result.level = currentPerkRank + 1;
					result.perk = a_perk;
					return result;
				}
				else
				{
					return result;
				}
			}
			else
			{
				std::uint32_t counter = 0;
				while (true)
				{
					temporaryPerk = temporaryPerk->nextPerk;
					counter++;
					if (temporaryPerk == a_perk || temporaryPerk == nullptr)
					{
						break;
					}
				}
				if (counter != a_perk->data.numRanks)
				{
					result.numRanks = counter;
				}
			}
			temporaryPerk = a_perk;

			for (std::uint32_t i = 0; i < result.numRanks; i++)
			{
				if (playerCharacter->GetPerkRank(temporaryPerk) == 0)
				{
					if (!gotResult)
					{
						gotResult = true;
						result.perk = temporaryPerk;
						result.level = i + 1;
					}
				}
				else
				{
					if (!learned)
					{
						learned = true;
					}
				}
				temporaryPerk = temporaryPerk->nextPerk;
			}
			return result;
		}

		PerkData GetPerkRequirements(BGSPerk* a_perk)
		{
			PerkData result;
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

			bool isAllowable = true;
			bool isHighLevel = a_perk->data.level > playerCharacter->GetLevel();
			bool isEligible = !isHighLevel;
			std::uint32_t filterFlag = 0;
			result.requiredLevel = a_perk->data.level;
			std::string requirementString = "";

			TESConditionItem* perkConditionItem = a_perk->perkConditions.head;

			bool lastFlag = false;
			
			ActorValueInfo* temporaryAVI;

			while (perkConditionItem != nullptr)
			{
				SCRIPT_OUTPUT scriptOutput = perkConditionItem->data.functionData.function.get();
				float compareValue = perkConditionItem->GetComparisonValue();
				ENUM_COMPARISON_CONDITION compareCondition = static_cast<ENUM_COMPARISON_CONDITION>(static_cast<std::int32_t>(perkConditionItem->data.condition));

				bool eligible = true;
				bool lessThan = false;
				bool lessThanEqual = false;

				switch (scriptOutput)
				{
				case SCRIPT_OUTPUT::kScript_GetPermanentValue:
				case SCRIPT_OUTPUT::kScript_GetBaseValue:
				case SCRIPT_OUTPUT::kScript_GetValue:
					temporaryAVI = static_cast<ActorValueInfo*>(perkConditionItem->data.functionData.param[0]);
					eligible = perkConditionItem->IsTrue(playerCharacter, nullptr);

					if (compareCondition == ENUM_COMPARISON_CONDITION::kLessThan)
					{
						lessThan = true;
					}
					else if (compareCondition == ENUM_COMPARISON_CONDITION::kLessThanEqual)
					{
						lessThanEqual = true;
					}

					filterFlag |= FilterFlags::kNonSpecial;

					if (!eligible)
					{
						requirementString += "<font color=\'#838383\'>";
					}

					switch (temporaryAVI->formID)
					{
					case 706:
						requirementString += "$CAS_STR";
						break;
					case 707:
						requirementString += "$CAS_PER";
						break;
					case 708:
						requirementString += "$CAS_END";
						break;
					case 709:
						requirementString += "$CAS_CHA";
						break;
					case 710:
						requirementString += "$CAS_INT";
						break;
					case 711:
						requirementString += "$CAS_AGI";
						break;
					case 712:
						requirementString += "$CAS_LCK";
						break;
					default:
						if (temporaryAVI->GetFullName() == "")
						{
							requirementString += temporaryAVI->GetFormEditorID();
						}
						else
						{
							requirementString += temporaryAVI->GetFullName();
						}
						break;
					}

					if (lessThan) {
						requirementString += " > " + std::to_string((std::uint32_t)compareValue);
					}
					else if (lessThanEqual) {
						requirementString += " > " + std::to_string((std::uint32_t)compareValue - 1);
					}
					else {
						requirementString += " " + std::to_string((std::uint32_t)compareValue);
					}
					break;
				case SCRIPT_OUTPUT::kScript_GetIsSex:
					isAllowable = perkConditionItem->IsTrue(playerCharacter, nullptr);
					break;
				default:
					break;
				}

				lastFlag = (perkConditionItem->data.compareOr == 0);

				perkConditionItem = perkConditionItem->next;

				if (perkConditionItem)
				{
					SCRIPT_OUTPUT scriptOutputNext = perkConditionItem->data.functionData.function.get();

					if (scriptOutputNext == SCRIPT_OUTPUT::kScript_GetPermanentValue || scriptOutputNext == SCRIPT_OUTPUT::kScript_GetBaseValue || scriptOutputNext == SCRIPT_OUTPUT::kScript_GetValue) {
						if (scriptOutput == SCRIPT_OUTPUT::kScript_GetIsSex)
						{
							requirementString += "";
							if (!eligible)
							{
								requirementString += "</font>";
							}
						}
						else if (lastFlag)
						{
							requirementString += ", ";
							if (!eligible)
							{
								requirementString += "</font>";
							}
						}
						else
						{
							requirementString += " $CAS_or ";
						}
					}
				}
			}

			result.isEligible = isEligible && a_perk->perkConditions.IsTrue(playerCharacter, playerCharacter);
			result.isAllowable = isAllowable;
			result.isHighLevel = isHighLevel;
			if (filterFlag == 0) filterFlag |= FilterFlags::kOther;
			result.filterFlag = filterFlag | (result.isEligible ? 1 : 2);
			result.requirementsString = requirementString;
			result.SWFPath = a_perk->swfFile.c_str();
			result.isTagged = false;

			return result;
		}
	}
}