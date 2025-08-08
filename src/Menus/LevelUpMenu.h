#pragma once

#include "Serialization/Serialization.h"
#include "Shared/PerkHelpers.h"
#include "Shared/SharedFunctions.h"
#include "Systems/Skills.h"

namespace RE
{
	namespace Cascadia
	{
		namespace LevelUpMenu
		{
			float perkPoints;

			std::uint32_t menuModeType = 0;
			std::uint32_t tagPointsValue = 0;
			
			bool allowRetag = false;

			namespace InitialValues
			{
				std::uint32_t barter;
				std::uint32_t energyWeapons;
				std::uint32_t explosives;
				std::uint32_t guns;
				std::uint32_t lockpick;
				std::uint32_t medicine;
				std::uint32_t meleeWeapons;
				std::uint32_t repair;
				std::uint32_t science;
				std::uint32_t sneak;
				std::uint32_t speech;
				std::uint32_t survival;
				std::uint32_t unarmed;
			}

			namespace TempModValues
			{
				std::uint32_t barter;
				std::uint32_t energyWeapons;
				std::uint32_t explosives;
				std::uint32_t guns;
				std::uint32_t lockpick;
				std::uint32_t medicine;
				std::uint32_t meleeWeapons;
				std::uint32_t repair;
				std::uint32_t science;
				std::uint32_t sneak;
				std::uint32_t speech;
				std::uint32_t survival;
				std::uint32_t unarmed;
			}

			void InitialiseValues()
			{
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

				InitialValues::barter = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Barter);
				InitialValues::energyWeapons = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.EnergyWeapons);
				InitialValues::explosives = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Explosives);
				InitialValues::guns = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Guns);
				InitialValues::lockpick = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Lockpick);
				InitialValues::medicine = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Medicine);
				InitialValues::meleeWeapons = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.MeleeWeapons);
				InitialValues::repair = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Repair);
				InitialValues::science = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Science);
				InitialValues::sneak = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Sneak);
				InitialValues::speech = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Speech);
				InitialValues::survival = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Survival);
				InitialValues::unarmed = playerCharacter->GetPermanentActorValue(*Skills::CascadiaActorValues.Unarmed);
			}

			enum SkillArray
			{
				Barter = 0,
				EnergyWeapons = 1,
				Explosives = 2,
				Guns = 3,
				Lockpick = 4,
				Medicine = 5,
				MeleeWeapons = 6,
				Repair = 7,
				Science = 8,
				Sneak = 9,
				Speech = 10,
				Survival = 11,
				Unarmed = 12
			};

			enum LevelTypes
			{
				kLevelUp = 0,			// Normal level up
				kTagSkills = 1,			// Tag skills mode
				kSpecialRespec = 2,		// Special respec mode
				kIntenseTraining = 3	// 'Intense Training' perk
			};

			enum PerkTypes
			{
				kDefault = 0,
				kStrength = 1,
				kPerception = 2,
				kEndurance = 3,
				kCharisma = 4,
				kIntelligence = 5,
				kAgility = 6,
				kLuck = 7,
				kIntenseTrainingPerk = 8,
				kCancel = 9
			};

			float GetInitialActorValue(ActorValueInfo* a_skill)
			{
				if (a_skill == Skills::CascadiaActorValues.Barter)
				{
					return InitialValues::barter;
				}
				else if (a_skill == Skills::CascadiaActorValues.EnergyWeapons)
				{
					return InitialValues::energyWeapons;
				}
				else if (a_skill == Skills::CascadiaActorValues.Explosives)
				{
					return InitialValues::explosives;
				}
				else if (a_skill == Skills::CascadiaActorValues.Guns)
				{
					return InitialValues::guns;
				}
				else if (a_skill == Skills::CascadiaActorValues.Lockpick)
				{
					return InitialValues::lockpick;
				}
				else if (a_skill == Skills::CascadiaActorValues.Medicine)
				{
					return InitialValues::medicine;
				}
				else if (a_skill == Skills::CascadiaActorValues.MeleeWeapons)
				{
					return InitialValues::meleeWeapons;
				}
				else if (a_skill == Skills::CascadiaActorValues.Repair)
				{
					return InitialValues::repair;
				}
				else if (a_skill == Skills::CascadiaActorValues.Science)
				{
					return InitialValues::science;
				}
				else if (a_skill == Skills::CascadiaActorValues.Sneak)
				{
					return InitialValues::sneak;
				}
				else if (a_skill == Skills::CascadiaActorValues.Speech)
				{
					return InitialValues::speech;
				}
				else if (a_skill == Skills::CascadiaActorValues.Survival)
				{
					return InitialValues::survival;
				}
				else if (a_skill == Skills::CascadiaActorValues.Unarmed)
				{
					return InitialValues::unarmed;
				}
				else
				{
					return -1;
				}
			}

			void PopulateSkillEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, ActorValueInfo* a_skill)
			{
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
				float buffedValue = playerCharacter->GetActorValue(*a_skill);

				Scaleform::GFx::Value skillEntry;
				a_movieRoot->CreateObject(&skillEntry);

				BSStringT<char> description;
				a_skill->GetDescription(description);

				skillEntry.SetMember("text", a_skill->fullName.c_str());
				skillEntry.SetMember("editorID", a_skill->GetFormEditorID());
				skillEntry.SetMember("description", description.c_str());
				skillEntry.SetMember("formid", a_skill->formID);

				float baseValue = GetInitialActorValue(a_skill);
				float value = playerCharacter->GetPermanentActorValue(*a_skill);
				skillEntry.SetMember("value", (std::uint32_t)baseValue);
				skillEntry.SetMember("baseValue", (std::uint32_t)baseValue);
				skillEntry.SetMember("buffedValue", (std::uint32_t)value);
				
				skillEntry.SetMember("alreadyTagged", Serialization::IsSkillTagged(a_skill->formID));
				skillEntry.SetMember("tagged", Serialization::IsSkillTagged(a_skill->formID));
				a_destination->PushBack(skillEntry);
			}

			bool ProcessSkillsList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				Scaleform::GFx::Value argumentsArray[3];
				a_movieRoot->CreateArray(&argumentsArray[0]);

				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Barter);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.EnergyWeapons);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Explosives);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Guns);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Lockpick);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Medicine);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.MeleeWeapons);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Repair);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Science);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Sneak);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Speech);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Survival);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Unarmed);

				argumentsArray[1] = Serialization::GetSkillPoints();
				argumentsArray[2] = PlayerCharacter::GetSingleton()->GetLevel();
				a_movieRoot->Invoke("root.Menu_mc.onLevelUpStart", nullptr, argumentsArray, 3);

				return true;
			}

			bool ProcessTagSkillsList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, std::uint32_t a_tagSkills, bool a_retag)
			{
				Scaleform::GFx::Value argumentsArray[3];
				a_movieRoot->CreateArray(&argumentsArray[0]);

				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Barter);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.EnergyWeapons);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Explosives);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Guns);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Lockpick);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Medicine);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.MeleeWeapons);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Repair);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Science);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Sneak);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Speech);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Survival);
				PopulateSkillEntry(&argumentsArray[0], a_movieRoot, Skills::CascadiaActorValues.Unarmed);

				argumentsArray[1] = a_tagSkills;
				argumentsArray[2] = a_retag;

				a_movieRoot->Invoke("root.Menu_mc.onTagSkillsStart", nullptr, argumentsArray, 3);

				return true;

			}

			void PopulateSpecialEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, ActorValueInfo* a_skill)
			{
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

				Scaleform::GFx::Value skillEntry;
				a_movieRoot->CreateObject(&skillEntry);

				BSStringT<char> description;
				a_skill->GetDescription(description);

				skillEntry.SetMember("text", a_skill->fullName.c_str());
				skillEntry.SetMember("editorID", a_skill->GetFormEditorID());
				skillEntry.SetMember("description", description.c_str());
				skillEntry.SetMember("formid", a_skill->formID);

				float baseAVValue = playerCharacter->GetBaseActorValue(*a_skill);
				float avValue = playerCharacter->GetActorValue(*a_skill);

				skillEntry.SetMember("value", (std::uint32_t)baseAVValue);
				skillEntry.SetMember("baseValue", (std::uint32_t)baseAVValue);
				skillEntry.SetMember("buffedValue", (std::uint32_t)avValue);

				a_destination->PushBack(skillEntry);
			}

			bool ProcessSpecialList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				Scaleform::GFx::Value argumentsArray[1];
				a_movieRoot->CreateArray(&argumentsArray[0]);

				PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, Skills::VanillaActorValues.Strength);
				PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, Skills::VanillaActorValues.Perception);
				PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, Skills::VanillaActorValues.Endurance);
				PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, Skills::VanillaActorValues.Charisma);
				PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, Skills::VanillaActorValues.Intelligence);
				PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, Skills::VanillaActorValues.Agility);
				PopulateSpecialEntry(&argumentsArray[0], a_movieRoot, Skills::VanillaActorValues.Luck);

				switch (menuModeType) {
				case kSpecialRespec:
					a_movieRoot->Invoke("root.Menu_mc.onSpecialRespecStart", nullptr, argumentsArray, 1);
					break;
				case kIntenseTraining:
					a_movieRoot->Invoke("root.Menu_mc.onIntenseTrainingStart", nullptr, argumentsArray, 1);
					break;
				default:
					break;
				}

				return true;
			}

			void PopulatePerkEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, BGSPerk* a_perk, bool a_eligibleOnly)
			{
				PerkHelpers::AvailablePerk currentPerk = PerkHelpers::GetAvailablePerk(a_perk);

				if (currentPerk.perk != nullptr)
				{
					REX::DEBUG("Processing perk: {}", currentPerk.perk->GetFormEditorID());

					PerkHelpers::PerkData temporaryPerkData = PerkHelpers::GetPerkRequirements(currentPerk.perk);

					Scaleform::GFx::Value perkEntry;
					a_movieRoot->CreateObject(&perkEntry);
					perkEntry.SetMember("text", currentPerk.perk->fullName.c_str());
					perkEntry.SetMember("reqs", temporaryPerkData.requirementsString.c_str());
					perkEntry.SetMember("filterFlag", temporaryPerkData.filterFlag);
					perkEntry.SetMember("iselig", temporaryPerkData.isEligible);
					perkEntry.SetMember("isHighLevel", temporaryPerkData.isHighLevel);
					perkEntry.SetMember("reqLevel", temporaryPerkData.requiredLevel);
					perkEntry.SetMember("qname", a_perk->formID);
					perkEntry.SetMember("formid", currentPerk.perk->formID);
					perkEntry.SetMember("level", currentPerk.level);
					perkEntry.SetMember("SWFPath", temporaryPerkData.SWFPath.c_str());
					perkEntry.SetMember("numranks", currentPerk.numRanks);
					perkEntry.SetMember("type", PerkTypes::kDefault);
					perkEntry.SetMember("ranksinfo", "");

					BSStringT<char> description;
					currentPerk.perk->GetDescription(description);

					perkEntry.SetMember("description", description.c_str());
					perkEntry.SetMember("isTagged", temporaryPerkData.isTagged);

					if (a_eligibleOnly)
					{
						if (temporaryPerkData.isEligible && temporaryPerkData.isAllowable)
						{
							a_destination->PushBack(perkEntry);
						}
					}
					else
					{
						if (!temporaryPerkData.isEligible && temporaryPerkData.isAllowable)
						{
							a_destination->PushBack(perkEntry);
						}
					}
				}
			}

			bool ProcessPerkList(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				Scaleform::GFx::Value argumentsArray[2];
				a_movieRoot->CreateArray(&argumentsArray[0]);

				// Eligible perks iterated through first to sort at top.
				for (std::uint32_t p1 = 0; p1 < Skills::CascadiaPerksLevelUp.size(); p1++)
				{
					BGSPerk* perk = Skills::CascadiaPerksLevelUp.at(p1);
					PopulatePerkEntry(&argumentsArray[0], a_movieRoot, perk, true);
				}

				// Ineligible perks iterated through last to sort at bottom.
				for (std::uint32_t p2 = 0; p2 < Skills::CascadiaPerksLevelUp.size(); p2++)
				{
					BGSPerk* perk = Skills::CascadiaPerksLevelUp.at(p2);
					PopulatePerkEntry(&argumentsArray[0], a_movieRoot, perk, false);
				}

				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

				argumentsArray[1] = playerCharacter->perkCount;

				a_movieRoot->Invoke("root.Menu_mc.onSwitchToPerks", nullptr, argumentsArray, 2);
				return true;
			}

			std::uint32_t GetSkillArrayIndexByEditorID(const char* editorID) 
			{
				// Create a map of editor IDs to skill array indices.
				static const std::unordered_map<std::string, std::uint32_t> editorIDMap = {
					{"CAS_Barter", SkillArray::Barter},
					{"CAS_EnergyWeapons", SkillArray::EnergyWeapons},
					{"CAS_Explosives", SkillArray::Explosives},
					{"CAS_Guns", SkillArray::Guns},
					{"CAS_Lockpick", SkillArray::Lockpick},
					{"CAS_Medicine", SkillArray::Medicine},
					{"CAS_MeleeWeapons", SkillArray::MeleeWeapons},
					{"CAS_Repair", SkillArray::Repair},
					{"CAS_Science", SkillArray::Science},
					{"CAS_Speech", SkillArray::Speech},
					{"CAS_Surival", SkillArray::Survival},
					{"CAS_Unarmed", SkillArray::Unarmed}
				};

				// Find the editorID in the map.
				auto it = editorIDMap.find(editorID);
				if (it != editorIDMap.end()) {
				    return it->second;
				}

				// Return -1 if the editor ID is not found.
				return static_cast<std::uint32_t>(-1);
			}

			float GetSkillPointsToAdd()
			{
				// Fallout 3 formula is 10 + base intelligence
				// Extra 3 points if the player has "Educated" perk
				// Using a global in 'FalloutCascaida.esm' we can adjust the base value.

				TESGlobal* baseSkillPointsGlobal = TESDataHandler::GetSingleton()->LookupForm<TESGlobal>(0x1F9DFD, "FalloutCascadia.esm");
				float skillPointsValue = 0;

				if (baseSkillPointsGlobal)
				{
					skillPointsValue = baseSkillPointsGlobal->value;
				}
				else
				{
					skillPointsValue = 10;
				}

				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

				float playerIntelligence = playerCharacter->GetBaseActorValue(*Skills::VanillaActorValues.Intelligence);

				skillPointsValue = (skillPointsValue + playerIntelligence);

				BGSPerk* educatedPerk = TESDataHandler::GetSingleton()->LookupForm<BGSPerk>(0x0F399E, "FalloutCascadia.esm");

				if (educatedPerk)
				{
					if (playerCharacter->GetPerkRank(educatedPerk) != 0)
					{
						skillPointsValue = (skillPointsValue + 3);
					}
				}

				return skillPointsValue;
			}

			bool CanLevelUpMenuBeShown()
			{
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
				if (playerCharacter->IsInCombat())
				{
					return false;
				}

				if (InMenuMode())
				{
					return false;
				}

				if (IsPlayerInDialogue())
				{
					return false;
				}

				if (IsXPMetervisible())
				{
					return false;
				}

				return true;
			}

			// Waits for level up to be ready and then shows menu
			void WaitForLevelUpReady()
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				if (!CanLevelUpMenuBeShown())
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					std::async(std::launch::async, WaitForLevelUpReady);
					return;
				}
				else
				{
					UIMessageQueue* uiMessageQueue = UIMessageQueue::GetSingleton();
					if (UI* ui = UI::GetSingleton())
					{
						if (ui->menuMap.contains("CASLevelUpMenu"))
						{
							uiMessageQueue->AddMessage("CASLevelUpMenu", UI_MESSAGE_TYPE::kShow);
						}
					}
				}
			}

			void ModPerkCount(std::int8_t a_count)
			{
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
				std::int8_t result = playerCharacter->perkCount;

				if (a_count < -result)
				{
					result = 0;
				}
				else
				{
					result = result + a_count;
					if (result > 255)
					{
						result = 255;
					}
				}
				playerCharacter->SetPerkCount(result);
			}

			void SetSkillTagged(ActorValueInfo* a_skill, bool a_tag)
			{
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
				auto formID = a_skill->formID;

				if (a_tag)
				{
					if (!Serialization::IsSkillTagged(formID))
					{
						Serialization::SetSkillTagged(formID);
						playerCharacter->ModActorValue(ACTOR_VALUE_MODIFIER::kPermanent, *a_skill, 15);
					}
				}
				else
				{
					if (Serialization::IsSkillTagged(formID))
					{
						Serialization::RemoveSkillTagged(formID);
						playerCharacter->ModActorValue(ACTOR_VALUE_MODIFIER::kPermanent, *a_skill, -15);
					}
				}
			}

			// Called from 'LevelIncrease::Event'
			void HandleLevelUp()
			{
				float pointsToAdd = GetSkillPointsToAdd();

				Serialization::ModSkillPoints(pointsToAdd);

				// TODO - UpdateLevelUpFormsFromGame

				Serialization::SetReadyToLevelUp(true);

				std::thread LevelUpWait(WaitForLevelUpReady);
				LevelUpWait.detach();
			}

			void ModSkillByName(std::string a_skillName, std::uint32_t a_value, std::uint32_t a_baseValue)
			{
				std::string skillNameString = a_skillName;
				skillNameString.erase(std::remove_if(skillNameString.begin(), skillNameString.end(), isspace), skillNameString.end());

				ActorValueInfo* skill = Skills::GetSkillByName(skillNameString);
				std::string skillEditorID = skill->GetFormEditorID();

				std::uint32_t modValue = (a_value - a_baseValue);

				if (skillEditorID == "CAS_Barter")
				{
					TempModValues::barter = modValue;
				}
				else if (skillEditorID == "CAS_EnergyWeapons")
				{
					TempModValues::energyWeapons = modValue;
				}
				else if (skillEditorID == "CAS_Explosives")
				{
					TempModValues::explosives = modValue;
				}
				else if (skillEditorID == "CAS_Guns")
				{
					TempModValues::guns = modValue;
				}
				else if (skillEditorID == "CAS_Lockpick")
				{
					TempModValues::lockpick = modValue;
				}
				else if (skillEditorID == "CAS_Medicine")
				{
					TempModValues::medicine = modValue;
				}
				else if (skillEditorID == "CAS_MeleeWeapons")
				{
					TempModValues::meleeWeapons = modValue;
				}
				else if (skillEditorID == "CAS_Repair")
				{
					TempModValues::repair = modValue;
				}
				else if (skillEditorID == "CAS_Science")
				{
					TempModValues::science = modValue;
				}
				else if (skillEditorID == "CAS_Sneak")
				{
					TempModValues::sneak = modValue;
				}
				else if (skillEditorID == "CAS_Speech")
				{
					TempModValues::speech = modValue;
				}
				else if (skillEditorID == "CAS_Survival")
				{
					TempModValues::survival = modValue;
				}
				else if (skillEditorID == "CAS_Unarmed")
				{
					TempModValues::unarmed = modValue;
				}
				
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
				playerCharacter->ModActorValue(ACTOR_VALUE_MODIFIER::kPermanent, *skill, modValue);
			}

			// Check if player saved in the middle of a level up.
			void CheckForLevelUp()
			{
				if (Serialization::IsReadyToLevelUp())
				{
					std::thread LevelUpWait(WaitForLevelUpReady);
					LevelUpWait.detach();
				}
			}

			// Send initial data to AS3 for Level Up.
			void OnLevelUpStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				ProcessSkillsList(a_movieRoot);
			}

			// Send initial data to AS3 for 'Tag Skills'.
			void OnTagSkillsStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				ProcessTagSkillsList(a_movieRoot, tagPointsValue, allowRetag);
			}

			// Send initial data to AS3 for S.P.E.C.I.A.L respec - might be cut.
			void OnSpecialRespecStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				ProcessSpecialList(a_movieRoot);
			}

			// Send initial data to AS3 for  Intense Training.
			void OnIntenseTrainingStart(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				ProcessSpecialList(a_movieRoot);
			}

			void HandleLevelUpMenuOpen(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				switch (menuModeType)
				{
				case kLevelUp:
					OnLevelUpStart(a_movieRoot);
					break;

				case kTagSkills:
					OnTagSkillsStart(a_movieRoot);
					break;

				case kSpecialRespec:
					OnSpecialRespecStart(a_movieRoot);
					break;

				case kIntenseTraining:
					OnIntenseTrainingStart(a_movieRoot);
					break;

				default:
					break;
				}
			}

			void CompleteLevelUp()
			{
				switch (menuModeType)
				{
				case kLevelUp:
					Serialization::SetReadyToLevelUp(false);
					Serialization::SetSkillPoints(0);
					REX::DEBUG("'CompleteLevelUp' - 'kLevelUp'");
					break;

				case kTagSkills:
					tagPointsValue = 0;
					allowRetag = false;
					REX::DEBUG("'CompleteLevelUp' - 'kTagSkills'");
					break;

				case kSpecialRespec:
					REX::DEBUG("'CompleteLevelUp' - 'kSpecialRespec'");
					break;

				case kIntenseTraining:
					REX::DEBUG("'CompleteLevelUp' - 'kIntenseTraining'");
					break;
				default:
					break;
				}

				menuModeType = 0;
			}

			class OpenMenu : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'OpenMenu' called from AS3.");
					LevelUpMenu::InitialiseValues();
					// TODO - RegisterForInput
					LevelUpMenu::HandleLevelUpMenuOpen(a_params.movie->asMovieRoot);
				}
			};

			class CloseMenu : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'CloseMenu' called from AS3.");
					
					UIMessageQueue* uiMessageQueue = UIMessageQueue::GetSingleton();
					if (UI* ui = UI::GetSingleton())
					{
						if (ui->menuMap.contains("CASLevelUpMenu"))
						{
							uiMessageQueue->AddMessage("CASLevelUpMenu", UI_MESSAGE_TYPE::kHide);
						}
					}
					CompleteLevelUp();
				}
			};

			class SetSkills : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'SetSkills' called from AS3.");

					std::uint32_t skillsCount = a_params.args[0].GetArraySize();
					Scaleform::GFx::Value arrayElement, skillName, skillValue, skillBaseValue;

					for (std::uint32_t i = 0; i < skillsCount; i++)
					{
						a_params.args[0].GetElement(i, &arrayElement);
						arrayElement.GetMember("sName", &skillName);
						arrayElement.GetMember("iValue", &skillValue);
						arrayElement.GetMember("iBaseValue", &skillBaseValue);
						
						auto skillValueNormal = 0;
						if (skillValue.IsNumber())
						{
							skillValueNormal = skillValue.GetNumber();
						}
						else if (skillValue.IsUInt())
						{
							skillValueNormal = skillValue.GetUInt();
						}

						auto skillBaseValueNormal = 0;
						if (skillBaseValue.IsNumber())
						{
							skillBaseValueNormal = skillBaseValue.GetNumber();
						}
						else if (skillBaseValue.IsUInt())
						{
							skillBaseValueNormal = skillBaseValue.GetUInt();
						}

						ModSkillByName(skillName.GetString(), skillValueNormal, skillBaseValueNormal);
					}

					ProcessPerkList(a_params.movie->asMovieRoot);
				}
			};

			class ResetSkills : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'ResetSkills' called from AS3.");
					ProcessSkillsList(a_params.movie->asMovieRoot);
				}
			};

			class ResetTagSkills : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'ResetTagSkills' called from AS3.");
					OnTagSkillsStart(a_params.movie->asMovieRoot);
				}
			};

			class BackToSkills : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'BackToSkills' called from AS3.");
				}
			};

			class UpdatePerkMenu : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'UpdatePerksMenu' called from AS3.");
					ProcessPerkList(a_params.movie->asMovieRoot);
				}
			};

			class AddPerks : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'AddPerks' called from AS3.");
					std::uint32_t perkCount = a_params.args[0].GetArraySize();
					Scaleform::GFx::Value arrayElement, perkFormID;

					PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

					for (std::uint32_t i = 0; i < perkCount; i++) {
						a_params.args[0].GetElement(i, &arrayElement);
						arrayElement.GetMember("iFormID", &perkFormID);				
						TESForm* a_perkForm = TESForm::GetFormByNumericID(perkFormID.GetUInt());
						BGSPerk* a_perk = static_cast<BGSPerk*>(a_perkForm);
						playerCharacter->AddPerk(a_perk);
						ModPerkCount(-1);
					}
				}
			};

			class TagSkills : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'TagSkills' called from AS3.");
					std::uint32_t skillsCount = a_params.args[0].GetArraySize();
					Scaleform::GFx::Value arrayElement, skillName, skillTagged;

					for (std::uint32_t i = 0; i < skillsCount; i++) {
						a_params.args[0].GetElement(i, &arrayElement);
						arrayElement.GetMember("sName", &skillName);
						arrayElement.GetMember("isTagged", &skillTagged);
						SetSkillTagged(Skills::GetSkillByName(skillName.GetString()), skillTagged.GetBoolean());
					}
				}
			};

			class PlayUISound : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'PlayUISound' called from AS3.");
					if (a_params.argCount < 1) return;
					if (!a_params.args[0].IsString()) return;

					auto uiSound = a_params.args[0].GetString();

					UIUtils::PlayMenuSound(uiSound);
				}
			};

			class LearnSpecial : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'LearnSpecial' called from AS3.");
					std::uint32_t specialCount = a_params.args[0].GetArraySize();
					Scaleform::GFx::Value arrayElement, specialFormID, specialValue;

					ActorValueInfo* special;

					for (int i = 0; i < specialCount; i++) {
						a_params.args[0].GetElement(i, &arrayElement);
						arrayElement.GetMember("formID", &specialFormID);
						arrayElement.GetMember("iValue", &specialValue);
					}
				}
			};

			class ResetSpecial : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'ResetSpecial' called from AS3.");
					LevelUpMenu::HandleLevelUpMenuOpen(a_params.movie->asMovieRoot);
				}
			};

			class PlaySkillSound : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'PlaySkillSound' called from AS3.");
					// TODO
				}
			};

			class PlayPerkSound : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					REX::DEBUG("'PlayPerkSound' called from AS3.");
					// TODO
				}
			};

			class CASLevelUpMenu :
				public GameMenuBase
			{
			public:
				static IMenu* CreateCASLevelUpMenu(const UIMessage&)
				{
					return new CASLevelUpMenu();
				}

				const char* sMenuName = "CASLevelUpMenu";

				void ProcessUserEventExternal(const char* controlName, bool isDown, std::uint32_t deviceType, BS_BUTTON_CODE buttonCode)
				{
					UI* ui = UI::GetSingleton();

					if (ui->GetMenuOpen(sMenuName))
					{
						IMenu* menu = ui->GetMenu(sMenuName).get();
						Scaleform::GFx::ASMovieRootBase* movieRoot = menu->uiMovie->asMovieRoot.get();
						Scaleform::GFx::Value arguments[4];
						arguments[0] = controlName;
						arguments[1] = isDown;
						arguments[2] = deviceType;
						arguments[3] = static_cast<std::int32_t>(buttonCode);
						movieRoot->Invoke("root.Menu_mc.ProcessUserEventExternal", nullptr, arguments, 4);
					}
				}

				void OnButtonEvent(const ButtonEvent* a_event) override 
				{
					ProcessUserEventExternal(a_event->QUserEvent().c_str(), a_event->QHeldDown(), a_event->device.underlying(), a_event->GetBSButtonCode());
				}

				CASLevelUpMenu()
				{
					menuFlags.set(
						UI_MENU_FLAGS::kPausesGame,
						UI_MENU_FLAGS::kUsesCursor,
						UI_MENU_FLAGS::kTopmostRenderedMenu,
						UI_MENU_FLAGS::kUpdateUsesCursor,
						UI_MENU_FLAGS::kUsesBlurredBackground
					);
					menuHUDMode = "SpecialMode";
					depthPriority = UI_DEPTH_PRIORITY::kTerminal;
					const auto ScaleformManager = BSScaleformManager::GetSingleton();

					[[maybe_unused]] const auto LoadMovieSuccess =
						ScaleformManager->LoadMovieEx(*this, "Interface/CASLevelUpMenu.swf"sv, "root", Scaleform::GFx::Movie::ScaleModeType::kExactFit);
					assert(LoadMovieSuccess);

					Scaleform::GFx::ASMovieRootBase* movieRoot = uiMovie.get()->asMovieRoot.get();
					Scaleform::GFx::Value bgsCodeObj;
					movieRoot->GetVariable(&bgsCodeObj, "root.Menu_mc.BGSCodeObj");

					filterHolder = std::make_unique<BSGFxShaderFXTarget>(*uiMovie, "root.Menu_mc");
					if (filterHolder)
					{
						filterHolder->CreateAndSetFiltersToHUD(HUDColorTypes::kGameplayHUDColor);
						shaderFXObjects.push_back(filterHolder.get());
					}

					RegisterFunction<OpenMenu>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "OpenMenu");
					RegisterFunction<SetSkills>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "SetSkills");
					RegisterFunction<ResetSkills>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "ResetSkills");
					RegisterFunction<ResetTagSkills>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "ResetTagSkills");
					RegisterFunction<BackToSkills>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "BackToSkills");
					RegisterFunction<UpdatePerkMenu>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "UpdatePerkMenu");
					RegisterFunction<AddPerks>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "AddPerks");
					RegisterFunction<TagSkills>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "TagSkills");
					RegisterFunction<LearnSpecial>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "LearnSpecial");
					RegisterFunction<ResetSpecial>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "ResetSpecial");
					RegisterFunction<PlaySkillSound>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "PlaySkillSound");
					RegisterFunction<PlayPerkSound>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "PlayPerkSound");
					RegisterFunction<CloseMenu>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "CloseMenu");
					RegisterFunction<PlayUISound>(&bgsCodeObj, uiMovie.get()->asMovieRoot, "PlayUISound");

					movieRoot->Invoke("root.Menu_mc.onCodeObjCreate", nullptr, nullptr, 0);
				};

				static IMenu* Create(const UIMessage&)
				{
					return new CASLevelUpMenu();
				}
			};

			bool RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value)
			{
				Scaleform::GFx::Value currentSWFPath;
				// Register native code handlers.
				if (a_view->asMovieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url"))
				{
					if (_stricmp(currentSWFPath.GetString(), "Interface/CASLevelUpMenu.swf") == 0)
					{
						Scaleform::GFx::Value bgsCodeObj;
						a_view->asMovieRoot->GetVariable(&bgsCodeObj, "root.Menu_mc.BGSCodeObj");

						RegisterFunction<OpenMenu>(&bgsCodeObj, a_view->asMovieRoot, "OpenMenu");
						RegisterFunction<SetSkills>(&bgsCodeObj, a_view->asMovieRoot, "SetSkills");
						RegisterFunction<ResetSkills>(&bgsCodeObj, a_view->asMovieRoot, "ResetSkills");
						RegisterFunction<ResetTagSkills>(&bgsCodeObj, a_view->asMovieRoot, "ResetTagSkills");
						RegisterFunction<BackToSkills>(&bgsCodeObj, a_view->asMovieRoot, "BackToSkills");
						RegisterFunction<UpdatePerkMenu>(&bgsCodeObj, a_view->asMovieRoot, "UpdatePerkMenu");
						RegisterFunction<AddPerks>(&bgsCodeObj, a_view->asMovieRoot, "AddPerks");
						RegisterFunction<TagSkills>(&bgsCodeObj, a_view->asMovieRoot, "TagSkills");
						RegisterFunction<LearnSpecial>(&bgsCodeObj, a_view->asMovieRoot, "LearnSpecial");
						RegisterFunction<ResetSpecial>(&bgsCodeObj, a_view->asMovieRoot, "ResetSpecial");
						RegisterFunction<PlaySkillSound>(&bgsCodeObj, a_view->asMovieRoot, "PlaySkillSound");
						RegisterFunction<PlayPerkSound>(&bgsCodeObj, a_view->asMovieRoot, "PlayPerkSound");
						RegisterFunction<CloseMenu>(&bgsCodeObj, a_view->asMovieRoot, "CloseMenu");
						RegisterFunction<PlayUISound>(&bgsCodeObj, a_view->asMovieRoot, "PlayUISound");

						a_view->asMovieRoot->Invoke("root.Menu_mc.onCodeObjCreate", nullptr, nullptr, 0);
					}
					return true;
				}
				return false;
			}

			inline void RegisterMenu()
			{
				if (UI* ui = UI::GetSingleton())
				{
					if (!ui->menuMap.contains("CASLevelUpMenu"))
					{
						ui->RegisterMenu("CASLevelUpMenu", CASLevelUpMenu::Create);
						REX::DEBUG("Registered 'CASLevelUpMenu'.");
					}
				}
			}
		}
	}
}