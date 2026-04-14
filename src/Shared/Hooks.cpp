#include "Shared/Hooks.h"

using namespace RE;

namespace Cascadia
{
	extern std::map<TESAmmo*, float> ammoDegradationMap;

	namespace Hooks
	{
		#define NOP_BYTES(size) std::vector<std::uint8_t>(size, 0x90)

		void HookPipboyDataPopulateItemCardInfo(PipboyInventoryData* a_pipboyInventoryData, const BGSInventoryItem* a_inventoryItem, const BGSInventoryItem::Stack* a_stack, PipboyObject* a_data)
		{
			a_pipboyInventoryData->PopulateItemCardInfo(a_inventoryItem, a_stack, a_data);

			if (a_stack->extra->HasType(EXTRA_DATA_TYPE::kHealth))
			{
				PipboyArray* pipboyArray = a_data->GetMember<PipboyArray*>(BSFixedString("itemCardInfoList"));
				const BSFixedStringCS cnd = BSFixedStringCS("CND");
				a_pipboyInventoryData->AddItemCardInfoEntry(&cnd, a_stack->extra->GetHealthPerc() * 100.0f, pipboyArray);

				std::uint32_t cndIndexPosition = a_inventoryItem->object->GetFormType() == ENUM_FORM_ID::kWEAP ? 1 : 0;

				PipboyValue* cndEntry = std::move(pipboyArray->elements.back());
				for (std::size_t i = pipboyArray->elements.size() - 1; i > cndIndexPosition; --i)
				{
					pipboyArray->elements[i] = std::move(pipboyArray->elements[i - 1]);
				}
				pipboyArray->elements[cndIndexPosition] = std::move(cndEntry);
			}
			return;
		}

		std::int64_t HookActorUtilsArmorRatingVisitorBaseOperator(ActorUtils::ArmorRatingVisitorBase* a_this, BGSInventoryItem* a_item, std::uint32_t a_stackID)
		{
			std::int64_t result = a_this->operator()(a_item, a_stackID);

			BGSInventoryItem::Stack* stack = nullptr;

			// Retail logic followed here, don't ask.
			if (a_stackID == -1)
			{
				BGSInventoryItem::Stack* pointer = a_item->stackData.get();
				while (pointer)
				{
					if (!a_this->checkEquipped || (stack ==	 nullptr && (pointer->flags.any(BGSInventoryItem::Stack::Flag::kSlotMask))))
					{
						stack = pointer;
					}

					if (stack)
					{
						break;
					}

					pointer = pointer->nextStack.get();
				}
			}
			else
			{
				stack = a_item->GetStackByID(a_stackID);
			}

			if (stack && stack->extra && stack->extra->HasType(EXTRA_DATA_TYPE::kHealth))
			{
				float condition = stack->extra->GetHealthPerc();
				a_this->rating = a_this->rating * (0.66f + std::min((0.34f * condition) / 0.5f, 0.34f));
			}

			return result;
		}

		bool HookFavoritesManagerUseQuickkeyItem(FavoritesManager* a_this, std::uint32_t a_quickkeyIndex)
		{
			REX::DEBUG("'FavoritesManager::UseQuickkeyItem' - quickkey index: {}.", a_quickkeyIndex);
			if (a_quickkeyIndex == 1)
			{
				return AmmoSwitch::InitializeAmmoSwitch();
			}
			else
			{
				return a_this->UseQuickkeyItem(a_quickkeyIndex);
			}
		}

		void HookWorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu()
		{
			GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
			SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sCannotBuildMessage")->GetString().data(), nullptr, true, true);
			return;
		}

		void HookBGSTerminalActivate_ShowHUDMessage(const char* a_message, const char* a_sound, bool a_throttle, bool a_warning)
		{
			// TODO: A bit hacky? Alternative is match for full strings.
			if (std::strchr(a_message, '{') == nullptr)
			{
				SendHUDMessage::ShowHUDMessage(a_message, a_sound, a_throttle, a_warning);
			}
		}

		char HookAIFormulasGetBarterValue_HandleEntryPoint(std::int32_t a_EntryPoint, Actor* a_perkOwner, ...)
		{
			bool selling = (static_cast<BGSEntryPoint::ENTRY_POINT>(a_EntryPoint) == BGSEntryPoint::ENTRY_POINT::kModSellPrices);

			va_list args;
			va_start(args, a_perkOwner);

			BGSObjectInstance* objectInstance = va_arg(args, BGSObjectInstance*);
			float* newPrice = va_arg(args, float*);

			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			float barterMultiplier;


			if (selling)
			{
				barterMultiplier = 0.45f * playerCharacter->GetActorValue(*Skills::CascadiaActorValues.Barter) + 45.0f;
			}
			else
			{
				barterMultiplier = -0.45f * playerCharacter->GetActorValue(*Skills::CascadiaActorValues.Barter) + 155.0f;
				
			}

			REX::DEBUG("Selling: {}, barter modifier: {}", selling, barterMultiplier);

			*newPrice *= (barterMultiplier / 100.0f);

			va_end(args);
			return 1;
		}

		// We don't want the players 'Charisma' stat to influence prices, new calculation is based purely on 'Barter' level.
		float HookGamePlayFormulasCalculateItemValue_GetBarterValue(float a_baseValue, float a_charisma, bool a_selling, TESObjectREFR* a_refTarget)
		{
			return AIFormulas::GetBarterValue(a_baseValue, 0.0f, a_selling, a_refTarget);
		}

		float HookActorUpdateSprinting_CalcSprintingActionPoints(float a_equippedWeight, float a_elapsedTime, float a_endurance)
		{
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			return playerCharacter->IsJumping() ? 0 : GamePlayFormulas::CalcSprintingActionPoints(a_equippedWeight, a_elapsedTime, a_endurance);
		}

		// Util function for Actor::Jump hooks
		float JumpCostCalculator(Actor* a_actor)
		{
			ActorValue* actorValue = ActorValue::GetSingleton();

			float calculatedCost = 5.0;
			float maxActionPoints = a_actor->GetActorValue(*actorValue->actionPoints);

			std::uint32_t brokenLegs = 0;
			if (a_actor->GetActorValue(*actorValue->leftMobiltyCondition) == 0.0f )
			{
				brokenLegs += 1;
			}
			
			if (a_actor->GetActorValue(*actorValue->rightMobilityCondition) == 0.0f)
			{
				brokenLegs += 1;
			}

			calculatedCost += calculatedCost * (brokenLegs * 0.5f);

			return calculatedCost;
		}


		std::uint8_t HookActorJump_GetMobilityCrippled(Actor* a_this)
		{
			if (a_this->GetActorValue(*ActorValue::GetSingleton()->actionPoints) < JumpCostCalculator(a_this))
			{
				GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
				SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sNoJumpWarning")->GetString().data(), nullptr, true, true);
				return 2;
			}

			return a_this->GetMobilityCrippled();
		}

		void HookActorJump_bhkCharacterControllerJump(bhkCharacterController* a_this, float a_height)
		{
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			float jumpCost = JumpCostCalculator(playerCharacter);

			ActorValue* actorValue = ActorValue::GetSingleton();
			playerCharacter->ModActorValue(ACTOR_VALUE_MODIFIER::kDamage, *actorValue->actionPoints, -1.0f * jumpCost);

			// Note: If we want to globally modify the players jump height, here is a good place to do it.
			return a_this->Jump(a_height);
		}

		void HookTaskQueueInterfaceTaskUnpackFunc_DoHitMe(Actor* a_this, HitData* a_hitData)
		{
			const auto targetActor = a_hitData->target.get().get();

			if (!targetActor)
			{
				REX::DEBUG("HookedDoHitMe: skipping hit with null target actor.");
				a_this->DoHitMe(a_hitData);
				return;
			}

			if (targetActor->IsDead(false))
			{
				REX::DEBUG("HookedDoHitMe: skipping dead actor {:08X}", targetActor->formID);
				a_this->DoHitMe(a_hitData);
				return;
			}

			if (a_hitData->flags.any(HitData::Flag::kExplosion))
			{
				REX::DEBUG("HookedDoHitMe: skipping explosion hit on {:08X}", a_this->formID);
				a_this->DoHitMe(a_hitData);
				return;
			}

			std::uint8_t projCount = 1;
			const auto weapon = a_hitData->weapon.object;
			if (weapon && weapon->formType == ENUM_FORM_ID::kWEAP)
			{
				const auto instanceData = a_hitData->weapon.instanceData.get();
				const auto weaponData = instanceData
					? reinterpret_cast<TESObjectWEAP::InstanceData*>(instanceData)
					: static_cast<TESObjectWEAP::InstanceData*>(&weapon->As<TESObjectWEAP>()->weaponData);
				if (weaponData && weaponData->type == WEAPON_TYPE::kGun)
				{
					const auto rangedData = weaponData->rangedData;
					if (rangedData)
						projCount = rangedData->numProjectiles;
				}
			}

			Actor* aggressor = a_hitData->aggressor.get().get();

			const auto damageTypes = a_hitData->damageTypes;
			const bool hasTypedDamage = damageTypes && !damageTypes->empty();

			REX::DEBUG("HookedDoHitMe: target={:08X} aggressor={:08X} projCount={} weapon={:08X} damageTypeCount={}",
				targetActor->formID,
				aggressor ? aggressor->formID : 0,
				projCount,
				weapon ? weapon->formID : 0,
				hasTypedDamage ? damageTypes->size() : 0);

			REX::DEBUG("HookedDoHitMe: before -- totalDamage={:.2f} physicalDamage={:.2f} "
				"resistedPhysicalDamage={:.2f} healthDamage={:.2f} resistedTypedDamage={:.2f} "
				"sneakMult={:.2f} critMult={:.2f}",
				a_hitData->totalDamage,
				a_hitData->physicalDamage,
				a_hitData->resistedPhysicalDamage,
				a_hitData->healthDamage,
				a_hitData->resistedTypedDamage,
				a_hitData->sneakAttackBonus,
				a_hitData->criticalDamageMult);

			const float divisor = a_hitData->totalDamage - a_hitData->resistedPhysicalDamage;

			REX::DEBUG("HookedDoHitMe: divisor={:.2f} hasTypedDamage={}", divisor, hasTypedDamage);

			// For pure energy weapons physicalDamage and resistedPhysicalDamage are both 0
			// so we can't derive bpmMult from physical fields - use 1.0 as fallback
			if (divisor == 0.0f && !hasTypedDamage)
			{
				REX::DEBUG("HookedDoHitMe: no physical or typed damage, skipping.");
				a_this->DoHitMe(a_hitData);
				return;
			}

			const float bpmMult = divisor != 0.0f ? a_hitData->healthDamage / divisor : 1.0f;

			REX::DEBUG("HookedDoHitMe: bpmMult={:.4f}", bpmMult);

			BGSObjectInstance targetInstance(targetActor, nullptr);
			BGSObjectInstance aggressorInstance(aggressor, nullptr);


			a_hitData->healthDamage = 0.0f;

			if (hasTypedDamage)
			{
				const auto dmgCount = damageTypes->size();
				for (std::uint32_t i = 0; i < dmgCount; ++i)
				{
					const auto& [form, value] = (*damageTypes)[i];

					float baseDmg = value.f;
					const float critBonus = (a_hitData->criticalDamageMult - 1.0f) * baseDmg;

					REX::DEBUG("HookedDoHitMe: damageType[{}] form={:08X} rawBaseDmg={:.2f} critBonus={:.2f}",
						i, form ? form->formID : 0, baseDmg, critBonus);

					if (aggressor)
					{
						BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModAttackDamage, aggressor, a_hitData->weapon, targetInstance, &baseDmg);
						BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModTypedAttackDamage, aggressor, a_hitData->weapon, targetInstance, &baseDmg);
					}

					REX::DEBUG("HookedDoHitMe: damageType[{}] baseDmg after attack perks={:.2f}", i, baseDmg);

					baseDmg *= a_hitData->sneakAttackBonus;
					baseDmg += critBonus;

					REX::DEBUG("HookedDoHitMe: damageType[{}] baseDmg after sneak+crit={:.2f}", i, baseDmg);

					const auto dmgType = form && form->formType == RE::ENUM_FORM_ID::kDMGT ? form->As<RE::BGSDamageType>() : nullptr;
					const auto avInfo = dmgType ? dmgType->data.resistance : nullptr;

					float typedDR = avInfo ? targetActor->GetActorValue(*avInfo) : 0.0f;

					REX::DEBUG("HookedDoHitMe: damageType[{}] avInfo={} typedDR before perks={:.2f}", i, avInfo != nullptr, typedDR);

					if (aggressor && avInfo)
					{
						BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModTargetDamageResistance, aggressor, a_hitData->weapon, targetInstance, &typedDR);
					}

					REX::DEBUG("HookedDoHitMe: damageType[{}] typedDR after perks={:.2f}", i, typedDR);

					float resistedPercentage = CombatFormulas::CalcResistedPercentage(avInfo, baseDmg, typedDR);
					float contribution = (baseDmg / static_cast<float>(projCount)) * resistedPercentage * bpmMult;

					REX::DEBUG("HookedDoHitMe: damageType[{}] resistedPercentage={:.4f} contribution before incoming perks={:.2f}",
						i, resistedPercentage, contribution);

					if (aggressor)
					{
						BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModIncomingDamage, targetActor, a_hitData->weapon, targetInstance, &contribution);
					}

					REX::DEBUG("HookedDoHitMe: damageType[{}] contribution after incoming perks={:.2f} healthDamage running total={:.2f}",
						i, contribution, a_hitData->healthDamage + contribution);

					a_hitData->healthDamage += contribution;
				}
			}

			const auto damageResist = RE::ActorValue::GetSingleton()->damageResistance;
			float physicalDR = targetActor->GetActorValue(*damageResist);

			REX::DEBUG("HookedDoHitMe: physical -- physicalDR before perks={:.2f}", physicalDR);

			if (aggressor)
			{
				BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModTargetDamageResistance, aggressor, a_hitData->weapon, targetInstance, &physicalDR);
			}

			const float physicalDamage = a_hitData->resistedPhysicalDamage + a_hitData->physicalDamage;
			const float physicalResistPercentage = CombatFormulas::CalcResistedPercentage(damageResist, physicalDamage * static_cast<float>(projCount), physicalDR);
			const float physicalContribution = (physicalDamage * bpmMult) * physicalResistPercentage;

			REX::DEBUG("HookedDoHitMe: physical -- physicalDR after perks={:.2f} physicalDamage={:.2f} "
				"physResistPct={:.4f} physContribution={:.2f}",
				physicalDR, physicalDamage, physicalResistPercentage, physicalContribution);

			a_hitData->healthDamage += physicalContribution;

			float vatsMult = 1.0f;
			if (UI::GetSingleton()->GetMenuOpen("VATSMenu"))
			{
				vatsMult = GameSettingCollection::GetSingleton()->GetSetting("fVATSPlayerDamageMult")->GetFloat();
			}

			PlayerCharacter* player = PlayerCharacter::GetSingleton();
			if (targetActor == player && !a_hitData->flags.any(HitData::Flag::kPredictBaseDamage))
			{
				REX::DEBUG("HookedDoHitMe: applying VATS mult {:.2f} to player hit", vatsMult);
				a_hitData->healthDamage *= vatsMult;
				a_hitData->totalDamage *= vatsMult;
				a_hitData->physicalDamage *= vatsMult;
				a_hitData->targetedLimbDamage *= vatsMult;
				a_hitData->resistedPhysicalDamage *= vatsMult;
				a_hitData->resistedTypedDamage *= vatsMult;
			}

			REX::DEBUG("HookedDoHitMe: after -- healthDamage={:.2f} totalDamage={:.2f} physicalDamage={:.2f} "
				"resistedPhysicalDamage={:.2f} resistedTypedDamage={:.2f}",
				a_hitData->healthDamage,
				a_hitData->totalDamage,
				a_hitData->physicalDamage,
				a_hitData->resistedPhysicalDamage,
				a_hitData->resistedTypedDamage);

			a_this->DoHitMe(a_hitData);
		}

		void Install()
		{
			auto& trampoline = REL::GetTrampoline();

			// Shared stub for a few patches.
			LockLevels::g_lockLevelNamesStub = reinterpret_cast<Setting**>(trampoline.allocate(sizeof(Setting*) * 10));
			std::uintptr_t stubAddr = reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub);

			// TaskQueueInterface::TaskUnpackFunc - { ID 2229323 + 0x8F7 }
			typedef void(DoHitMe_Sig)(Actor*, HitData*);
			REL::Relocation<DoHitMe_Sig> DoHitMe_Location{ ID::TaskQueueInterface::TaskUnpackFunc, 0x8F7 };
			trampoline.write_call<5>(DoHitMe_Location.address(), &HookTaskQueueInterfaceTaskUnpackFunc_DoHitMe);

			// GetInfoForPlayerDialogueOptionHook - { ID 2196817 + 0x40A }
			typedef TESTopicInfo(GetCurrentTopicInfo_Player_Sig)(BGSSceneActionPlayerDialogue* apPlayerDialogue, BGSScene* apParentScene, TESObjectREFR* apTarget, std::uint32_t aeType);
			REL::Relocation<GetCurrentTopicInfo_Player_Sig> GetCurrentTopicInfo_Player_Location{ ID::BGSSceneActionNPCResponseDialogue::UpdateAction1, 0x40A };
			trampoline.write_call<5>(GetCurrentTopicInfo_Player_Location.address(), &Dialogue::GetCurrentTopicInfo_Player_Hook);

			// GetInfoForNPCResponseDialogOptionHook - { ID 2196817 + 0x7A1 }
			typedef TESTopicInfo(GetCurrentTopicInfo_NPC_Sig)(BGSSceneActionPlayerDialogue* apPlayerDialogue, BGSScene* apParentScene, TESObjectREFR* apTarget, std::uint32_t aeType);
			REL::Relocation<GetCurrentTopicInfo_NPC_Sig> GetCurrentTopicInfo_NPC_Location{ ID::BGSSceneActionNPCResponseDialogue::UpdateAction1, 0x7A1 };
			trampoline.write_call<5>(GetCurrentTopicInfo_NPC_Location.address(), &Dialogue::GetCurrentTopicInfo_NPC_Hook);

			// GetNPCResponseInfoForOptionHook - { ID 2196800 + 0x388 }
			typedef TESTopicInfo(GetCurrentTopicInfo_NPCAction_Sig)(BGSSceneActionNPCResponseDialogue* apNPCDialogue, BGSScene* apParentScene);
			REL::Relocation<GetCurrentTopicInfo_NPCAction_Sig> GetCurrentTopicInfo_NPCAction_Location{ ID::BGSSceneActionNPCResponseDialogue::UpdateAction2, 0x388 };
			trampoline.write_call<5>(GetCurrentTopicInfo_NPCAction_Location.address(), &Dialogue::GetCurrentTopicInfo_NPCAction_Hook);

			// PipboyInventoryData::PopulateItemCardInfo - { ID 2225264 + 0x651 }
			typedef void(PopulateItemCardInfo1_Sig)(PipboyInventoryData* a_pipboyInventoryData, const BGSInventoryItem* a_inventoryItem, const BGSInventoryItem::Stack* a_stack, PipboyObject* a_data);
			REL::Relocation<PopulateItemCardInfo1_Sig> PopulateItemCardInfo1_Location{ ID::PipboyInventoryData::InitializeItem, 0x651 };
			trampoline.write_jmp<5>(PopulateItemCardInfo1_Location.address(), &HookPipboyDataPopulateItemCardInfo);

			// PipboyInventoryData::PopulateItemCardInfo - { ID 2225279 + 0x40F }
			typedef void(PopulateItemCardInfo2_Sig)(PipboyInventoryData* a_pipboyInventoryData, const BGSInventoryItem* a_inventoryItem, const BGSInventoryItem::Stack* a_stack, PipboyObject* a_data);
			REL::Relocation<PopulateItemCardInfo2_Sig> PopulateItemCardInfo2_Location{ ID::PipboyInventoryData::RepopulateItemCardOnSection, 0x40F };
			trampoline.write_call<5>(PopulateItemCardInfo2_Location.address(), &HookPipboyDataPopulateItemCardInfo);

			// InventoryUserUIUtils::PopulateItemCardInfo_Helper - { ID 2222625 + 0x1226 }
			REL::Relocation<std::uintptr_t> InventoryUserUIUtilsPopulateItemCardInfo_Helper_NOP{ ID::InventoryUserUIUtils::PopulateItemCardInfo_Helper, 0x1226 };
			InventoryUserUIUtilsPopulateItemCardInfo_Helper_NOP.write_fill(REL::NOP, 5);

			// PipboyInventoryData::PopulateItemCardInfo - { 2225266 + 0x8CD }
			REL::Relocation<std::uintptr_t> PipboyInventoryDataPopulateItemCardInfo_NOP{ ID::PipboyInventoryData::PopulateItemCardInfo, 0x8CD };
			PipboyInventoryDataPopulateItemCardInfo_NOP.write_fill(REL::NOP, 5);

			// HUDExperienceMeter::UpdateDisplayObject - { 2220382 + 0x221 }
			REL::Relocation<std::uintptr_t> HUDExperienceMeterUpdateDisplayObject_NOP_1{ ID::HUDExperienceMeter::UpdateDisplayObject, 0x221 };
			HUDExperienceMeterUpdateDisplayObject_NOP_1.write_fill(REL::NOP, 5);

			// HUDExperienceMeter::UpdateDisplayObject - { 2220382 + 0x26B }
			REL::Relocation<std::uintptr_t> HUDExperienceMeterUpdateDisplayObject_NOP_2{ ID::HUDExperienceMeter::UpdateDisplayObject, 0x26B };
			HUDExperienceMeterUpdateDisplayObject_NOP_2.write_fill(REL::NOP, 5);

			// Actor::GetDesirability - { 2229946 + 0x56 }
			REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_1{ ID::Actor::GetDesirability, 0x56 };
			trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_1.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

			// Actor::CalcArmorRating - { 2230008 + 0xFC }
			REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_2{ ID::Actor::CalcArmorRating1, 0xFC };
			trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_2.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

			// Actor::CalcArmorRating - { 2230009 + 0x2B }
			REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_3{ ID::Actor::CalcArmorRating2, 0x2B };
			trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_3.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

			// sub_140BFBE70 - { 2230010 + 0x13C } - inlined in .980? 
			REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_4{ REL::ID(2230010), 0x13C };
			trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_4.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

			// CombatBehaviourFindObject::EvaluateArmor { 2241004 + 0x4ED }
			REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_5{ ID::CombatBehaviorFindObject::EvaluateArmor, 0x4ED };
			trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_5.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

			// CombatBehaviourFindObject::EvaluateArmor { 2241004 + 0x579 }
			REL::Relocation<std::uintptr_t> ActorUtilsArmorRatingVisitorBaseoperator_6{ ID::CombatBehaviorFindObject::EvaluateArmor, 0x579 };
			trampoline.write_call<5>(ActorUtilsArmorRatingVisitorBaseoperator_6.address(), &HookActorUtilsArmorRatingVisitorBaseOperator);

			// FavoritesManager::UseQuickkeyItem - { ID 2248740 + 0x5B }
			REL::Relocation<std::uintptr_t> FavoritesManagerUseQuickkeyItem_1{ ID::FavoritesManager::HandleEvent, 0x5B };
			trampoline.write_call<5>(FavoritesManagerUseQuickkeyItem_1.address(), &HookFavoritesManagerUseQuickkeyItem);

			// FavoritesManager::UseQuickkeyItem - { ID 2248766 + 0x68 }
			REL::Relocation<std::uintptr_t> FavoritesManagerUseQuickkeyItem_2{ ID::FavoritesManager::Call, 0x68 };
			trampoline.write_call<5>(FavoritesManagerUseQuickkeyItem_2.address(), &HookFavoritesManagerUseQuickkeyItem);

			// WorkbenchMenuBase::ShowBuildFailureMessage { ID 2224322 + 0x2C5 }
			REL::Relocation<std::uintptr_t> WorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu{ ID::PowerArmorModMenu::ShowBuildFailureMessage, 0x2C5 };
			trampoline.write_jmp<5>(WorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu.address(), &HookWorkbenchMenuBaseShowBuildFailureMessage_PowerArmorModMenu);

			// PlayerCharacter::TryUnlockObject { 2233040 + 0x186 }
			REL::Relocation<std::uintptr_t> PlayerCharacterTryUnlockObject1{ ID::PlayerCharacter::TryUnlockObject, 0x186 };
			PlayerCharacterTryUnlockObject1.write_fill(REL::NOP, 5);

			// PlayerCharacter::TryUnlockObject { 2233040 + 0x9A }
			REL::Relocation<std::uintptr_t> PlayerCharacterTryUnlockObject2{ ID::PlayerCharacter::TryUnlockObject, 0x9A };
			PlayerCharacterTryUnlockObject2.write<0>(static_cast<std::uint8_t>(0x05));

			// BGSTerminal::Activate { 2197778 + 0x423 }
			REL::Relocation<std::uintptr_t> BGSTerminalActivate{ ID::BGSTerminal::Activate, 0x423 };
			trampoline.write_call<5>(BGSTerminalActivate.address(), &HookBGSTerminalActivate_ShowHUDMessage);

			// AiFormulas::GetBarterValue { 2208969 + 0xC8 }
			REL::Relocation<std::uintptr_t> AiFormulasGetBarterValue_HandleEntryPoint{ ID::AIFormulas::GetBarterValue, 0xC8 };
			trampoline.write_call<5>(AiFormulasGetBarterValue_HandleEntryPoint.address(), &HookAIFormulasGetBarterValue_HandleEntryPoint);

			// GamePlayFormulas::CalculateItemValue { 2209074 + 0xF4 }
			REL::Relocation<std::uintptr_t> GamePlayFormulasCalculateItemValue_GetBarterValue{ ID::GamePlayFormulas::CalculateItemValue, 0xF4 };
			trampoline.write_call<5>(GamePlayFormulasCalculateItemValue_GetBarterValue.address(), &HookGamePlayFormulasCalculateItemValue_GetBarterValue);
			
			// Actor::UpdateSprinting { 2230498 + 0x1C4 }
			REL::Relocation<std::uintptr_t> ActorUpdateSprinting_CalcSprintingActionPoints{ ID::Actor::UpdateSprinting, 0x1C4 };
			trampoline.write_call<5>(ActorUpdateSprinting_CalcSprintingActionPoints.address(), &HookActorUpdateSprinting_CalcSprintingActionPoints);
			
			// Actor::Jump { 2229650 + 0x17 }
			REL::Relocation<std::uintptr_t> ActorJump_GetMobilityCrippled{ ID::Actor::Jump, 0x17 };
			trampoline.write_call<5>(ActorJump_GetMobilityCrippled.address(), &HookActorJump_GetMobilityCrippled);

			// Actor::Jump { 2229650 + 0x1D5 }
			REL::Relocation<std::uintptr_t> ActorJump_bhkCharacterControllerJump{ ID::Actor::Jump, 0x1D5 };
			trampoline.write_call<5>(ActorJump_bhkCharacterControllerJump.address(), &HookActorJump_bhkCharacterControllerJump);

			// Lockpicking 
			// Index has been shifted up 1 by default for retail entries, gotta take that into account, turns out it's a few places.
			
			// LockpickingMenu::OpenLockpickingMenu { 2249263 + 0x87 }
			REL::Relocation<std::uintptr_t> LockLevelUpperBoundCheck{ ID::LockpickingMenu::OpenLockpickingMenu, 0x87 };
			LockLevelUpperBoundCheck.write<0>(static_cast<std::uint8_t>(0x05));

			// BGSTerminal::GetHackDifficultyLockLevel { 2197777 + 0x5B }
			REL::Relocation<std::uintptr_t> HackDifficultyLockLevelUpperBoundCheck{ ID::BGSTerminal::GetHackDifficultyLockLevel, 0x5B };
			HackDifficultyLockLevelUpperBoundCheck.write<0>(static_cast<std::uint32_t>(0x00000004));

			// BGSTerminal::Activate1 { 2197778 + 0x2B0 }
			REL::Relocation<std::uintptr_t> BGSTerminalActivate1{ ID::BGSTerminal::Activate, 0x2B0 };
			BGSTerminalActivate1.write<0>(static_cast<std::uint32_t>(0x00000004));

			// BGSTerminal::Activate2 { 2197778 + 0x2F1 }
			REL::Relocation<std::uintptr_t> BGSTerminalActivate2{ ID::BGSTerminal::Activate, 0x2F1 };
			BGSTerminalActivate2.write<0>(static_cast<std::uint8_t>(0xFB));

			// BGSTerminal::Activate3 { 2197778 + 0x313 }
			REL::Relocation<std::uintptr_t> BGSTerminalActivate3{ ID::BGSTerminal::Activate, 0x313 };
			BGSTerminalActivate3.write<0>(static_cast<std::uint8_t>(0x06));

			// BGSTerminal::Activate4 { 2197778 + 0x387 }
			REL::Relocation<std::uintptr_t> BGSTerminalActivate4{ ID::BGSTerminal::Activate, 0x387 };
			BGSTerminalActivate4.write<0>(static_cast<std::uint8_t>(0x06));

			// BGSTerminal::Activate5 { 2197778 + 0x3DA }
			REL::Relocation<std::uintptr_t> BGSTerminalActivate5{ ID::BGSTerminal::Activate, 0x3DA };
			BGSTerminalActivate5.write<0>(static_cast<std::uint8_t>(0x05));

			// sub_1404435E0 { 2197829 + 0x87 }
			REL::Relocation<std::uintptr_t> sub_1404435E0{REL::ID(2197829), 0x87};
			sub_1404435E0.write<0>(static_cast<std::uint8_t>(0x05));

			// sub_140443960 { 2197832 + 0x155 }
			REL::Relocation<std::uintptr_t> sub_140443960{REL::ID(2197832), 0x155};
			sub_140443960.write<0>(static_cast<std::uint8_t>(0x05));

			// TESObjectCONT::GetActivateText { 2198653 + 0x7B }
			REL::Relocation<std::uintptr_t> TESObjectCONTGetActivateText1{ID::TESObjectCONT::GetActivateText, 0x7B};
			TESObjectCONTGetActivateText1.write<0>(static_cast<std::uint8_t>(0x0A));

			// TESObjectCONT::GetActivateText { 2198653 + 0x321 }
			REL::Relocation<std::uintptr_t> TESObjectCONTGetActivateText2{ ID::TESObjectCONT::GetActivateText, 0x321 };
			std::int32_t disp1 = static_cast<std::int32_t>(stubAddr - (TESObjectCONTGetActivateText2.address() + 7));
			TESObjectCONTGetActivateText2.write<3>(disp1);

			// Console::DisplayRef { 2248550 + 0x58F }
			REL::Relocation<std::uintptr_t> ConsoleDisplayRefLEA{ ID::Console::DisplayRef, 0x58F };
			std::int32_t disp2 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub) - (ConsoleDisplayRefLEA.address() + 7));
			ConsoleDisplayRefLEA.write<3>(disp2);

			// LockpickingMenu::SendLockInfoToMenu { 2249267 + 0x1E }
			REL::Relocation<std::uintptr_t> LockpickingMenuSendLockInfoLEA{ ID::LockpickingMenu::SendLockInfoToMenu, 0x1E };
			std::int32_t disp3 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub) - (LockpickingMenuSendLockInfoLEA.address() + 7));
			LockpickingMenuSendLockInfoLEA.write<3>(disp3);

			// sub_14044CC90 { 2198055 + 0x91 }
			REL::Relocation<std::uintptr_t> sub_14044CC90{ REL::ID(2198055), 0x91 };
			sub_14044CC90.write<0>(static_cast<std::uint8_t>(0x06));

			// sub_14044CC90 { 2198055 + 0xE4 }
			REL::Relocation<std::uintptr_t> sub_14044CC90_LEA{ REL::ID(2198055), 0xE4 };
			std::int32_t disp4 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub) - (sub_14044CC90_LEA.address() + 7));
			sub_14044CC90_LEA.write<3>(disp4);

			// TESObjectDOORGetActivateText1 { 2198690 + 0x101 }
			REL::Relocation<std::uintptr_t> TESObjectDOORGetActivateText1{ID::TESObjectDOOR::GetActivateText, 0x101};
			TESObjectDOORGetActivateText1.write<0>(static_cast<std::uint8_t>(0x0A));

			// TESObjectDOOR::GetActivateText LEA patches
			REL::Relocation<std::uintptr_t> TESObjectDOORGetActivateText_LEA1{ ID::TESObjectDOOR::GetActivateText, 0x17A };
			std::int32_t doorDisp1 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub) - (TESObjectDOORGetActivateText_LEA1.address() + 7));
			TESObjectDOORGetActivateText_LEA1.write<3>(doorDisp1);

			REL::Relocation<std::uintptr_t> TESObjectDOORGetActivateText_LEA2{ ID::TESObjectDOOR::GetActivateText, 0x1DC };
			std::int32_t doorDisp2 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub) - (TESObjectDOORGetActivateText_LEA2.address() + 7));
			TESObjectDOORGetActivateText_LEA2.write<3>(doorDisp2);

			REL::Relocation<std::uintptr_t> TESObjectDOORGetActivateText_LEA3{ ID::TESObjectDOOR::GetActivateText, 0x27B };
			std::int32_t doorDisp3 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub) - (TESObjectDOORGetActivateText_LEA3.address() + 7));
			TESObjectDOORGetActivateText_LEA3.write<3>(doorDisp3);

			REL::Relocation<std::uintptr_t> TESObjectDOORGetActivateText_LEA4{ ID::TESObjectDOOR::GetActivateText, 0x2D8 };
			std::int32_t doorDisp4 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(LockLevels::g_lockLevelNamesStub) - (TESObjectDOORGetActivateText_LEA4.address() + 7));
			TESObjectDOORGetActivateText_LEA4.write<3>(doorDisp4);

			// TESObjectDOOR::CalculateDoFavor { 2198692 + 0x108 }
			REL::Relocation<std::uintptr_t> TESObjectDOORCalculateDoFavor{ ID::TESObjectDOOR::CalculateDoFavor, 0x108 };
			TESObjectDOORCalculateDoFavor.write<0>(static_cast<std::uint8_t>(0x05));
		}

		DetourXS hook_ShowBuildFailureMessage;
		typedef void(ShowBuildFailureMessageSig)(WorkbenchMenuBase*);

		void HookWorkbenchMenuBaseShowBuildFailureMessage(WorkbenchMenuBase* a_this)
		{
			if (a_this->repairing)
			{
				Scaleform::Ptr<RE::ExamineMenu> examineMenu = UI::GetSingleton()->GetMenu<RE::ExamineMenu>();

				std::uint32_t selectedIndex = examineMenu->GetSelectedIndex();
				if (!examineMenu->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < examineMenu->invInterface.stackedEntries.size())
				{
					InventoryUserUIInterfaceEntry* inventoryUUIEntry = (examineMenu->invInterface.stackedEntries.data() + selectedIndex);
					const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

					if (inventoryItem)
					{
						BSTArray<BSTTuple<TESForm*, BGSTypedFormValuePair::SharedVal>>* requiredItems = a_this->QCurrentModChoiceData()->recipe->requiredItems;
						if (requiredItems)
						{
							RE::RepairFailureCallback* repairFailureCallback = new RE::RepairFailureCallback(examineMenu.get());
							RE::ExamineConfirmMenu::InitDataRepairFailure* initDataRepair = new RE::ExamineConfirmMenu::InitDataRepairFailure(requiredItems);

							BSTHashMap<TESBoundObject*, std::uint32_t> availableComponents;

							BSTTuple<TESBoundObject*, std::uint32_t> insertObject;

							std::uint32_t size = requiredItems->size();
							if (size)
							{
								for (std::uint32_t i = 0; i < size; ++i) {
									const auto& tuple = requiredItems->at(i);
									TESForm* form = tuple.first;
									const auto& value = tuple.second;

									if (!form) continue;

									if (!form->IsBoundObject()) continue;

									TESObjectREFR* sharedContainerREF = examineMenu.get()->sharedContainerRef.get();
									BGSInventoryList* inventoryList = sharedContainerREF->inventoryList;

									if (inventoryList)
									{
										TESBoundObject* object = reinterpret_cast<TESBoundObject*>(form);
										std::uint32_t availableCount = Shared::GetAvailableComponentCount(inventoryList, form);
										insertObject = { object, availableCount };
									}
									else
									{
										BaseFormComponent* container = sharedContainerREF->HasContainer();
										if (container)
										{
											// TODO - handling needed here.
											REX::DEBUG("'sharedContainerREF' has container - MISSING HANDLING");
										}
									}

									availableComponents.insert(insertObject);										
								}
								initDataRepair->availableComponents = availableComponents;
							}

							examineMenu->ShowConfirmMenu(initDataRepair, repairFailureCallback);
						}
						a_this->repairing = false;
					}
				}
			}
			else
			{
				GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
				SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sCannotBuildMessage")->GetString().data(), nullptr, true, true);
			}
		}

		DetourXS hook_GetBuildConfirmQuestion;
		typedef void(GetBuildConfirmQuestionSig)(RE::ExamineMenu*, char*, std::uint32_t);

		void HookExamineMenuGetBuildConfirmQuestion(RE::ExamineMenu* a_this, char* a_buffer, std::uint32_t a_bufferLength)
		{
			const WorkbenchMenuBase::ModChoiceData* modChoiceData;
			const char* fullName;
			const char* type;

			if (a_this->QCurrentModChoiceData()->recipe)
			{
				modChoiceData = a_this->QCurrentModChoiceData();
				if (a_this->repairing)
				{
					type = "$$Repair";
					std::uint32_t selectedIndex = a_this->GetSelectedIndex();
					if (!a_this->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < a_this->invInterface.stackedEntries.size())
					{
						InventoryUserUIInterfaceEntry* inventoryUUIEntry = (a_this->invInterface.stackedEntries.data() + selectedIndex);
						const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

						fullName = inventoryItem->GetDisplayFullName(inventoryUUIEntry->stackIndex.at(0));
					}
				}
				else
				{
					type = "$$Make";
					fullName = TESFullName::GetFullName(*modChoiceData->recipe->createdItem).data();
				}
				snprintf(a_buffer, a_bufferLength, "%s %s?", type, fullName);
			}
		}

		DetourXS hook_SetHealthPerc;
		typedef void(SetHealthPercSig)(ExtraDataList*, float);
		REL::Relocation<SetHealthPercSig> SetHealthPercOriginal;

		// Required to allow the game to store a value of 1.0, by default it would set it to '-1.0'
		// This causes issues in the way we handle initialization of the health data.
		// Here we simply, run the original function, then if 'a_health' was '1.0', we manually set it 
		// again to '1.0', reverting the original functions result of '-1.0'.
		void HookExtraDataListSetHealthPerc(ExtraDataList* a_this, float a_health)
		{
			SetHealthPercOriginal(a_this, a_health);
			if (a_health == 1.0f && a_this->GetHealthPerc() != 1.0f)
			{
				a_this->AddExtra(new ExtraHealth(1.0f));
			}
		}

		DetourXS hook_QCurrentModChoiceData;
		typedef const WorkbenchMenuBase::ModChoiceData* (QCurrentModChoiceDataSig)(WorkbenchMenuBase*);

		const WorkbenchMenuBase::ModChoiceData* HookWorkbenchMenuBaseQCurrentModChoiceData(WorkbenchMenuBase* a_this)
		{
			if (a_this->repairing)
			{
				// TODO: Merge this into singular function returning dynamic repair cost.
				if (typeid(*a_this) == typeid(PowerArmorModMenu))
				{
					PowerArmorModMenu* powerArmorModMenu = dynamic_cast<PowerArmorModMenu*>(a_this);
					return &powerArmorModMenu->repairData;
				}
				else
				{
					std::uint32_t modChoiceIndex = a_this->modChoiceIndex;
					if (modChoiceIndex >= a_this->modChoiceArray.size())
					{
						return 0;
					}
					else
					{
						WorkbenchMenuBase::ModChoiceData* currentModChoiceData = (a_this->modChoiceArray.data() + modChoiceIndex);

						// Remove any possible required perks, as we don't take that into account when repairing.
						if (currentModChoiceData->requiredPerks.size() > 0)
						{
							currentModChoiceData->requiredPerks.clear();
						}

						// Remove any possible conditions on the recipe, as we don't take that into account when repairing.
						currentModChoiceData->recipe->conditions.ClearAllConditionItems();

						return (currentModChoiceData);
					}
				}
			}
			else // Retail logic come into play here.
			{
				std::uint32_t modChoiceIndex = a_this->modChoiceIndex;
				if (modChoiceIndex >= a_this->modChoiceArray.size())
				{
					return 0;
				}
				else
				{
					return (a_this->modChoiceArray.data() + modChoiceIndex);
				}
			}
		}

		DetourXS hook_GetInventoryValue;
		typedef std::int64_t(GetInventoryValueSig)(TESBoundObject*, const ExtraDataList*);
		REL::Relocation<GetInventoryValueSig> GetInventoryValueOriginal;

		std::int64_t HookBGSInventoryItemUtilsGetInventoryValue(TESBoundObject* a_baseObj, const ExtraDataList* a_extra)
		{
			if (!a_extra->HasType(EXTRA_DATA_TYPE::kHealth))
			{
				return GetInventoryValueOriginal(a_baseObj, a_extra);
			}

			ExtraDataList* non_const_a_extra = const_cast<ExtraDataList*>(a_extra);

			std::int64_t newValue;

			// Clamp to 5% value at minimum.
			if (non_const_a_extra->GetHealthPerc() < 0.05f)
			{
				newValue = std::int64_t(GetInventoryValueOriginal(a_baseObj, a_extra) * 0.05f * std::sqrt(0.05f));
			}
			else
			{
				newValue = std::int64_t(GetInventoryValueOriginal(a_baseObj, a_extra) * non_const_a_extra->GetHealthPerc() * std::sqrt(non_const_a_extra->GetHealthPerc()));
			}
			return newValue;
		}

		DetourXS hook_AddItem;
		typedef void(AddItemSig)(BGSInventoryList*, TESBoundObject*, const BGSInventoryItem::Stack*, std::uint32_t*, std::uint32_t*);
		REL::Relocation<AddItemSig> AddItemOriginal;

		// All health initialization occurs here, this triggers both when a NPC is spawned with objects, added to inventories through
		// a command or any other way, items also stay uninitialized until picked up saving a 'float' per object from player saves until
		// they're picked up and serialized.
		void HookBGSInventoryListAddItem(BGSInventoryList* a_this, TESBoundObject* a_boundObject, const BGSInventoryItem::Stack* a_stack, std::uint32_t* a_oldCount, std::uint32_t* a_newCount)
		{
			ENUM_FORM_ID formType = a_boundObject->GetFormType();
			if (a_boundObject)
			{
				if (formType == ENUM_FORM_ID::kWEAP || formType == ENUM_FORM_ID::kARMO)
				{
					std::uint32_t iterCount = 0;
					for (const BGSInventoryItem::Stack* traverse = a_stack; traverse; traverse->nextStack)
					{
						if (!traverse || !traverse->extra)
						{
							break;
						}

						if (formType == ENUM_FORM_ID::kWEAP)
						{
							TESObjectWEAP* tempREFR = static_cast<TESObjectWEAP*>(a_boundObject);
							if (tempREFR->weaponData.type == WEAPON_TYPE::kGrenade || tempREFR->weaponData.type == WEAPON_TYPE::kMine)
							{
								REX::DEBUG("BGSInventoryList::AddItem: REFR grenade/mine weapon type.");
								break;
							}

							// Set to '1.0' when initializing if the 'noDegradation' keyword is on the object.
							if (tempREFR->HasKeyword(Shared::noDegradation))
							{
								REX::DEBUG("'CAS_NoDegradation' keyword found on weapon: {}.", tempREFR->GetFormEditorID());
								break;
							}
						}

						if (formType == ENUM_FORM_ID::kARMO)
						{
							TESObjectARMO* tempREFR = static_cast<TESObjectARMO*>(a_boundObject);
							// Set to '1.0' when initializing if the 'noDegradation' keyword is on the object.

							if (tempREFR->armorData.rating == 0 && !tempREFR->armorData.damageTypes)
							{
								break;
							}

							if (tempREFR->HasKeyword(Shared::noDegradation))
							{
								REX::DEBUG("'CAS_NoDegradation' keyword found on armor: {}.", tempREFR->GetFormEditorID());
								break;
							}
						}

						// GetHealthPerc returns -1.0 if it can't find the 'kHealth' type.
						if (traverse->extra->GetHealthPerc() < 0.0f)
						{
							traverse->extra->SetHealthPerc(BSRandom::Float(0.45f, 0.85f));
							break;
						}

						iterCount++;

						if (iterCount > traverse->count - 1)
						{
							break;
						}
					}
				}
			}
			AddItemOriginal(a_this, a_boundObject, a_stack, a_oldCount, a_newCount);
		}

		DetourXS hook_ExamineMenuBuildConfirmed;
		typedef void(ExamineMenuBuildConfirmedSig)(ExamineMenu*, bool);
		REL::Relocation<ExamineMenuBuildConfirmedSig> ExamineMenuBuildConfirmedOriginal;

		void HookExamineMenuBuildConfirmed(ExamineMenu* a_this, bool a_ownerIsWorkbench)
		{
			if (a_this->repairing)
			{
				std::uint32_t selectedIndex = a_this->GetSelectedIndex();
				if (!a_this->invInterface.entriesInvalid && (selectedIndex & 0x80000000) == 0 && selectedIndex < a_this->invInterface.stackedEntries.size())
				{
					InventoryUserUIInterfaceEntry* inventoryUUIEntry = (a_this->invInterface.stackedEntries.data() + selectedIndex);
					const BGSInventoryItem* inventoryItem = BGSInventoryInterface::GetSingleton()->RequestInventoryItem(inventoryUUIEntry->invHandle.id);

					if (inventoryItem)
					{
						switch (a_this->GetCurrentObj()->formType.get())
						{
						case ENUM_FORM_ID::kARMO:
						case ENUM_FORM_ID::kWEAP:
							BGSInventoryItem::Stack* stack = inventoryItem->GetStackByID(inventoryUUIEntry->stackIndex.at(0));

							if (stack)
							{
								stack->extra->SetHealthPerc(1.0f);

								BGSInventoryItem::CheckStackIDFunctor compareFunction(inventoryUUIEntry->stackIndex.at(0));
								BGSInventoryItem::SetHealthFunctor writeFunction(stack->extra->GetHealthPerc());
								writeFunction.shouldSplitStacks = 0x101;

								PlayerCharacter::GetSingleton()->FindAndWriteStackDataForInventoryItem(a_this->GetCurrentObj(), compareFunction, writeFunction);

								BGSDefaultObject* craftingSound = TESDataHandler::GetSingleton()->LookupForm<BGSDefaultObject>(0x112622, "Fallout4.esm");

								a_this->ConsumeSelectedItems(true, craftingSound->GetForm<BGSSoundDescriptorForm>());
								a_this->UpdateOptimizedAutoBuildInv();
								selectedIndex = a_this->GetSelectedIndex();
								a_this->UpdateItemList(selectedIndex);
								a_this->uiMovie->Invoke("RefreshList", nullptr, nullptr, 0);
								a_this->CreateModdedInventoryItem();
								a_this->UpdateItemCard(false);
								a_this->repairing = false;
								a_this->uiMovie->Invoke("UpdateButtons", nullptr, nullptr, 0); // TODO - update custom buttons for this functionality.
							}
						}
					}
				}
			}
			else
			{
				ExamineMenuBuildConfirmedOriginal(a_this, a_ownerIsWorkbench);
			}
		}

		DetourXS hook_TESObjectWEAPFire;
		typedef void(TESObjectWEAPFireSig)(const BGSObjectInstanceT<TESObjectWEAP>*, TESObjectREFR*, BGSEquipIndex, TESAmmo*, AlchemyItem*);
		REL::Relocation<TESObjectWEAPFireSig> TESObjectWEAPFireOriginal;

		void HookTESObjectWEAPFire(const BGSObjectInstanceT<TESObjectWEAP>* a_weapon, TESObjectREFR* a_source, BGSEquipIndex a_equipIndex, TESAmmo* a_ammo, AlchemyItem* a_poison)
		{
			// Make sure 'a_source' is valid.
			if (!a_source)
			{
				return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
			}

			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

			// Only extend logic if the source is the player.
			if (a_source != playerCharacter)
			{
				return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
			}

			// God Mod - no degradation.
			if (playerCharacter->IsGodMode() || Shared::noWeaponDegradation)
			{
				return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
			}

			TESObjectWEAP* weapon = (TESObjectWEAP*)a_weapon->object;

			// No degradation to mines/grenades.
			if (weapon->weaponData.type == WEAPON_TYPE::kGrenade || weapon->weaponData.type == WEAPON_TYPE::kMine)
			{
				return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
			}

			// No degradation on objects with 'noDegradation' keyword.
			if (weapon->HasKeyword(Shared::noDegradation))
			{
				return TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);
			}

			TESObjectWEAPFireOriginal(a_weapon, a_source, a_equipIndex, a_ammo, a_poison);

			BGSInventoryItem* inventoryItem = nullptr;;
			TESFormID weaponFormID = a_weapon->object->GetFormID();
			for (BGSInventoryItem& item : playerCharacter->inventoryList->data)
			{
				if (item.object->GetFormID() == weaponFormID)
				{
					inventoryItem = &item;
					break;
				}
			}

			if (inventoryItem)
			{
				EquippedItem& equippedWeapon = playerCharacter->currentProcess->middleHigh->equippedItems[0];

				// If ammo is mapped to a degradation value, override default value of 1%.
				float conditionReduction = 0.01f;
				TESObjectWEAP::InstanceData* data = (TESObjectWEAP::InstanceData*)(a_weapon->instanceData.get());

				TESAmmo* ammoInstance = data->ammo;

				auto it = ammoDegradationMap.find(ammoInstance);
				if (it != ammoDegradationMap.end())
				{
					conditionReduction = it->second;
				}

				if (conditionReduction == 0.01f)
				{
					REX::WARN("TESObjectWEAP::Fire - Ammo type '{}' is not declared in 'ammoDegradationMap'.", ammoInstance->GetFormEditorID());
				}

				// TODO: Rework these numbers after gameplay tests.
				std::uint32_t flags = data->flags.underlying();
				if (flags & std::uint32_t(WEAPON_FLAGS::kAutomatic))
				{
					conditionReduction *= 0.5f;
				}
				else if (flags & std::uint32_t(WEAPON_FLAGS::kBoltAction))
				{
					conditionReduction *= 2.0f;
				}

				// Reduces damage to weapon depending on players relevant weapon skill level.
				// Linear reduction from 0 - 100, 100 resulting in 20% less damage to weapon.
				float gunsSkillValue = 0.0f;
				ActorValueInfo* gunSkill = data->skill;

				if (gunSkill)
				{
					REX::DEBUG("Weapon '{}' set to skill: {}.", a_weapon->object->GetFormEditorID(), gunSkill->GetFormEditorID());
					gunsSkillValue = playerCharacter->GetActorValue(*gunSkill);
				}
				else {
					REX::DEBUG("Weapon '{}' is missing skill value - please correct this in the weapon record in XEdit.", a_weapon->object->GetFormEditorID());
				}
				
				float reductionPercentFromSkill = (gunsSkillValue / 100.0f) * 0.2;
				conditionReduction *= (1.0f - reductionPercentFromSkill);

				ExtraDataList* extraDataList = inventoryItem->stackData->extra.get();

				float currentHealth = extraDataList->GetHealthPerc();
				float newHealth = std::max(currentHealth - conditionReduction, 0.0f);

				if (newHealth == 0.0f)
				{
					ActorEquipManager::GetSingleton()->UnequipItem(playerCharacter, &equippedWeapon, false);
					GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
					SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sWeaponBreak")->GetString().data(), "UIWorkshopModeItemScrapGeneric", true, true);
				}
				extraDataList->SetHealthPerc(newHealth);
				PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardOnSection(ENUM_FORM_ID::kWEAP);
			}
			return;
		}

		DetourXS hook_CombatFormulasCalcWeaponDamage;
		typedef float(CombatFormulasCalcWeaponDamageSig)(const TESForm*, const TESObjectWEAP::InstanceData*, const TESAmmo*, float, float);
		REL::Relocation<CombatFormulasCalcWeaponDamageSig> CombatFormulasCalcWeaponDamageOriginal;

		float HookCombatFormulasCalcWeaponDamage(const TESForm* a_actorForm, const TESObjectWEAP::InstanceData* a_weapon, const TESAmmo* a_ammo, float a_condition, float a_damageMultiplier)
		{
			float retailDamage = CombatFormulasCalcWeaponDamageOriginal(a_actorForm, a_weapon, a_ammo, a_condition, a_damageMultiplier);
			if (a_condition != -1.0f || a_condition < 0.75f)
			{
				retailDamage = retailDamage * (0.5f + std::min((0.5f * a_condition) / 0.75, 0.5));
			}
			return retailDamage;
		}

		DetourXS hook_GetEquippedArmorDamageResistance;
		typedef float(GetEquippedArmorDamageResistanceSig)(Actor*, const ActorValueInfo*);
		REL::Relocation<GetEquippedArmorDamageResistanceSig> GetEquippedArmorDamageResistanceOriginal;

		float HookGetEquippedDamageResistance(Actor* a_actor, const ActorValueInfo* a_info)
		{
			float retailDamageResistance = GetEquippedArmorDamageResistanceOriginal(a_actor, a_info);

			if (a_actor != PlayerCharacter::GetSingleton())
			{
				return retailDamageResistance;
			}
			else
			{
				BGSInventoryList* inventoryList = a_actor->inventoryList;
				inventoryList->rwLock.lock_read();
				std::uint32_t inventoryListSize = inventoryList->data.size();

				for (BGSInventoryItem& item : inventoryList->data)
				{
					if (item.IsEquipped(0))
					{
						TESObjectARMO* armor = static_cast<TESObjectARMO*>(item.object);
						if (item.object && armor->formType == ENUM_FORM_ID::kARMO && armor->Protects(a_info, false))
						{
							// TODO - add in check to handle items with no condition, they should not have a impact at all.
							ExtraDataList* extraDataList = item.stackData->extra.get();
							if (extraDataList->HasType(EXTRA_DATA_TYPE::kHealth))
							{
								float currentHealth = extraDataList->GetHealthPerc();

								if (currentHealth == 0.0f) // Armor is fully broken, return no value.
								{
									inventoryList->rwLock.unlock_read();
									return 0.0f;
								}

								REX::DEBUG("Damage resistance prior to condition modifier: {}", retailDamageResistance);
								float conditionModifier = 0.66f + std::min((0.34f * extraDataList->GetHealthPerc()) / 0.5f, 0.34f);
								retailDamageResistance *= conditionModifier;
								REX::DEBUG("Damage resistance after condition modifier: {}", retailDamageResistance);
							}
							break;
						}
					}
				}

				inventoryList->rwLock.unlock_read();
				return retailDamageResistance;
			}
		}

		DetourXS hook_IUUIIUtilsAddItemCardInfoEntry;
		typedef void(IUUIIUtilsAddItemCardInfoEntrySig)(Scaleform::GFx::Value&, Scaleform::GFx::Value&, const BSFixedStringCS&, Scaleform::GFx::Value&, float, float, float);
		REL::Relocation<IUUIIUtilsAddItemCardInfoEntrySig> IUUIIUtilsAddItemCardInfoEntryOriginal;

		void HookIUUIIUtilsAddItemCardInfoEntry(Scaleform::GFx::Value& a_array, Scaleform::GFx::Value& a_newEntry, const BSFixedStringCS& a_textID, Scaleform::GFx::Value& a_value, float a_difference, float a_totalDamage, float a_compareDamage)
		{
			IUUIIUtilsAddItemCardInfoEntryOriginal(a_array, a_newEntry, a_textID, a_value, a_difference, a_totalDamage, a_compareDamage);

			if (std::string(a_textID.c_str()) == "CND")
			{
				bool showAsPercent = true;
				std::uint32_t precision = 1;

				a_newEntry.SetMember("showAsPercent", showAsPercent);
				a_newEntry.SetMember("precision", precision);
			}
		}

		DetourXS hook_PipboyInventoryDataBaseAddItemCardInfoEntry;
		typedef PipboyObject* (PipboyInventoryDataBaseAddItemCardInfoEntrySig)(PipboyInventoryData*, const BSFixedStringCS*, PipboyArray*);
		REL::Relocation<PipboyInventoryDataBaseAddItemCardInfoEntrySig> PipboyInventoryDataBaseAddItemCardInfoEntryOriginal;

		PipboyObject* HookPipboyInventoryDataBaseAddItemCardInfoEntry(PipboyInventoryData* a_inventory, const BSFixedStringCS* a_textID, PipboyArray* a_array)
		{
			PipboyObject* result = PipboyInventoryDataBaseAddItemCardInfoEntryOriginal(a_inventory, a_textID, a_array);

			if (a_textID->c_str() == "CND")
			{
				PipboyPrimitiveValue<bool>* showAsPercent = new PipboyPrimitiveValue<bool>(true, result);
				PipboyPrimitiveValue<std::uint32_t>* precision = new PipboyPrimitiveValue<std::uint32_t>(1, result);

				const BSFixedString showAsPercentString = "showAsPercent";
				const BSFixedString precisionString = "precision";

				result->AddMember(&showAsPercentString, showAsPercent);
				result->AddMember(&precisionString, precision);
			}

			return result;
		}

		DetourXS hook_IUUIIUtilsPopulateItemCardInfo_Helper;
		typedef void(IUUIIUtilsPopulateItemCardInfo_HelperSig)(Scaleform::GFx::Value*, const BGSInventoryItem*, std::uint32_t, BSScrapArray<BSTTuple<BGSInventoryItem const*, std::uint32_t>>, bool);
		REL::Relocation<IUUIIUtilsPopulateItemCardInfo_HelperSig> IUUIIUtilsPopulateItemCardInfo_HelperOriginal;

		void HookIUUIIUtilsPopulateItemCardInfo_Helper(Scaleform::GFx::Value* a_menuObj, const BGSInventoryItem* a_item, std::uint32_t a_itemStackID, BSScrapArray<BSTTuple<BGSInventoryItem const*, std::uint32_t>> a_comparisonItems, bool a_forceArmorCompare)
		{
			IUUIIUtilsPopulateItemCardInfo_HelperOriginal(a_menuObj, a_item, a_itemStackID, a_comparisonItems, a_forceArmorCompare);

			if (!UI::GetSingleton()->GetMenuOpen("CookingMenu") && a_item->stackData->extra->HasType(EXTRA_DATA_TYPE::kHealth))
			{
				Scaleform::GFx::Value defaultArray;
				
				if (a_menuObj->HasMember("ItemCardInfoList"))
				{
					a_menuObj->GetMember("ItemCardInfoList", &defaultArray);
				}
				else
				{
					defaultArray = *a_menuObj;
				}
				
				float condition = a_item->stackData->extra->GetHealthPerc();
				InventoryUserUIUtils::AddItemCardInfoEntry(defaultArray, "CND", condition * 100.0f);

				std::uint32_t cndIndexPosition = a_item->object->GetFormType() == ENUM_FORM_ID::kWEAP ? 1 : 0;

				Scaleform::GFx::Value cndEntry;
				defaultArray.GetElement(defaultArray.GetArraySize() - 1, &cndEntry);
				for (std::uint32_t i = defaultArray.GetArraySize() - 1; i > cndIndexPosition; --i)
				{
					Scaleform::GFx::Value tempEntry;
					defaultArray.GetElement(i - 1, &tempEntry);
					defaultArray.SetElement(i, tempEntry);
				}

				defaultArray.SetElement(cndIndexPosition, cndEntry);
			}
			return;
		}

		DetourXS hook_PipboyInventoryUtilsFillResistTypeInfo;
		typedef void(PipboyInventoryUtilsFillResistTypeInfoSig)(const BGSInventoryItem*, const BGSInventoryItem::Stack*, BSScrapArray<BSTTuple<std::uint32_t, float>>*, float);
		REL::Relocation<PipboyInventoryUtilsFillResistTypeInfoSig> PipboyInventoryUtilsFillResistTypeInfo_Original;

		void HookPipboyInventoryUtilsFillResistTypeInfo(const BGSInventoryItem* a_item, const BGSInventoryItem::Stack* a_stack, BSScrapArray<BSTTuple<std::uint32_t, float>>* a_resistValuesPerType, float a_scale)
		{
			PipboyInventoryUtilsFillResistTypeInfo_Original(a_item, a_stack, a_resistValuesPerType, a_scale);

			// Return early if need be.
			if (!a_stack->extra || !a_stack->extra->HasType(EXTRA_DATA_TYPE::kHealth))
			{
				return;
			}
			float condition = a_stack->extra->GetHealthPerc();
			for (std::uint32_t type = 0; type < 6; type++)
			{
				float resistance = a_resistValuesPerType->at(type).second;
				if (resistance != 0.0f && type == 0)
				{
					a_resistValuesPerType->at(type).second = resistance * (0.66f + std::min(0.34 * condition / 0.5, 0.34));
				}
			}
		}

		DetourXS hook_ActorUnequipObject;
		typedef void(ActorUnequipObjectSig)(Actor*, TESBoundObject*, ObjectEquipParams*);
		REL::Relocation<ActorUnequipObjectSig> ActorUnequipObject_Original;

		void HookActorUnequipObject(Actor* a_this, TESBoundObject* a_object, ObjectEquipParams* a_params)
		{
			ActorUnequipObject_Original(a_this, a_object, a_params);

			TESDataHandler* tesDataHandler = TESDataHandler::GetSingleton();
			ActorValueInfo* armorPenetrationAV = tesDataHandler->LookupForm<ActorValueInfo>(0x097341, "Fallout4.esm");

			if (a_object->formType == ENUM_FORM_ID::kWEAP)
			{
				if (a_this->GetActorValue(*armorPenetrationAV) > 0.0)
				{
					a_this->SetActorValue(*armorPenetrationAV, 0.0);
				}
			}
		}

		DetourXS hook_CombatFormulasCalcTargetedLimbDamage;
		typedef float(CombatFormulasCalcTargetedLimbDamageSig)(Actor*, const BGSBodyPart*, float, BSTArray<BSTTuple<TESForm*, BGSTypedFormValuePair::SharedVal>, BSTArrayHeapAllocator>*);
		REL::Relocation<CombatFormulasCalcTargetedLimbDamageSig> CombatFormulasCalcTargetedLimbDamage_Original;

		float HookCombatFormulasCalcTargetedLimbDamage(Actor* a_target, const BGSBodyPart* a_bodyPart, float a_physicalDamage, BSTArray<BSTTuple<TESForm*, BGSTypedFormValuePair::SharedVal>, BSTArrayHeapAllocator>* a_damageTypes)
		{
			float retailValue = CombatFormulasCalcTargetedLimbDamage_Original(a_target, a_bodyPart, a_physicalDamage, a_damageTypes);
			
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			if (a_target == playerCharacter) // TODO: Might rework this so enemy armor takes damage as well.
			{

				// Early return.
				if (playerCharacter->IsGodMode() || Shared::noArmorDegradation)
				{
					return retailValue;
				}

				ActorValueInfo* conditionAV = a_bodyPart->data.actorValue;
				float damageResistanceForHitLimb = ActorUtils::GetEquippedArmorDamageResistance(a_target, conditionAV);

				if (a_physicalDamage > damageResistanceForHitLimb)
				{
					BGSInventoryList* inventoryList = a_target->inventoryList;
					inventoryList->rwLock.lock_read();
					std::uint32_t inventoryListSize = inventoryList->data.size();

					for (BGSInventoryItem& item : inventoryList->data)
					{
						if (item.IsEquipped(0))
						{
							TESObjectARMO* armor = static_cast<TESObjectARMO*>(item.object);
							if (item.object && armor->formType == ENUM_FORM_ID::kARMO && armor->Protects(conditionAV, false))
							{

								
								ExtraDataList* extraDataList = item.stackData->extra.get();
								if (extraDataList->HasType(EXTRA_DATA_TYPE::kHealth))
								{
									float currentHealth = extraDataList->GetHealthPerc();

									if (currentHealth > 0.0f)
									{
										REX::DEBUG("Hit item: {}, condition prior to degradation: {}", item.object->GetFormEditorID(), currentHealth);

										float conditionReduction = (a_physicalDamage - damageResistanceForHitLimb) / damageResistanceForHitLimb * 0.0025f; // 0.0025% per percent.
										float newHealth = std::max(currentHealth - conditionReduction, 0.0f);

										if (newHealth == 0.0f)
										{
											TBO_InstanceData instanceData = armor->armorData;
											BGSObjectInstance* armorInstance = new BGSObjectInstance(armor, &instanceData);

											inventoryList->rwLock.unlock_read();
											ActorEquipManager::GetSingleton()->UnequipObject(playerCharacter, armorInstance, 1, armor->equipSlot, 0, false, true, true, true, nullptr);
											inventoryList->rwLock.lock_read();
										
											SendHUDMessage::ShowHUDMessage("$CAS_ArmorBroken", "UIWorkshopModeItemScrapGeneric", true, true);
										}

										REX::DEBUG("Hit item: {}, condition after degradation: {} with a reduction of: {}%", item.object->GetFormEditorID(), newHealth, conditionReduction * 100);

										extraDataList->SetHealthPerc(newHealth);
										PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardOnSection(ENUM_FORM_ID::kARMO);
									}
								}
								break;
							}
						}
					}

					inventoryList->rwLock.unlock_read();
				}
			}
			return retailValue;
		}

		DetourXS hook_LoadingMenuPopulateLoadScreens;
		typedef void(LoadingMenuPopulateLoadScreensSig)(LoadingMenu*);
		REL::Relocation<LoadingMenuPopulateLoadScreensSig> LoadingMenuPopulateLoadScreens_Original;

		void HookLoadingMenuPopulateLoadScreens(LoadingMenu* a_this)
		{
			LoadingMenuPopulateLoadScreens_Original(a_this);
			
			if (Shared::forcedLoadScreen)
			{
				a_this->artScreen = Shared::forcedLoadScreen;
				Shared::forcedLoadScreen = nullptr;
			}
		}

		DetourXS hook_GamePlayFormulasCanPickLockGateCheck;
		typedef bool(GamePlayFormulasCanPickLockGateCheckSig)(LockLevels::LOCK_LEVEL_EXTENDED);
		bool HookGamePlayFormulasCanPickLockGateCheck(LockLevels::LOCK_LEVEL_EXTENDED a_lockLevel)
		{
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			float skillLevelRequired = 0.0f;

			switch (a_lockLevel)
			{
			case LockLevels::LOCK_LEVEL_EXTENDED::kVeryEasy:
				skillLevelRequired = 0.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kEasy:
				skillLevelRequired = 25.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kAverage:
				skillLevelRequired = 50.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kHard:
				skillLevelRequired = 75.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kVeryHard:
				skillLevelRequired = 100.0f;
				break;
			}

			bool returnValue = playerCharacter->GetActorValue(*Skills::CascadiaActorValues.Lockpick) >= skillLevelRequired;

			if (!returnValue)
			{
				GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
				auto formatStr = gameSettingCollection->GetSetting("sAutoLockPickGateFail")->GetString();
				auto returnMessage = std::vformat(formatStr, std::make_format_args(skillLevelRequired));

				SendHUDMessage::ShowHUDMessage(returnMessage.c_str(), nullptr, true, true);
			}
			return returnValue;
		}

		DetourXS hook_GamePlayFormulasCanHackGateCheck;
		typedef bool(GamePlayFormulasCanHackGateCheckSig)(LockLevels::LOCK_LEVEL_EXTENDED);
		bool HookGamePlayFormulasCanHackGateCheck(LockLevels::LOCK_LEVEL_EXTENDED a_lockLevel)
		{
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			float skillLevelRequired = 0.0f;

			switch (a_lockLevel)
			{
			case LockLevels::LOCK_LEVEL_EXTENDED::kVeryEasy:
				skillLevelRequired = 0.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kEasy:
				skillLevelRequired = 25.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kAverage:
				skillLevelRequired = 50.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kHard:
				skillLevelRequired = 75.0f;
				break;

			case LockLevels::LOCK_LEVEL_EXTENDED::kVeryHard:
				skillLevelRequired = 100.0f;
				break;
			}

			bool returnValue = playerCharacter->GetActorValue(*Skills::CascadiaActorValues.Science) >= skillLevelRequired;

			if (!returnValue)
			{
				GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
				auto formatStr = gameSettingCollection->GetSetting("sHackingGateFail")->GetString();
				auto returnMessage = std::vformat(formatStr, std::make_format_args(skillLevelRequired));

				SendHUDMessage::ShowHUDMessage(returnMessage.c_str(), nullptr, true, true);
			}
			return returnValue;
		}

		DetourXS hook_ActorSPECIALModifiedCallback;
		typedef void(ActorSPECIALModifiedCallbackSig)(Actor*, const ActorValueInfo*, float, float);
		REL::Relocation<ActorSPECIALModifiedCallbackSig> ActorSPECIALModifiedCallback_Original;

		void HookActorSPECIALModifiedCallback(Actor* a_this, const ActorValueInfo* a_info, float a_originalValue, float a_delta)
		{
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

			ActorSPECIALModifiedCallback_Original(a_this, a_info, a_originalValue, a_delta);

			REX::DEBUG("Special Modified Callback - AV: {}", a_info->GetFormEditorID());


			if (a_this == playerCharacter)
			{
				bool boost = true;

				if (a_delta < 0.0f)
				{
					boost = false;
				}

				bool temporary = true;

				float baseValue = playerCharacter->GetBaseActorValue(*a_info);
				REX::DEBUG("'{}' base value: {}", a_info->GetFormEditorID(), baseValue);
				float modValue = playerCharacter->GetActorValue(*a_info);
				REX::DEBUG("'{}' mod value: {}", a_info->GetFormEditorID(), modValue);
				if (baseValue == modValue)
				{
					temporary = false;
				}
				REX::DEBUG("Temporary modification: {}", temporary);

				ActorValue* actorValueSingleton = ActorValue::GetSingleton();
				bool isLuck = a_info == actorValueSingleton->luck;

				if (isLuck)
				{

				}
				else
				{
					auto skillsIt = Skills::specialToSkillsMap.find(a_info);
					if (skillsIt != Skills::specialToSkillsMap.end()) {
						for (auto* skill : skillsIt->second) {
							REX::DEBUG("Skill found for {}: {}", a_info->GetFormEditorID(), skill->GetFormEditorID());

							// Correct
							float originalValueFloat = 2.0f + (2.0f * a_originalValue) + (playerCharacter->GetBaseActorValue(*actorValueSingleton->luck) * 0.5f);
							int originalValueInt = static_cast<int>(std::ceil(originalValueFloat));
							REX::DEBUG("Skill, initial S.P.E.C.I.A.L value: {}", originalValueInt);

							// Correct
							float newValueFloat = 2.0f + (2.0f * playerCharacter->GetActorValue(*a_info)) + (playerCharacter->GetActorValue(*actorValueSingleton->luck) * 0.5f);
							int newValueInt = static_cast<int>(std::ceil(newValueFloat));
							REX::DEBUG("Skill, new S.P.E.C.I.A.L value: {}", newValueInt);

							const int delta = newValueInt - originalValueInt;
							REX::DEBUG("Delta of skill values (initial/new): {}", delta);

							// Correct
							float currentSkillLevelWithoutInitialValue = (playerCharacter->GetBaseActorValue(*skill) - originalValueFloat) - 0.5f; // Needs 0.5f reduction for math reasons.
							REX::DEBUG("Current skill level without S.P.E.C.I.A.L modification: {}", currentSkillLevelWithoutInitialValue);							


							float currentBaseSkillValue = playerCharacter->GetBaseActorValue(*skill);
							REX::DEBUG("Current base skill value: {}", currentBaseSkillValue);
							float currentModSkillValue = playerCharacter->GetActorValue(*skill);
							REX::DEBUG("Current mod skill value: {}", currentModSkillValue);
							
							
							bool skillModifiedByReduction = false;
							if (currentBaseSkillValue > currentModSkillValue)
							{
								skillModifiedByReduction = true;
							}
							if (skillModifiedByReduction)
							{
								REX::DEBUG("Skill is reduced by mod by: {} points.", (currentBaseSkillValue - currentModSkillValue));

								// We make the assumption that a S.P.E.C.I.A.L can only ever be reduced temporarily, 
								// this is the only limiting factor we have at play here.
								if ((currentBaseSkillValue - currentModSkillValue) >= delta && !temporary)
								{
									REX::DEBUG("Current reduction is bigger than, or equals to the newly calculated delta of: {} points.", delta);
									playerCharacter->ModActorValue(ACTOR_VALUE_MODIFIER::kTemporary, *skill, delta);
								}
								return;

							}

							if (temporary)
							{
								playerCharacter->SetActorValue(*skill, originalValueInt + currentSkillLevelWithoutInitialValue);
								playerCharacter->ModActorValue(ACTOR_VALUE_MODIFIER::kTemporary, *skill, delta);
							}
							else
							{
								playerCharacter->SetActorValue(*skill, newValueInt + currentSkillLevelWithoutInitialValue);
							}
						}
					}
				}
			}
		}

		DetourXS hook_nsHUDTypesNotificationData_ctor;
		typedef void(nsHUDTypesNotificationData_ctorSig)(nsHUDTypes::NotificationInfo*, const HUDNotificationEvent*);
		REL::Relocation<nsHUDTypesNotificationData_ctorSig> nsHUDTypesNotificationData_ctor_Original;

		void HooknsHUDTypesNotificationData_ctor(nsHUDTypes::NotificationInfo* a_this, const HUDNotificationEvent* a_event)
		{
			nsHUDTypesNotificationData_ctor_Original(a_this, a_event);

			REX::DEBUG("Audio string: {}", a_this->soundName.c_str());
			a_this->soundName = BSFixedStringCS("MUSDiscoveryBrotherhood");
		}

		DetourXS hook_REFR_LOCKIsInaccessible;
		typedef bool(REFR_LOCKIsInaccessibleSig)(std::uint32_t);

		bool HookREFR_LOCKIsInaccessible(std::uint32_t a_lockLevel)
		{
			return ((a_lockLevel - 6) & 0xFFFFFFFC) == 0 && a_lockLevel != 7;
		}

		DetourXS hook_GamePlayFormulasGetLockXPReward;
		typedef float (GamePlayFormulasGetLockXPRewardSig)(LockLevels::LOCK_LEVEL_EXTENDED);

		float HookGamePlayFormulasGetLockXPReward(LockLevels::LOCK_LEVEL_EXTENDED a_lockLevel)
		{
			GameSettingCollection* settings = GameSettingCollection::GetSingleton();
			switch (a_lockLevel) {
			case LockLevels::LOCK_LEVEL_EXTENDED::kVeryEasy:
				return 5.0f;
			case LockLevels::LOCK_LEVEL_EXTENDED::kEasy:
				return settings->GetSetting("fLockpickXPRewardEasy")->GetFloat();
			case LockLevels::LOCK_LEVEL_EXTENDED::kAverage:
				return settings->GetSetting("fLockpickXPRewardAverage")->GetFloat();
			case LockLevels::LOCK_LEVEL_EXTENDED::kHard:
				return settings->GetSetting("fLockpickXPRewardHard")->GetFloat();
			case LockLevels::LOCK_LEVEL_EXTENDED::kVeryHard:
				return settings->GetSetting("fLockpickXPRewardVeryHard")->GetFloat();
			default:
				return 0.0f;
			}
		}

		DetourXS hook_REFR_LOCKNumericValueToEnum;
		typedef LockLevels::LOCK_LEVEL_EXTENDED(REFR_LOCKNumericValueToEnumSig)(std::uint32_t);

		LockLevels::LOCK_LEVEL_EXTENDED HookREFR_LOCKNumericValueToEnum(std::int32_t a_val)
		{
			if (a_val <= 1) {
				return LockLevels::LOCK_LEVEL_EXTENDED::kVeryEasy;
			}

			if (a_val <= LockLevels::GetLockLevelMaxEasy())
			{
				return LockLevels::LOCK_LEVEL_EXTENDED::kEasy;
			}

			if (a_val <= LockLevels::GetLockLevelMaxAverage())
			{
				return LockLevels::LOCK_LEVEL_EXTENDED::kAverage;
			}

			if (a_val <= LockLevels::GetLockLevelMaxHard())
			{
				return LockLevels::LOCK_LEVEL_EXTENDED::kHard;
			}

			if (a_val <= LockLevels::GetLockLevelMaxVeryHard())
			{
				return LockLevels::LOCK_LEVEL_EXTENDED::kVeryHard;
			}

			if (a_val <= LockLevels::GetLockLevelMaxBarred())
			{
				return LockLevels::LOCK_LEVEL_EXTENDED::kBarred;
			}

			if (a_val <= LockLevels::GetLockLevelMaxChained())
			{
				return LockLevels::LOCK_LEVEL_EXTENDED::kChained;
			}

			// Special locks.
			if (a_val == 251) return LockLevels::LOCK_LEVEL_EXTENDED::kBarred;
			if (a_val == 252) return LockLevels::LOCK_LEVEL_EXTENDED::kTerminal;
			if (a_val == 253) return LockLevels::LOCK_LEVEL_EXTENDED::kInaccessible;
			if (a_val == 254) return LockLevels::LOCK_LEVEL_EXTENDED::kChained;

			// Default if above all thresholds, which is the same as the highest vanilla lock level.
			return LockLevels::LOCK_LEVEL_EXTENDED::kRequiresKey;
		}

		DetourXS hook_AIProcessGetActorLightLevel;
		typedef float(AIProcessGetActorLightLevelSig)(AIProcess*);

		float HookAIProcessGetActorLightLevel(AIProcess* a_this)
		{
			float lightLevel = 0.0f;

			if (a_this && a_this->high)
			{
				lightLevel = a_this->high->lightLevel;
			}

			// If process is player and pipboy light is on, add 200 to the light level.
			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			if (playerCharacter && playerCharacter->currentProcess == a_this) {
				if (playerCharacter->IsPipboyLightOn())
				{
					return lightLevel += 200.0f;
				}
			}

			return lightLevel;
		}

		DetourXS hook_ActorCalculateDetectionFormula;
		typedef void(ActorCalculateDetectionFormulaSig)(Actor*, Actor*, DetectionData*);
		REL::Relocation<ActorCalculateDetectionFormulaSig> ActorCalculateDetectionFormula_Original;

		void HookActorCalculateDetectionFormula(Actor* a_this, Actor* a_target, DetectionData* a_detectionData)
		{
			if (!ActorCalculateDetectionFormula_Original || !a_this || !a_target || !a_detectionData)
			{
				return;
			}

			ActorCalculateDetectionFormula_Original(a_this, a_target, a_detectionData);

			PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
			if (!playerCharacter || a_target != playerCharacter)
			{
				return;
			}

			const float dx = a_this->data.location.x - a_target->data.location.x;
			const float dy = a_this->data.location.y - a_target->data.location.y;
			const float dz = a_this->data.location.z - a_target->data.location.z;
			const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

			if (RadioManager::QPlayerRadioEnabled() && distance < 2000.0f)
			{
				const float normalizedDistance = std::fminf(1.0f, distance / 2000.0f);
				const float bonus = 98.0f * (1.0f - normalizedDistance) + 2.0f;
				const float soundBonus = std::clamp(bonus, 0.0f, 30000.0f);

				if (a_detectionData->soundDetectionLevel < soundBonus)
				{
					a_detectionData->soundDetectionLevel = soundBonus;
					REX::DEBUG("Radio is on, applying sound bonus of {} to detection level. Distance: {}", soundBonus, distance);
				}
			}

		}

		// ========== REGISTERS ==========

		void RegisterActorCalculateDetectionFormula()
		{
			REL::Relocation<ActorCalculateDetectionFormulaSig> functionLocation{ ID::Actor::CalculateDetectionFormula };
			if (hook_ActorCalculateDetectionFormula.Create(reinterpret_cast<void*>(functionLocation.address()), &HookActorCalculateDetectionFormula))
			{
				REX::DEBUG("Installed 'Actor::CalculateDetectionFormula' hook.");
				ActorCalculateDetectionFormula_Original = reinterpret_cast<uintptr_t>(hook_ActorCalculateDetectionFormula.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'Actor::CalculateDetectionFormula', exiting.");
			}
		}

		void RegisterActorProcessGetActorLightLevel()
		{
			REL::Relocation<AIProcessGetActorLightLevelSig> functionLocation{ ID::AIProcess::GetActorLightLevel };
			if (hook_AIProcessGetActorLightLevel.Create(reinterpret_cast<void*>(functionLocation.address()), &HookAIProcessGetActorLightLevel))
			{
				REX::DEBUG("Installed 'AIProcess::GetActorLightLevel' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'AIProcess::GetActorLightLevel', exiting.");
			}
		}

		void RegisterGamePlayFormulasGetLockXPReward()
		{
			REL::Relocation<GamePlayFormulasGetLockXPRewardSig> functionLocation{ ID::GamePlayFormulas::GetLockXPReward };
			if (hook_GamePlayFormulasGetLockXPReward.Create(reinterpret_cast<void*>(functionLocation.address()), &HookGamePlayFormulasGetLockXPReward))
			{
				REX::DEBUG("Installed 'GamePlayFormulas::GetLockXPReward' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'GamePlayFormulas::GetLockXPReward', exiting.");
			}
		}

		void RegisterREFR_LOCKIsInaccessible()
		{
			REL::Relocation<REFR_LOCKIsInaccessibleSig> functionLocation{ ID::REFR_LOCK::IsInaccessible };
			if (hook_REFR_LOCKIsInaccessible.Create(reinterpret_cast<void*>(functionLocation.address()), &HookREFR_LOCKIsInaccessible))
			{
				REX::DEBUG("Installed 'REFR_LOCK::IsInaccessible' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'REFR_LOCK::IsInaccessible', exiting.");
			}
		}

		void RegisterREFR_LOCKNumericValueToEnum()
		{
			REL::Relocation<REFR_LOCKNumericValueToEnumSig> functionLocation{ ID::REFR_LOCK::NumericValueToEnum };
			if (hook_REFR_LOCKNumericValueToEnum.Create(reinterpret_cast<void*>(functionLocation.address()), &HookREFR_LOCKNumericValueToEnum))
			{
				REX::DEBUG("Installed 'REFR_LOCK::NumericValueToEnum' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'REFR_LOC::KNumericValueToEnum', exiting.");
			}
		}

		void RegisternsHUDTypesNotificationData_ctor()
		{
			REL::Relocation<nsHUDTypesNotificationData_ctorSig> functionLocation{ ID::nsHUDTypes::NotificationInfo::ctor };
			if (hook_nsHUDTypesNotificationData_ctor.Create(reinterpret_cast<void*>(functionLocation.address()), &HooknsHUDTypesNotificationData_ctor))
			{
				REX::DEBUG("Installed 'nsHUDTypes::NotificationData::ctor' hook.");
				nsHUDTypesNotificationData_ctor_Original = reinterpret_cast<uintptr_t>(hook_nsHUDTypesNotificationData_ctor.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'nsHUDTypes::NotificationData::ctor', exiting.");
			}
		}

		void RegisterActorUnequipObject()
		{
			REL::Relocation<ActorUnequipObjectSig> functionLocation{ ID::Actor::UnequipObject };
			if (hook_ActorUnequipObject.Create(reinterpret_cast<void*>(functionLocation.address()), &HookActorUnequipObject))
			{
				REX::DEBUG("Installed 'Actor::UnequipObject' hook.");
				ActorUnequipObject_Original = reinterpret_cast<uintptr_t>(hook_ActorUnequipObject.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'Actor::UnequipObject', exiting.");
			}
		}

		void RegisterCalcTargetedLimbDamage()
		{
			REL::Relocation<CombatFormulasCalcTargetedLimbDamageSig> functionLocation{ ID::CombatFormulas::CalcTargetedLimbDamage };
			if (hook_CombatFormulasCalcTargetedLimbDamage.Create(reinterpret_cast<void*>(functionLocation.address()), &HookCombatFormulasCalcTargetedLimbDamage))
			{
				REX::DEBUG("Installed 'CombatFormulas::CalcTargetedLimbDamage' hook.");
				CombatFormulasCalcTargetedLimbDamage_Original = reinterpret_cast<uintptr_t>(hook_CombatFormulasCalcTargetedLimbDamage.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'CombatFormulas::CalcTargetedLimbDamage', exiting.");
			}
		}

		void RegisterGetBuildConfirmQuestion()
		{
			REL::Relocation<GetBuildConfirmQuestionSig> functionLocation{ ID::ExamineMenu::GetBuildConfirmQuestion };
			if (hook_GetBuildConfirmQuestion.Create(reinterpret_cast<void*>(functionLocation.address()), &HookExamineMenuGetBuildConfirmQuestion))
			{
				REX::DEBUG("Installed 'ExamineMenu::GetBuildConfirmQuestion' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'ExamineMenu::GetBuildConfirmQuestion', exiting.");
			}
		}

		void RegisterShowBuildFailureMessage()
		{
			REL::Relocation<ShowBuildFailureMessageSig> functionLocation{ ID::WorkbenchMenuBase::ShowBuildFailureMessage };
			if (hook_ShowBuildFailureMessage.Create(reinterpret_cast<void*>(functionLocation.address()), &HookWorkbenchMenuBaseShowBuildFailureMessage))
			{
				REX::DEBUG("Installed 'WorkbenchMenuBase::ShowBuildFailureMessage' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'WorkbenchMenuBase::ShowBuildFailureMessage', exiting.");
			}
		}

		void RegisterSetHealthPercHook()
		{
			REL::Relocation<SetHealthPercSig> functionLocation{ ID::ExtraDataList::SetHealthPerc };
			if (hook_SetHealthPerc.Create(reinterpret_cast<void*>(functionLocation.address()), &HookExtraDataListSetHealthPerc))
			{
				REX::DEBUG("Installed 'ExtraDataList::SetHealthPerc' hook.");
				SetHealthPercOriginal = reinterpret_cast<uintptr_t>(hook_SetHealthPerc.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'ExtraDataList::SetHealthPerc', exiting.");
			}
		}

		void RegisterAddItemHook()
		{
			REL::Relocation<AddItemSig> functionLocation{ ID::BGSInventoryList::AddItem };
			if (hook_AddItem.Create(reinterpret_cast<void*>(functionLocation.address()), &HookBGSInventoryListAddItem))
			{
				REX::DEBUG("Installed 'BGSInventoryList::AddItem' hook.");
				AddItemOriginal = reinterpret_cast<uintptr_t>(hook_AddItem.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'BGSInventoryList::AddItem', exiting.");
			}
		}

		void RegisterGetInventoryValueHook()
		{
			REL::Relocation<GetInventoryValueSig> functionLocation{ ID::BGSInventoryItemUtils::GetInventoryValue };
			if (hook_GetInventoryValue.Create(reinterpret_cast<void*>(functionLocation.address()), &HookBGSInventoryItemUtilsGetInventoryValue))
			{
				REX::DEBUG("Installed 'BGSInventoryItemUtils::GetInventoryValue' hook.");
				GetInventoryValueOriginal = reinterpret_cast<uintptr_t>(hook_GetInventoryValue.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'BGSInventoryItemUtils::GetInventoryValue', exiting.");
			}
		}

		void RegisterQCurrentModChoiceData()
		{
			REL::Relocation<QCurrentModChoiceDataSig> functionLocation{ ID::WorkbenchMenuBase::QCurrentModChoiceData };
			if (hook_QCurrentModChoiceData.Create(reinterpret_cast<void*>(functionLocation.address()), &HookWorkbenchMenuBaseQCurrentModChoiceData))
			{
				REX::DEBUG("Installed 'WorkbenchMenuBase::QCurrentModChoiceData' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'BGSInventoryItemUtils::GetInventoryValue', exiting.");
			}
		}

		void RegisterExamineMenuBuildConfirmed()
		{
			REL::Relocation<ExamineMenuBuildConfirmedSig> functionLocation{ ID::ExamineMenu::BuildConfirmed };
			if (hook_ExamineMenuBuildConfirmed.Create(reinterpret_cast<void*>(functionLocation.address()), &HookExamineMenuBuildConfirmed))
			{
				REX::DEBUG("Installed 'ExamineMenu::BuildConfirmed' hook.");
				ExamineMenuBuildConfirmedOriginal = reinterpret_cast<uintptr_t>(hook_ExamineMenuBuildConfirmed.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'ExamineMenu::BuildConfirmed', exiting.");
			}
		}

		void RegisterTESObjectWEAPFire()
		{
			REL::Relocation<TESObjectWEAPFireSig> functionLocation{ ID::TESObjectWEAP::Fire };
			if (hook_TESObjectWEAPFire.Create(reinterpret_cast<void*>(functionLocation.address()), &HookTESObjectWEAPFire))
			{
				REX::DEBUG("Installed 'TESObjectWEAP::Fire' hook.");
				TESObjectWEAPFireOriginal = reinterpret_cast<uintptr_t>(hook_TESObjectWEAPFire.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'TESObjectWEAP::Fire', exiting.");
			}
		}

		void RegisterCombatFormulasCalcWeaponDamage()
		{
			REL::Relocation<CombatFormulasCalcWeaponDamageSig> functionLocation{ ID::CombatFormulas::CalcWeaponDamage };
			if (hook_CombatFormulasCalcWeaponDamage.Create(reinterpret_cast<void*>(functionLocation.address()), &HookCombatFormulasCalcWeaponDamage))
			{
				REX::DEBUG("Installed 'CombatFormulas::CalcWeaponDamage' hook.");
				CombatFormulasCalcWeaponDamageOriginal = reinterpret_cast<uintptr_t>(hook_CombatFormulasCalcWeaponDamage.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'CombatFormulas::CalcWeaponDamage', exiting.");
			}
		}

		void RegisterGetEquippedArmorDamageResistance()
		{
			REL::Relocation<GetEquippedArmorDamageResistanceSig> functionLocation{ ID::ActorUtils::GetEquippedArmorDamageResistance };
			if (hook_GetEquippedArmorDamageResistance.Create(reinterpret_cast<void*>(functionLocation.address()), &HookGetEquippedDamageResistance))
			{
				REX::DEBUG("Installed 'ActorUtils::GetEquippedArmorDamageResistance' hook.");
				GetEquippedArmorDamageResistanceOriginal = reinterpret_cast<uintptr_t>(hook_GetEquippedArmorDamageResistance.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'ActorUtils::GetEquippedArmorDamageResistance', exiting.");
			}
		}

		void RegisterIUUIIUtilsAddItemCardInfoEntry()
		{
			REL::Relocation<IUUIIUtilsAddItemCardInfoEntrySig> functionLocation{ ID::InventoryUserUIUtils::AddItemCardInfoEntry };
			if (hook_IUUIIUtilsAddItemCardInfoEntry.Create(reinterpret_cast<void*>(functionLocation.address()), &HookIUUIIUtilsAddItemCardInfoEntry))
			{
				REX::DEBUG("Installed 'IUUIIUtils::AddItemCardInfoEntry' hook.");
				IUUIIUtilsAddItemCardInfoEntryOriginal = reinterpret_cast<uintptr_t>(hook_IUUIIUtilsAddItemCardInfoEntry.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'IUUIIUtils::AddItemCardInfoEntry', exiting.");
			}
		}

		void RegisterPipboyInventoryDataBaseAddItemsCardInfoEntry()
		{
			REL::Relocation<PipboyInventoryDataBaseAddItemCardInfoEntrySig> functionLocation{ ID::PipboyInventoryData::BaseAddItemCardInfoEntry };
			if (hook_PipboyInventoryDataBaseAddItemCardInfoEntry.Create(reinterpret_cast<void*>(functionLocation.address()), &HookPipboyInventoryDataBaseAddItemCardInfoEntry))
			{
				REX::DEBUG("Installed 'PipboyInventoryData::BaseAddItemCardInfoEntry' hook.");
				PipboyInventoryDataBaseAddItemCardInfoEntryOriginal = reinterpret_cast<uintptr_t>(hook_PipboyInventoryDataBaseAddItemCardInfoEntry.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'PipboyInventoryData::BaseAddItemCardInfoEntry', exiting.");
			}
		}

		void RegisterIUUIIUtilsPopulateItemCardInfo_Helper()
		{
			REL::Relocation<IUUIIUtilsPopulateItemCardInfo_HelperSig> functionLocation{ ID::InventoryUserUIUtils::PopulateItemCardInfo_Helper };
			if (hook_IUUIIUtilsPopulateItemCardInfo_Helper.Create(reinterpret_cast<void*>(functionLocation.address()), &HookIUUIIUtilsPopulateItemCardInfo_Helper))
			{
				REX::DEBUG("Installed 'IUUIIUtils::PopulateItemCardInfo_Helper' hook.");
				IUUIIUtilsPopulateItemCardInfo_HelperOriginal = reinterpret_cast<uintptr_t>(hook_IUUIIUtilsPopulateItemCardInfo_Helper.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'IUUIIUtils::PopulateItemCardInfo_Helper', exiting.");
			}
		}

		void RegisterPipboyInventoryUtilsFillResistTypeInfo()
		{
			REL::Relocation<PipboyInventoryUtilsFillResistTypeInfoSig> functionLocation{ ID::PipboyInventoryUtils::FillResistTypeInfo };
			if (hook_PipboyInventoryUtilsFillResistTypeInfo.Create(reinterpret_cast<void*>(functionLocation.address()), &HookPipboyInventoryUtilsFillResistTypeInfo))
			{
				REX::DEBUG("Installed 'PipboyInventoryUtils::FillResistTypeInfo' hook.");
				PipboyInventoryUtilsFillResistTypeInfo_Original = reinterpret_cast<uintptr_t>(hook_PipboyInventoryUtilsFillResistTypeInfo.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'PipboyInventoryUtils::FillResistTypeInfo', exiting.");
			}
		}

		void RegisterLoadingMenuPopulateLoadScreens()
		{
			REL::Relocation<LoadingMenuPopulateLoadScreensSig> functionLocation{ ID::LoadingMenu::PopulateLoadScreens };
			if (hook_LoadingMenuPopulateLoadScreens.Create(reinterpret_cast<void*>(functionLocation.address()), &HookLoadingMenuPopulateLoadScreens))
			{
				REX::DEBUG("Installed 'LoadingMenu::PopulateLoadScreens' hook.");
				LoadingMenuPopulateLoadScreens_Original = reinterpret_cast<uintptr_t>(hook_LoadingMenuPopulateLoadScreens.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'LoadingMenu::PopulateLoadScreens', exiting.");
			}
		}

		void RegisterGamePlayFormulasCanPickLockGateCheck()
		{
			REL::Relocation<GamePlayFormulasCanPickLockGateCheckSig> functionLocation{ ID::GamePlayFormulas::CanPickLockGateCheck };
			if (hook_GamePlayFormulasCanPickLockGateCheck.Create(reinterpret_cast<void*>(functionLocation.address()), &HookGamePlayFormulasCanPickLockGateCheck))
			{
				REX::DEBUG("Installed 'GamePlayFormulas::CanPickLockGate' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'GamePlayFormulas::CanPickLockGate', exiting.");
			}
		}

		void RegisterGamePlayFormulasCanHackGateCheck()
		{
			REL::Relocation<GamePlayFormulasCanHackGateCheckSig> functionLocation{ ID::GamePlayFormulas::CanHackGateCheck };
			if (hook_GamePlayFormulasCanHackGateCheck.Create(reinterpret_cast<void*>(functionLocation.address()), &HookGamePlayFormulasCanHackGateCheck))
			{
				REX::DEBUG("Installed 'GamePlayFormulas::CanHackGateCheck' hook.");
			}
			else
			{
				REX::CRITICAL("Failed to hook 'GamePlayFormulas::CanHackGateCheck', exiting.");
			}
		}

		void RegisterActorSPECIALModifiedCallback()
		{
			REL::Relocation<ActorSPECIALModifiedCallbackSig> functionLocation{ ID::Actor::SPECIALModifiedCallback };
			if (hook_ActorSPECIALModifiedCallback.Create(reinterpret_cast<void*>(functionLocation.address()), &HookActorSPECIALModifiedCallback))
			{
				REX::DEBUG("Installed 'Actor::SPECIALModifiedCallback' hook.");
				ActorSPECIALModifiedCallback_Original = reinterpret_cast<uintptr_t>(hook_ActorSPECIALModifiedCallback.GetTrampoline());
			}
			else
			{
				REX::CRITICAL("Failed to hook 'Actor::SPECIALModifiedCallback', exiting.");
			}
		};
	}
}

