#pragma once

#include "Systems/Skills.h"

namespace RE
{
	namespace Cascadia
	{
		class TESHitEventWatcher :
			public BSTEventSink<TESHitEvent>
		{
			virtual BSEventNotifyControl ProcessEvent(const TESHitEvent& a_event, BSTEventSource<TESHitEvent>*) override
			{
				TESObjectREFR* target = a_event.target.get();

				// Return early on invalid events.
				// Note: Unsure about the logic RE Power Armor here.
				if (!target || !a_event.usesHitData || target->extraList->GetByType(kPowerArmor) != nullptr)
				{
					return BSEventNotifyControl::kContinue;
				}

				// Return if 'target' is not a 'Actor'.
				if (target->formType != ENUM_FORM_ID::kACHR)
				{
					REX::DEBUG("TESHitEvent: 'target' was not of type 'Actor'.");
					return BSEventNotifyControl::kContinue;
				}

				Actor* hitActor = (Actor*)target;

				// Extra power armor check.
				if (PowerArmor::ActorInPowerArmor(*hitActor))
				{
					return BSEventNotifyControl::kContinue;
				}

				if (hitActor != PlayerCharacter::GetSingleton())
				{
					TESObjectREFR* cause = a_event.cause.get();
					if (cause != nullptr)
					{
						if (cause->formType == ENUM_FORM_ID::kACHR)
						{
							Actor* causeActor = (Actor*)cause;

							// Here we also handle weapon degradation for melee attacks - including weapon bashing.
							if (causeActor == PlayerCharacter::GetSingleton())
							{
								PlayerCharacter* playerCharacter = (PlayerCharacter*)causeActor;
								TESObjectREFR* sourceREFR = a_event.hitData.sourceRef.get().get();
								if (!sourceREFR)
								{
									// If sourceREFR is 'nullptr' it's a melee attack.
									TESFormID weaponFormID = a_event.hitData.weapon.object->GetFormID();
									BGSInventoryItem* inventoryItem = nullptr;

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
										ExtraDataList* extraDataList = inventoryItem->stackData->extra.get();

										float currentHealth = extraDataList->GetHealthPerc();

										TESObjectWEAP* weapon = (TESObjectWEAP*)a_event.hitData.weapon.object;

										float newHealth;
										float conditionReduction = 0.015f; // 1.5% degradation when weapon bashing, melee weapons use random value.

										// Melee weapon
										if (weapon->weaponData.type != WEAPON_TYPE::kGun)
										{
											conditionReduction = BSRandom::Float(0.005, 0.015);
										}
										
										// Reduces damage to weapon depending on players 'Melee weapons' level.
										// Linear reduction from 0 - 100, 100 resulting in 20% less damage to weapon.
										float meleeWeaponsSkillValue = playerCharacter->GetActorValue(*Skills::CascadiaActorValues.MeleeWeapons);
										float reductionPercentFromSkill = (meleeWeaponsSkillValue / 100.0f) * 0.2;

										conditionReduction *= (1.0f - reductionPercentFromSkill);

										newHealth = std::max(currentHealth - conditionReduction, 0.0f); 

										if (newHealth == 0.0f)
										{
											ActorEquipManager::GetSingleton()->UnequipItem(playerCharacter, &equippedWeapon, false);
											GameSettingCollection* gameSettingCollection = GameSettingCollection::GetSingleton();
											SendHUDMessage::ShowHUDMessage(gameSettingCollection->GetSetting("sWeaponBreak")->GetString().data(), "UIWorkshopModeItemScrapGeneric", true, true);
										}
										extraDataList->SetHealthPerc(newHealth);
										PipboyDataManager::GetSingleton()->inventoryData.RepopulateItemCardsOnSection(ENUM_FORM_ID::kWEAP);
									}
								}
							}
						}
					}

					return BSEventNotifyControl::kContinue;
				}

				TESObjectREFR* sourceREFR = a_event.hitData.sourceRef.get().get();
				BGSBodyPartData* bodyPartData = hitActor->GetBodyPartData();

				// If sourceREFR is 'nullptr', most likely a melee attack.
				if (sourceREFR)
				{
					
					if (sourceREFR->IsExplosion())
					{
						REX::DEBUG("Player hit, explosion?");
						// TODO
					}
					else
					{
						REX::DEBUG("Player hit, not explosion, melee?");
						std::uint32_t damageLimb = a_event.hitData.damageLimb.underlying();
						if (damageLimb != std::uint32_t(BGSBodyPartDefs::LIMB_ENUM::kWeapon) && damageLimb != std::uint32_t(BGSBodyPartDefs::LIMB_ENUM::kNone))
						{
							REX::DEBUG("No explosion - limb hit: {}", a_event.hitData.damageLimb.underlying());
						}
					}
				}
				else
				{
					REX::DEBUG("Player hit, melee?");
					std::uint32_t damageLimb = a_event.hitData.damageLimb.underlying();
					if (damageLimb != std::uint32_t(BGSBodyPartDefs::LIMB_ENUM::kWeapon) && damageLimb != std::uint32_t(BGSBodyPartDefs::LIMB_ENUM::kNone))
					{
						// Weapons like the 'Ashmaker' only do physical damage (unclear as to why), take that into account here.
						float damage = std::max(a_event.hitData.healthDamage, a_event.hitData.physicalDamage);

						REX::DEBUG("No explosion - limb hit: {}", a_event.hitData.damageLimb.underlying());
					}
				}

				return BSEventNotifyControl::kContinue;
			}
		};

		void RegisterTESHitEventSink()
		{
			TESHitEventWatcher* tesHitEvent = new TESHitEventWatcher();
			TESHitEvent::GetEventSource()->RegisterSink(tesHitEvent);
			REX::DEBUG("Registered 'TESHitEvent' sink.");
		}
	}
}