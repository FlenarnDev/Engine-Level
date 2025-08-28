#include "Systems/Dialogue.h"

namespace RE
{
	// Internal
	namespace
	{
		DialogueHolder g_dialogueHolder;

		struct DialogueEntry
		{
			TESTopicInfo* a_info;
			std::uint32_t optionID;
		};
	}

	void BuildDialogueMap(bool a_force)
	{
		BGSSceneActionPlayerDialogue* playerDialogue = GetCurrentPlayerDialogueAction();
		if (!playerDialogue)
		{
			return;
		}
		BGSScene* currentScene = PlayerCharacter::GetSingleton()->GetCurrentScene();

		// If we already have a map for this scene + scene action then return.
		if (!a_force && g_dialogueHolder.scene == currentScene && g_dialogueHolder.playerDialogue == playerDialogue)
		{
			return;
		}

		g_dialogueHolder.dialogueMap.clear();

		for (std::uint32_t c = 0; c < 4; c++)
		{

			TESTopic* playerTopic = playerDialogue->responseTopics[c];
			TESTopic* npcTopic = playerDialogue->pNPCResponseTopics[c];
			std::uint32_t playerInfoCount = playerTopic->numTopicInfos;
			std::uint32_t npcInfoCount = npcTopic ? npcTopic->numTopicInfos : 0;;

			BSTArray<TESTopicInfo*> randomOptions;

			// Loop through each player TopicInfo in the dialogue topic.
			for (std::uint32_t i = 0; i < playerInfoCount; i++)
			{
				TESTopicInfo* playerInfo = playerTopic->topicInfos[i];

				if (!playerInfo->responses.head && !playerInfo->dataInfo)
				{
					continue;  // Skip over infos with no content.
				}

				std::vector<TESTopicInfo*> npcResponses;

				std::uint32_t nextPlayerInfoIndex;
				for (nextPlayerInfoIndex = i + 1; nextPlayerInfoIndex < playerInfoCount; nextPlayerInfoIndex++)
				{
					TESTopicInfo* nextPlayerInfo = playerTopic->topicInfos[nextPlayerInfoIndex];
					if (nextPlayerInfo->responses.head || nextPlayerInfo->dataInfo)
					{
						// We found it, yay!
						break;
					}
				}

				if (nextPlayerInfoIndex == playerInfoCount && npcInfoCount > playerInfoCount)
				{
					// Reached the end of the player info list but there are still NPC infos.
					// Associate the remaining NPC infos with the current player info index.
					nextPlayerInfoIndex = npcInfoCount;
				}

				// Add all NPC infos from current player info index to next plyaer info index.
				for (std::uint32_t k = i; k < nextPlayerInfoIndex; k++)
				{
					if (k < npcInfoCount)
					{
						TESTopicInfo* npcInfo = npcTopic->topicInfos[k];
						npcResponses.push_back(npcInfo);
					}
				}

				g_dialogueHolder.dialogueMap.push_back(std::make_pair(playerInfo, npcResponses));
			}
		}

		g_dialogueHolder.scene = currentScene;
		g_dialogueHolder.playerDialogue = playerDialogue;
	}

	// Return player response TopicInfos.
	BSTArray<TESTopicInfo*> GetPlayerInfos()
	{
		BuildDialogueMap();
		BSTArray<TESTopicInfo*> infos{};
		for (auto const& info : g_dialogueHolder.dialogueMap)
		{
			infos.push_back(info.first);
		}
		return infos;
	}

	TESTopicInfo* GetPlayerInfo(BGSSceneActionPlayerDialogue* playerdialogue, std::uint32_t optionID)
	{
		BuildDialogueMap();
		if (optionID < g_dialogueHolder.dialogueMap.size())
		{
			return g_dialogueHolder.dialogueMap[optionID].first;
		}
		else
		{
			return nullptr;
		}
	}

	TESTopicInfo* PrependNPCSkillCheckText(TESTopicInfo* a_info)
	{
		if (TESCondition conditions = a_info->objConditions)
		{
			TESConditionItem* conditionItem = conditions.head;
			while (conditionItem != nullptr)
			{
				if (conditionItem->data.functionData.function.get() == SCRIPT_OUTPUT::kScript_DialogueGetAv)
				{
					const char* rawResponseText = a_info->responses.head->GetResponseText();
					if (!rawResponseText) continue;

					std::string responseText(rawResponseText);
					if (a_info->parentTopic && a_info->parentTopic->ownerQuest)
					{
						BSStringT<char> response;
						response.Set(rawResponseText, 500);
						BGSQuestInstanceText::ParseString(&response, a_info->parentTopic->ownerQuest, a_info->parentTopic->ownerQuest->currentInstanceID);
						responseText = response.c_str();
					}

					const std::string failedPrefix = "[FAILED] ";
					const std::string succeededPrefix = "[SUCCEEDED] ";
					switch (conditionItem->data.condition)
					{
					case 4: // ENUM_COMPARISON_CONDITION::kLessThan
						if (responseText.find(failedPrefix) != 0)
						{
							responseText.insert(0, failedPrefix);
						}
						break;
					case 3: // ENUM_COMPARISON_CONDITION::kGreaterThanEqual
						if (responseText.find(succeededPrefix) != 0)
						{
							responseText.insert(0, succeededPrefix);
						}
						break;
					default:
						break;
					}

					a_info->responses.head->responseText = responseText;
				}
				conditionItem = conditionItem->next;
			}
		}
		return a_info;
	}

	// Returns the first NPC response info that passes its condition check.
	TESTopicInfo* GetNPCInfo(BGSSceneActionPlayerDialogue* playerDialogue, std::uint32_t optionID)
	{
		BuildDialogueMap();
		std::vector<TESTopicInfo*> npcInfos = g_dialogueHolder.dialogueMap[optionID].second;
		BSTArray<TESTopicInfo*> randomOptions{};
		std::unordered_map<TESTopicInfo*, bool> infoGroupConditions;

		for (TESTopicInfo* info : npcInfos)
		{
			if ((info->formFlags >> 5) & 1) // Check if 'Deleted'.
			{
				continue;
			}

			if (info->formFlags & TESTopicInfo::Flags::kInfoGroup)
			{
				infoGroupConditions[info] = EvaluateInfoConditions(info, playerDialogue, true);
				continue;
			}

			if (TESTopicInfo* parentInfoGroup = info->GetParentInfoGroup())
			{
				auto it = infoGroupConditions.find(parentInfoGroup);
				if (it != infoGroupConditions.end())
				{
					if (!it->second)
					{
						continue;
					}
				}
				else
				{
					bool valid = EvaluateInfoConditions(parentInfoGroup, playerDialogue, true);
					infoGroupConditions[parentInfoGroup] = valid;
					
					if (!valid)
					{
						continue;
					}
				}
			}

			if ((info->data.flags & TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kSayOnce) && (info->data.flags & TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kDialogueInfoSaid))
			{
				continue;
			}

			if (info->data.flags & TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kRandom)
			{
				if (EvaluateInfoConditions(info, playerDialogue, true))
				{
					randomOptions.push_back(info);
				}

				if (info->data.flags & TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kRandomEnd && randomOptions.size() > 0)
				{
					return PrependNPCSkillCheckText(randomOptions[BSRandom::UnsignedInt(0, randomOptions.size())]);
				}
				continue;
			}

			if (randomOptions.size() > 0)
			{
				return PrependNPCSkillCheckText(randomOptions[BSRandom::UnsignedInt(0, randomOptions.size())]);
			}

			if (EvaluateInfoConditions(info, playerDialogue, true))
			{
				return PrependNPCSkillCheckText(info);
			}
		}

		// Do a random roll if the last info topic is 'random' flagged, but not 'randomEnd'.
		if (randomOptions.size() != 0)
		{
			// Shortcut, skip BSRandom::UnsignedInt if only one option available.
			if (randomOptions.size() == 1)
			{
				return PrependNPCSkillCheckText(randomOptions[0]);
			}
			return PrependNPCSkillCheckText(randomOptions[BSRandom::UnsignedInt(0, randomOptions.size())]);
		}

		// All infos failed their condition checks.
		return nullptr;
	}

	// Return the first NPC response info in a NPC Response scene action that passes its condition check.
	// Allows for infos to be grouped via info groups.
	TESTopicInfo* GetNPCResponseInfo(BGSSceneActionNPCResponseDialogue* a_npcDialogue, uint32_t optionID)
	{
		// Vector of vector of TESTopicInfo
		// infos[optionId][items inside the infogroup]
		std::vector<std::vector<TESTopicInfo*>> infos(optionID + 1, std::vector<TESTopicInfo*>());

		std::map<TESTopicInfo*, std::uint32_t> infoGroupMap;
		std::uint32_t index = 0;  // The next available free slot in the vector.

		for (std::uint32_t c = 0; c < 4; c++)
		{
			TESTopic* topic = a_npcDialogue->responseTopics[c];
			std::uint32_t infoCount = topic ? topic->numTopicInfos : 0;

			// Loop through all TopicInfos while index <= optionID + 1
			for (std::uint32_t i = 0; i < infoCount && index <= optionID + 1; i++)
			{
				TESTopicInfo* info = topic->topicInfos[i];

				if (info->formFlags & TESTopicInfo::Flags::kInfoGroup)
				{
					// This is a info group.
					infoGroupMap[info] = index++;
				}
				else
				{
					if (TESTopicInfo* parentInfoGroup = info->GetParentInfoGroup())
					{
						// Info has a parent.
						auto idx = infoGroupMap.find(parentInfoGroup);
						if (idx != infoGroupMap.end())
						{
							// Put it into the parents slot.
							infos[idx->second].push_back(info);
						}
					}
					else
					{
						// No parent, add normally.
						if (index <= optionID)
						{
							infos[index++].push_back(info);
						}
						else
						{
							break;
						}
					}
				}
			}
		}

		// Evaluate conditions and pick the best one from the group.
		std::vector<TESTopicInfo*> infoGroup = infos[optionID];

		for (TESTopicInfo* info : infoGroup)
		{
			if (EvaluateInfoConditions(info, a_npcDialogue, true))
			{
				if (info->responses.head || info->dataInfo)
				{
					return info;
				}
				else
				{
					return nullptr;  // Return nullptr if this is an empty info.
				}
			}
		}

		// Nothing was found.
		return nullptr;
	}

	BGSSceneActionNPCResponseDialogue* FindNextNPCResponseAction(BGSScene* scene, std::uint32_t currentPhase)
	{
		if (scene)
		{
			for (std::uint32_t i = 0; i < scene->actions.size(); i++)
			{
				BGSSceneAction* action = scene->actions[i];
				if (action->startPhase == currentPhase + 1)
				{
					// Check that the action is a NPC response action and that the alias is filled for this action.
					// If the alias is not filled, any conditions that check the subject will cause a CTD.
					if (action->GetActionType() == SCENE_ACTION_TYPE::kNPCResponse && GetActionREFR(scene, action))
					{
						return reinterpret_cast<BGSSceneActionNPCResponseDialogue*>(action);
					}
				}
			}
		}

		return nullptr;
	}

	// Returns the currently executing player dialogue action, or NULL if no player dialogue action is currenctly active.
	BGSSceneActionPlayerDialogue* GetCurrentPlayerDialogueAction()
	{
		BGSScene* scene = PlayerCharacter::GetSingleton()->GetCurrentScene();
		if (scene)
		{
			for (std::uint32_t i = 0; i < scene->actions.size(); i++)
			{
				BGSSceneAction* action = scene->actions[i];
				if (action->GetActionType() == SCENE_ACTION_TYPE::kPlayerDialogue)
				{
					if (action->status & BGSSceneAction::Status::kRunning || IsSceneActionWithinPhase(action, scene->currentActivePhase))
					{
						return (BGSSceneActionPlayerDialogue*)(action);
					}
				}
			}
		}
		REX::DEBUG("GetCurrentPlayerDialogueAction - return `nullptr`.");
		return nullptr;
	}

	bool SelectDialogueOption(std::uint32_t option)
	{
		if (!(MenuTopicManager::GetSingleton()->allowInput))
		{
			return false;
		}

		if (BGSSceneActionPlayerDialogue* playerDialogue = GetCurrentPlayerDialogueAction())
		{
			// We're using option 5 and up for additional options. (Options 5 = Option 0).
			PlayerCharacter::GetSingleton()->SetLastDialogueInput(option + 5);
			return true;
		}
		else
		{
			return false;
		}
	}

	TESObjectREFR* GetCurrentPlayerDialogueTarget()
	{
		if (BGSSceneActionPlayerDialogue* playerDialogue = GetCurrentPlayerDialogueAction())
		{
			BGSScene* scene = PlayerCharacter::GetSingleton()->GetCurrentScene();

			BSPointerHandle<TESObjectREFR> targetHandle;
			TESObjectREFR* targetREFR = nullptr;

			if (scene)
			{
				scene->parentQuest->GetAliasedRef(&targetHandle, playerDialogue->actorID);
			}

			auto REFRptr = targetHandle.get();

			if (targetHandle && REFRptr != nullptr)
			{
				targetREFR = REFRptr.get();
				return targetREFR;
			}

			return nullptr;
		}
		return nullptr;
	}

	// Check to see whether the given scene action is active in the given phase.
	bool IsSceneActionWithinPhase(BGSSceneAction* action, std::uint32_t phase)
	{
		return (action->startPhase <= phase && action->endPhase >= phase);
	}

	// Return the reference associated with the action.
	TESObjectREFR* GetActionREFR(BGSScene* scene, BGSSceneAction* action)
	{
		BSPointerHandle<TESObjectREFR> targetHandle;
		TESObjectREFR* targetREFR = nullptr;

		if (scene)
		{
			scene->parentQuest->GetAliasedRef(&targetHandle, action->actorID);
		}

		auto REFRptr = targetHandle.get();

		if (targetHandle)
		{
			targetREFR = REFRptr.get();
			return targetREFR;
		}

		return nullptr;
	}

	// Topic Infos
	bool EvaluateInfoConditions(TESTopicInfo* a_info, BGSSceneAction* a_action, bool a_swap)
	{
		PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
		TESCondition conditions = a_info->objConditions;
		if (!conditions)
		{
			return true;
		}

		BSPointerHandle<TESObjectREFR> targetHandle;
		TESObjectREFR* targetREFR = nullptr;
		BGSScene* scene = playerCharacter->GetCurrentScene();

		if (scene)
		{
			scene->parentQuest->GetAliasedRef(&targetHandle, a_action->actorID);
		}

		NiPointer<TESObjectREFR> REFRptr = targetHandle.get();

		if (targetHandle)
		{
			targetREFR = REFRptr.get();
		}
		else
		{
			targetREFR = playerCharacter;
		}

		// Test against conditions
		TESObjectREFR* refA, * refB;
		if (!a_swap)
		{
			refA = playerCharacter;
			refB = targetREFR;
		}
		else
		{
			refA = targetREFR;
			refB = playerCharacter;
		}

		return conditions.IsTrue(refA, refB);
	}

	std::vector<DialogueOption> GetDialogueOptions()
	{
		std::vector<DialogueOption> options;

		if (BGSSceneActionPlayerDialogue* playerDialogue = GetCurrentPlayerDialogueAction())
		{
			BSTArray<TESTopicInfo*> infos = GetPlayerInfos();
			BSTArray<DialogueEntry> viableInfos;
			BSTArray<DialogueEntry> randomOptions;

			BGSScene* currentScene = PlayerCharacter::GetSingleton()->GetCurrentScene();

			std::uint32_t lastResponseIndex = -1;
			bool initialized = false;
			std::uint32_t currentInfoIndex = 0;
			for (TESTopicInfo* info : infos)
			{
				std::uint_fast32_t currentResponseIndex;

				for (std::uint32_t c = 0; c < 4; c++)
				{
					if (playerDialogue->responseTopics[c] == info->parentTopic)
					{
						currentResponseIndex = c;

						if (!initialized)
						{
							lastResponseIndex = currentResponseIndex;
							initialized = true;
						}
						break;
					}
				}

				if ((info->formFlags >> 5) & 1) // Check if form is 'Deleted'.
				{
					continue;
				}

				if (info->IsSayOnce() && info->data.flags.all(TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kDialogueInfoSaid))
				{
					continue;
				}

				if (info->IsRandom())
				{
					if (lastResponseIndex == currentResponseIndex)
					{
						randomOptions.push_back(DialogueEntry(info, currentInfoIndex));

						if (info->IsRandomEnd() && !randomOptions.empty())
						{
							std::uint32_t randomOption = BSRandom::UnsignedInt(0, randomOptions.size());
							viableInfos.push_back(DialogueEntry(randomOptions[randomOption].a_info, randomOptions[randomOption].optionID));
							randomOptions.clear();
						}
					}
					else
					{
						if (!randomOptions.empty())
						{
							std::uint32_t randomOption = BSRandom::UnsignedInt(0, randomOptions.size());
							viableInfos.push_back(DialogueEntry(randomOptions[randomOption].a_info, randomOptions[randomOption].optionID));
						}

						randomOptions.clear();
						randomOptions.push_back(DialogueEntry(info, currentInfoIndex));
						lastResponseIndex = currentResponseIndex;
					}
				}
				else
				{
					viableInfos.push_back(DialogueEntry(info, currentInfoIndex));
				}
				++currentInfoIndex;
			}

			if (!randomOptions.empty())
			{
				std::uint32_t randomOption = BSRandom::UnsignedInt(0, randomOptions.size());
				viableInfos.push_back(DialogueEntry(randomOptions[randomOption].a_info, randomOptions[randomOption].optionID));
			}

			for (DialogueEntry infoEntry : viableInfos)
			{
				TESTopicInfo* info = infoEntry.a_info;

				// If the Info was a SharedInfo, iterate through the dataInfo's until we have the actual info.
				TESTopicInfo* originalInfo = info;
				bool dataInfo = false;

				if (info->dataInfo)
				{
					dataInfo = true;
				}
				while (info->dataInfo)
				{
					info = info->dataInfo;
				}

				if ((info->formFlags >> 5) & 1) // Check if form is 'Deleted'.
				{
					continue;
				}

				if ((info->IsSayOnce()) && (info->data.flags & TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kDialogueInfoSaid))
				{
					continue;
				}

				// Get prompt
				BGSLocalizedString prompt;

				auto idx = GetAllPromptMap().find(info);
				if (idx != GetAllPromptMap().end())
				{
					prompt = idx->second;
				}

				// Get response and perform text replacement.
				std::string responseText = "";
				if (info->responses.head != nullptr)
				{
					const char* rawResponseText = info->responses.head->GetResponseText();
					BSStringT<char> response;
					response.Set(rawResponseText, 500);

					if (info->parentTopic && info->parentTopic->ownerQuest)
					{
						BGSQuestInstanceText::ParseString(&response, info->parentTopic->ownerQuest, info->parentTopic->ownerQuest->currentInstanceID);
					}
					responseText = response.c_str();
					TESCondition conditions = info->objConditions;
					if (conditions)
					{
						TESConditionItem* conditionItem = conditions.head;

						while (conditionItem != nullptr)
						{
							if (conditionItem->data.functionData.function == SCRIPT_OUTPUT::kScript_DialogueGetAv)
							{
								if (conditionItem->data.condition == std::to_underlying(ENUM_COMPARISON_CONDITION::kLessThan))
								{
									ActorValueInfo* actorValue = static_cast<ActorValueInfo*>(conditionItem->data.functionData.param[0]);
									if (actorValue)
									{
										prompt = R"({"BorderColor":16724787, "IconColor" : 16724787, "TextColor" : 16724787})";
										std::string skillCheck = "[" + std::string(actorValue->GetFullName()) + " " + std::to_string(static_cast<int>(PlayerCharacter::GetSingleton()->GetActorValue(*actorValue))) + "/" + std::to_string(static_cast<int>(conditionItem->data.value)) + "] ";
										responseText.insert(0, skillCheck);
									}
								}
								else if (conditionItem->data.condition == std::to_underlying(ENUM_COMPARISON_CONDITION::kGreaterThanEqual))
								{
									ActorValueInfo* actorValue = static_cast<ActorValueInfo*>(conditionItem->data.functionData.param[0]);
									if (actorValue)
									{
										std::string skillCheck = "[" + std::string(actorValue->GetFullName()) + " " + std::to_string(static_cast<int>(conditionItem->data.value)) + "] ";
										responseText.insert(0, skillCheck);
									}
								}
							}
							else if (conditionItem->data.functionData.function == SCRIPT_OUTPUT::kScript_HasPerk || conditionItem->data.functionData.function == SCRIPT_OUTPUT::kScript_DialogueHasPerk)
							{
								if (conditionItem->data.condition == std::to_underlying(ENUM_COMPARISON_CONDITION::kEqual))
								{
									BGSPerk* perk = static_cast<BGSPerk*>(conditionItem->data.functionData.param[0]);
									if (perk)
									{
										std::string perkCheck = "[" + std::string(perk->GetFullName()) + "] ";
										responseText.insert(0, perkCheck);
									}
								}
							}
							conditionItem = conditionItem->next;
						}
					}
				}

				// Expanded logic for 'Enabled' value, needs to take into account possible values from both Shared and 'local' info.
				bool enabled = false;
				if (dataInfo)
				{
					if (EvaluateInfoConditions(info, playerDialogue) && EvaluateInfoConditions(originalInfo, playerDialogue))
					{
						enabled = true;
					}
					info = originalInfo;
				}
				else
				{
					enabled = EvaluateInfoConditions(info, playerDialogue);
				}


				// Get NPC response TopicInfo for dialogue cues.
				TESTopicInfo* npcResponseInfo = GetNPCInfo(playerDialogue, infoEntry.optionID);
				if (!npcResponseInfo)
				{
					// No NPC response info - look one phase ahead (only) for a NPC response action.
					if (BGSSceneActionNPCResponseDialogue* npcResponseAction = FindNextNPCResponseAction(currentScene, currentScene->currentActivePhase))
					{
						// Another framework check normally goes here...
						npcResponseInfo = GetNPCResponseInfo(npcResponseAction, infoEntry.optionID);
					}
				}

				// Get scene links for response.
				TOPIC_INFO_SCENEDATA* sceneData = npcResponseInfo ? GetSceneData(npcResponseInfo) : nullptr;

				bool endsScene = false;
				if (info->data.flags.all(TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kEndRunningScene))
				{
					endsScene = true;
				}
				else if (npcResponseInfo)
				{
					if (npcResponseInfo->data.flags.all(TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kEndRunningScene))
					{
						endsScene = true;
					}
				}

				//REX::DEBUG("Response text: {:s}", responseText);

				DialogueOption option = {};
				option.optionID = infoEntry.optionID;
				option.info = info;
				option.prompText = prompt.data();
				option.responseText = responseText;
				option.enabled = enabled;
				option.said = info->data.flags.all(TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kDialogueInfoSaid);
				option.challengeLevel = info->GetChallengeLevel();
				option.challengeResult = info->GetSuccessLevel();
				option.linkedToSelf = sceneData ? (currentScene == sceneData->scene && playerDialogue->startPhase >= sceneData->phase && playerDialogue->endPhase <= sceneData->phase) : false;
				option.endsScene = endsScene;
				option.isBarterOption = npcResponseInfo ? Cascadia::HasVMScript(npcResponseInfo, "VendorInfoScript") : false;
				option.isInventoryOption = npcResponseInfo ? Cascadia::HasVMScript(npcResponseInfo, "OpenInventoryInfoScript") : false;
				options.push_back(option);
			}
		}

		return options;
	}

	TOPIC_INFO_SCENEDATA* GetSceneData(TESTopicInfo* topicInfo)
	{
		auto& sceneListMap = GetAllSceneListMap();
		auto sceneDataIter = sceneListMap.find(topicInfo);
		if (sceneDataIter != sceneListMap.end())
		{
			return &sceneDataIter->second;
		}
		return nullptr;
	}

	// Unused, ported over in case a need ever comes up.
	void SetSceneData(TESTopicInfo* topicInfo, BGSScene* scene, std::uint32_t phase)
	{
		if (!topicInfo)
		{
			return;
		}

		auto& sceneListMap = GetAllSceneListMap();
		auto sceneDataIter = sceneListMap.find(topicInfo);

		if (sceneDataIter != sceneListMap.end())
		{
			// Update existing TOPIC_INFO_SCENEDATA
			sceneDataIter->second.scene = scene;
			sceneDataIter->second.phase = phase;
		}
		else
		{
			TOPIC_INFO_SCENEDATA newSceneData;
			newSceneData.scene = scene;
			newSceneData.phase = phase;
			sceneListMap.emplace(topicInfo, newSceneData);
		}
	}

	void StartScene(BGSScene* apScene, std::uint32_t apPhase)
	{
		if (!apScene) return;
		BGSScene* currentScene = PlayerCharacter::GetSingleton()->GetCurrentScene();

		if (currentScene && currentScene != apScene)
		{
			currentScene->niFlags.flags |= static_cast<std::uint32_t>(BGSScene::BOOL_BITS::kQueueActive);
			apScene->ResetAllSceneActions();
			apScene->niFlags.flags &= static_cast<std::uint32_t>(BGSScene::BOOL_BITS::kPauseScene);
			apScene->SetSceneActive(true);
			apScene->startPhase = apPhase;
		}
	}

	void SetPlayerDialogue(bool a_enable)
	{
		if (a_enable)
		{
			// Restore player dialogue starting.
			// StartPlayerDialogue - 2196817 + 652 offset.
			REL::Relocation<std::uintptr_t> startPlayerDialogue{ ID::BGSSceneActionNPCResponseDialogue::UpdateAction1 };
			startPlayerDialogue.write<0x652>({ 0x48, 0x8B, 0xC8 });

			// SwitchToPlayerCC - 2214898 ID + ADC offset.
			REL::Relocation<std::uintptr_t> switchToPlayerCC{ ID::DialogueCameraState::UpdateActorsAndAction };
			switchToPlayerCC.write<0xADC>({ 0x01 });
		}
		else
		{
			// Disable player dialogue starting.
			uint8_t bytes[] = { 0xEB, 0x1E, 0x90 };
			// StartPlayerDialogue - 2196817 + 652 offset.
			REL::Relocation<std::uintptr_t> startPlayerDialogue{ ID::BGSSceneActionNPCResponseDialogue::UpdateAction1 };
			startPlayerDialogue.write<0x652>({ 0xEB, 0x1E, 0x90 });

			// Disable dialogue camera switching to player.
			// SwitchToPlayerCC - 2214898 ID + ADC offset.
			REL::Relocation<std::uintptr_t> switchToPlayerCC{ ID::DialogueCameraState::UpdateActorsAndAction };
			switchToPlayerCC.write<0xADC>({ 0x02 });
		}
	}

	// ===============
	//		Hooks
	// ===============

	TESTopicInfo* GetCurrentTopicInfo_Player_Hook(BGSSceneActionPlayerDialogue* apPlayerDialogue, BGSScene* apParentScene, TESObjectREFR* apTarget, std::uint32_t aeType)
	{
		// Use >5 for custom selections.
		if (apPlayerDialogue->playerInput >= 5)
		{
			std::uint32_t selectedOption = apPlayerDialogue->playerInput - 5;
			TESTopicInfo* info = GetPlayerInfo(apPlayerDialogue, selectedOption);

			if (info)
			{
				// Disable player dialogue.
				SetPlayerDialogue(false);

				// Mark info as 'said'.
				info->AddChange(CHANGE_TYPE::kTopicSaidPlayer);
				info->data.flags.set(TOPIC_INFO_DATA::TOPIC_INFO_FLAGS::kDialogueInfoSaid);

				TOPIC_INFO_SCENEDATA* sceneData = GetSceneData(info);
				if (sceneData)
				{
					// This is necessary as no response => no pre/post TopicInfo will run.
					StartScene(sceneData->scene, sceneData->phase);
				}
			}
			else
			{
				// Info not found...
				// reset playerInput to 4 so that the game doesn't keep trying to ask us for a non-existent TESTopicInfo.
				apPlayerDialogue->playerInput = 4;
			}
			return info;
		}
		// Re-enable player dialogue.
		SetPlayerDialogue(true);
		return apPlayerDialogue->GetCurrentTopicInfo(apParentScene, apTarget, aeType);
	}

	TESTopicInfo* GetCurrentTopicInfo_NPC_Hook(BGSSceneActionPlayerDialogue* apPlayerDialogue, BGSScene* apParentScene, TESObjectREFR* apTarget, std::uint32_t aeType)
	{
		// Use >5 for custom selections.
		if (apPlayerDialogue->playerInput >= 5)
		{
			std::uint32_t selectedOption = apPlayerDialogue->playerInput - 5;

			TESTopicInfo* info = GetNPCInfo(apPlayerDialogue, selectedOption);
			if (info)
			{
				// If the player info was a speech challenge then trigger the post-dialogue handler to display the challenge-success animation.
				// This needs to be done explicitly as the challenge-success animation is coded to trigger after the player dialogue-elapsed timer reaches zero.
				// Since player dialogue length is always zero with the framework active, the timed code will not run and an explicit call needs to be made.
				if (TESTopicInfo* playerInfo = GetPlayerInfo(apPlayerDialogue, selectedOption))
				{
					if (playerInfo->GetChallengeLevel() > TESTopicInfo::CC_CHALLENGE_NONE)
					{
						PlayerCharacter::GetSingleton()->UpdateVoiceTimer(true);
					}
				}
			}

			return info;
		}

		return apPlayerDialogue->GetCurrentTopicInfo(apParentScene, apTarget, aeType);
	}

	TESTopicInfo* GetCurrentTopicInfo_NPCAction_Hook(BGSSceneActionNPCResponseDialogue* apNPCDialogue, BGSScene* apParentScene)
	{
		std::uint32_t dialogueOption = PlayerCharacter::GetSingleton()->playerDialogueInput.underlying();

		// Use >5 for custom selections.
		if (dialogueOption >= 5)
		{
			std::uint32_t selectedOption = dialogueOption - 5;

			TESTopicInfo* info = GetNPCResponseInfo(apNPCDialogue, selectedOption);
			return info;
		}

		return apNPCDialogue->GetCurrentTopicInfo(apParentScene);
	}
}