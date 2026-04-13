#pragma once

namespace Cascadia
{
	namespace Patches
	{
		class LoadEditorIDs
		{
		private:
			template <class Form>
			static void InstallHook()
			{
				REL::Relocation<std::uintptr_t> vtbl{ Form::VTABLE[0] };
				_GetFormEditorID = vtbl.write_vfunc(0x3A, GetFormEditorID);
				_SetFormEditorID = vtbl.write_vfunc(0x3B, SetFormEditorID);
			}

			static const char* GetFormEditorID(TESForm* a_this)
			{
				auto it = rmap.find(a_this->formID);
				if (it != rmap.end())
				{
					return it->second.c_str();
				}

				return _GetFormEditorID(a_this);
			}

			static bool SetFormEditorID(TESForm* a_this, const char* a_editor)
			{
				auto edid = std::string_view{ a_editor };
				if (a_this->formID < 0xFF000000 && !edid.empty())
				{
					AddToGameMap(a_this, a_editor);
				}

				return _SetFormEditorID(a_this, a_editor);
			}

		private:
			static void AddToGameMap(TESForm* a_this, const char* a_editorID)
			{
				const auto& [map, lock] = TESForm::GetAllFormsByEditorID();
				const BSAutoWriteLock locker{ lock.get() };
				if (map)
				{
					auto iter = map->find(a_editorID);
					if (iter != map->end())
					{
						if (iter->second->GetFormID() != a_this->GetFormID())
						{
							REX::WARN(
								"EditorID Conflict: {:08X} and {:08X} are both {:s}."sv,
								iter->second->GetFormID(),
								a_this->GetFormID(),
								a_editorID);
						}

						return;
					}

					map->emplace(a_editorID, a_this);
					rmap.emplace(a_this->formID, a_editorID);
				}
			}

			inline static std::unordered_map<std::uint32_t, std::string> rmap;

			inline static REL::Relocation<decltype(&TESForm::SetFormEditorID)> _SetFormEditorID;
			inline static REL::Relocation<decltype(&TESForm::GetFormEditorID)> _GetFormEditorID;

		public:
			static void Install()
			{
				// InstallHook<BGSKeyword>();
				// InstallHook<BGSLocationRefType>();
				// InstallHook<BGSAction>();
				InstallHook<BGSTransform>();
				InstallHook<BGSComponent>();
				InstallHook<BGSTextureSet>();
				// InstallHook<BGSMenuIcon>();
				// InstallHook<TESGlobal>();
				InstallHook<BGSDamageType>();
				InstallHook<TESClass>();
				InstallHook<TESFaction>();
				// InstallHook<BGSHeadPart>();
				InstallHook<TESEyes>();
				// InstallHook<TESRace>();
				// InstallHook<TESSound>();
				InstallHook<BGSAcousticSpace>();
				InstallHook<EffectSetting>();
				InstallHook<Script>();
				InstallHook<TESLandTexture>();
				InstallHook<EnchantmentItem>();
				InstallHook<SpellItem>();
				InstallHook<ScrollItem>();
				InstallHook<TESObjectACTI>();
				InstallHook<BGSTalkingActivator>();
				InstallHook<TESObjectARMO>();
				InstallHook<TESObjectBOOK>();
				InstallHook<TESObjectCONT>();
				InstallHook<TESObjectDOOR>();
				InstallHook<IngredientItem>();
				InstallHook<TESObjectLIGH>();
				InstallHook<TESObjectMISC>();
				InstallHook<TESObjectSTAT>();
				InstallHook<BGSStaticCollection>();
				InstallHook<BGSMovableStatic>();
				InstallHook<TESGrass>();
				InstallHook<TESObjectTREE>();
				InstallHook<TESFlora>();
				InstallHook<TESFurniture>();
				InstallHook<TESObjectWEAP>();
				InstallHook<TESAmmo>();
				InstallHook<TESNPC>();
				InstallHook<TESLevCharacter>();
				InstallHook<TESKey>();
				InstallHook<AlchemyItem>();
				InstallHook<BGSIdleMarker>();
				InstallHook<BGSNote>();
				InstallHook<BGSProjectile>();
				InstallHook<BGSHazard>();
				InstallHook<BGSBendableSpline>();
				InstallHook<TESSoulGem>();
				InstallHook<BGSTerminal>();
				InstallHook<TESLevItem>();
				InstallHook<TESWeather>();
				InstallHook<TESClimate>();
				InstallHook<BGSShaderParticleGeometryData>();
				InstallHook<BGSReferenceEffect>();
				InstallHook<TESRegion>();
				// InstallHook<NavMeshInfoMap>();
				// InstallHook<TESObjectCELL>();
				// InstallHook<TESObjectREFR>();
				InstallHook<Explosion>();
				InstallHook<Projectile>();
				//InstallHook<Actor>();
				InstallHook<PlayerCharacter>();
				InstallHook<MissileProjectile>();
				InstallHook<ArrowProjectile>();
				InstallHook<GrenadeProjectile>();
				InstallHook<BeamProjectile>();
				InstallHook<FlameProjectile>();
				InstallHook<ConeProjectile>();
				InstallHook<BarrierProjectile>();
				InstallHook<Hazard>();
				// InstallHook<TESWorldSpace>();
				// InstallHook<TESObjectLAND>();
				// InstallHook<NavMesh>();
				InstallHook<TESTopicInfo>();
				// InstallHook<TESQuest>();
				// InstallHook<TESIdleForm>();
				InstallHook<TESPackage>();
				InstallHook<AlarmPackage>();
				InstallHook<DialoguePackage>();
				InstallHook<FleePackage>();
				InstallHook<SpectatorPackage>();
				InstallHook<TrespassPackage>();
				InstallHook<TESCombatStyle>();
				InstallHook<TESLoadScreen>();
				InstallHook<TESLevSpell>();
				// InstallHook<TESObjectANIO>();
				InstallHook<TESWaterForm>();
				InstallHook<TESEffectShader>();
				InstallHook<BGSExplosion>();
				InstallHook<BGSDebris>();
				InstallHook<TESImageSpace>();
				// InstallHook<TESImageSpaceModifier>();
				InstallHook<BGSListForm>();
				InstallHook<BGSPerk>();
				InstallHook<BGSBodyPartData>();
				InstallHook<BGSAddonNode>();
				// InstallHook<ActorValueInfo>();
				InstallHook<BGSCameraShot>();
				InstallHook<BGSCameraPath>();
				// InstallHook<BGSVoiceType>();
				InstallHook<BGSMaterialType>();
				InstallHook<BGSImpactData>();
				InstallHook<BGSImpactDataSet>();
				InstallHook<TESObjectARMA>();
				InstallHook<BGSEncounterZone>();
				InstallHook<BGSLocation>();
				InstallHook<BGSMessage>();
				// InstallHook<BGSDefaultObjectManager>();
				// InstallHook<BGSDefaultObject>();
				InstallHook<BGSLightingTemplate>();
				// InstallHook<BGSMusicType>();
				InstallHook<BGSFootstep>();
				InstallHook<BGSFootstepSet>();
				// InstallHook<BGSStoryManagerBranchNode>();
				// InstallHook<BGSStoryManagerQuestNode>();
				// InstallHook<BGSStoryManagerEventNode>();
				//InstallHook<BGSDialogueBranch>();
				InstallHook<BGSMusicTrackFormWrapper>();
				InstallHook<TESWordOfPower>();
				InstallHook<TESShout>();
				InstallHook<BGSEquipSlot>();
				InstallHook<BGSRelationship>();
				//InstallHook<BGSScene>();
				InstallHook<BGSAssociationType>();
				InstallHook<BGSOutfit>();
				InstallHook<BGSArtObject>();
				InstallHook<BGSMaterialObject>();
				InstallHook<BGSMovementType>();
				// InstallHook<BGSSoundDescriptorForm>();
				InstallHook<BGSDualCastData>();
				InstallHook<BGSSoundCategory>();
				InstallHook<BGSSoundOutput>();
				InstallHook<BGSCollisionLayer>();
				InstallHook<BGSColorForm>();
				InstallHook<BGSReverbParameters>();
				// InstallHook<BGSPackIn>();
				InstallHook<BGSAimModel>();
				InstallHook<BGSConstructibleObject>();
				InstallHook<BGSMod::Attachment::Mod>();
				InstallHook<BGSMaterialSwap>();
				InstallHook<BGSZoomData>();
				InstallHook<BGSInstanceNamingRules>();
				InstallHook<BGSSoundKeywordMapping>();
				InstallHook<BGSAudioEffectChain>();
				// InstallHook<BGSAttractionRule>();
				InstallHook<BGSSoundCategorySnapshot>();
				InstallHook<BGSSoundTagSet>();
				InstallHook<BGSLensFlare>();
				InstallHook<BGSGodRays>();

				REX::DEBUG("Installed Patch: 'LoadEditorIDs'.");
			}
		};
	}
}