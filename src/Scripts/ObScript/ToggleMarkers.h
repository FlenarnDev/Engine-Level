#pragma once
namespace ObScript
{
	class ToggleMarkers
	{
	public:
		static void Install()
		{
			const auto functions = RE::SCRIPT_FUNCTION::GetConsoleFunctions();
			const auto it = std::find_if(
				functions.begin(),
				functions.end(),
				[&](auto&& a_elem)
				{
					return _stricmp(a_elem.functionName, "ToggleMarkers") == 0;
				});

			if (it != functions.end())
			{
				*it = RE::SCRIPT_FUNCTION{ LONG_NAME.data(), SHORT_NAME.data(), it->output };
				it->helpString = HelpString().data();
				it->referenceFunction = false;
				it->executeFunction = Execute;

				REX::DEBUG("Registered 'ToggleMarkers' command.");
			}
			else
			{
				REX::WARN("Failed to register 'ToggleMarkers' command.");
			}
		}
	
	private:
		static bool Execute(
			const RE::SCRIPT_PARAMETER* a_parameters,
			const char* a_compiledParams,
			RE::TESObjectREFR* a_refObject,
			RE::TESObjectREFR* a_container,
			RE::Script* a_script,
			RE::ScriptLocals* a_scriptLocals,
			float&,
			std::uint32_t& a_offset)
		{
			RE::INISettingCollection* INISettingCollection = RE::INISettingCollection::GetSingleton();

			RE::Setting* bShowMarkersSetting = INISettingCollection->GetSetting("bShowMarkers:Display");
			bool showMarkers = !bShowMarkersSetting->GetBinary();
			bShowMarkersSetting->SetBinary(showMarkers);
			REX::DEBUG("'ToggleMarkers' - value set to: {}", showMarkers);

			CullMarkers(RE::TES::GetSingleton(), !showMarkers);
			if (showMarkers)
			{
				RE::ConsoleLog::GetSingleton()->PrintLine("Toggled markers on.");
			}
			else
			{
				RE::ConsoleLog::GetSingleton()->PrintLine("Toggled markers off.");
			}
			return true;
		}

		[[nodiscard]] static const std::string& HelpString()
		{
			static auto help = []()
			{
				std::string buf;
				buf += "Toggle markers"sv;
				return buf;
			}();
			return help;
		}

		static constexpr auto LONG_NAME = "ToggleMarkers"sv;
		static constexpr auto SHORT_NAME = "tmk"sv;

		static void CullMarkers(RE::TES* tes, bool hideMarkers)
		{
			RE::INIPrefSettingCollection* INIPrefSettingCollection = RE::INIPrefSettingCollection::GetSingleton();
			RE::Setting* uGridsToLoadSetting = INIPrefSettingCollection->GetSetting("uGridsToLoad:General");
			std::uint32_t uGridsToLoad = uGridsToLoadSetting->GetUInt();

			RE::SetCullMarkersFunctor setCullMarkersFunctor;
			for (std::uint32_t x = 0; x < uGridsToLoad; ++x)
			{
				std::uint32_t y = 0;
				if (uGridsToLoad)
				{
					do
					{
						RE::GridCell* gridCell = tes->gridCells->Get(x, y);
						RE::TESObjectCELL* cell = gridCell->cell;
						if (gridCell->cell && cell->cellState == RE::TESObjectCELL::CELL_STATE::kAttached)
						{
							RE::NiNode* cell3D = gridCell->cell->loadedData->cell3D.get();
							if (cell3D)
							{
								if (cell3D->children.size() > 1)
								{
									RE::NiAVObject* childData = cell3D->children[1].get();
									if (childData)
									{
										childData->SetAppCulled(hideMarkers);
									}
								}
								cell->SetCullCellMarkers(hideMarkers);
								setCullMarkersFunctor.cull = hideMarkers;
								ForEachReferenceSetCullMarkersFunctor(cell, &setCullMarkersFunctor);
							}
						}
						uGridsToLoad = uGridsToLoadSetting->GetUInt();
						++y;
					} 
					while (y < uGridsToLoad);
				}
			}

			RE::TES* tempTES = tes;
			RE::TESObjectCELL* interiorCell = tes->interiorCell;
			if (interiorCell)
			{
				RE::NiNode* cell3D = interiorCell->loadedData->cell3D.get();
				if (cell3D)
				{
					if (cell3D->children.size() > 1)
					{
						RE::NiAVObject* childData = cell3D->children[1].get();
						if (childData)
						{
							childData->SetAppCulled(hideMarkers);
						}
					}
					interiorCell->SetCullCellMarkers(hideMarkers);
					setCullMarkersFunctor.cull = hideMarkers;
					ForEachReferenceSetCullMarkersFunctor(interiorCell, &setCullMarkersFunctor);
					tempTES = tes;
				}
			}
			
			RE::TESWorldSpace* worldspace = tempTES->worldSpace;
			if (worldspace)
			{
				RE::TESObjectCELL* skyCell = worldspace->skyCell;
				if (skyCell)
				{
					RE::NiNode* skyCell3D = skyCell->loadedData->cell3D.get();
					if (skyCell3D)
					{
						if (skyCell3D->children.size() > 1)
						{
							RE::NiAVObject* childData = skyCell3D->children[1].get();
							if (childData)
							{
								childData->SetAppCulled(hideMarkers);
							}
						}
						skyCell->SetCullCellMarkers(hideMarkers);
						setCullMarkersFunctor.cull = hideMarkers;
						ForEachReferenceSetCullMarkersFunctor(skyCell, &setCullMarkersFunctor);
					}
				}
			}

			RE::NiNode* objRoot = tes->objRoot;
			RE::NiUpdateData updateData;
			memset(&updateData.camera, 0, 20);
			updateData.time = 0.0f;
			objRoot->Update(updateData);
		}

		static void ForEachReferenceSetCullMarkersFunctor(RE::TESObjectCELL* a_cell, RE::SetCullMarkersFunctor* a_func)
		{
			RE::BSSpinLock* spinLock = &a_cell->spinLock;
			spinLock->lock(0);
			RE::BSContainer::ForEachResult forEachResult = RE::BSContainer::ForEachResult::kStop;

			std::int64_t referencesSize;
			for (std::int64_t i = a_cell->references.size() - 1; i >= 0; i = (i - 1))
			{
				if (forEachResult != RE::BSContainer::ForEachResult::kStop)
				{
					break;
				}

				if (i >= a_cell->references.size())
				{
					referencesSize = a_cell->references.size();
				}

				if (a_cell->references.empty())
				{
					REX::WARN("'ToggleMarkers' - refrence array lacks data.");
				}

				forEachResult = SetCullMarkersFunctorOperator(a_func, &a_cell->references.at(i));
			}
			if (spinLock)
			{
				spinLock->unlock();
			}
		}

		static RE::BSContainer::ForEachResult SetCullMarkersFunctorOperator(RE::SetCullMarkersFunctor* a_func, RE::NiPointer<RE::TESObjectREFR>* a_ref)
		{
			RE::TESBoundObject* objectReference;
			RE::NiAVObject* activator;
			RE::NiAVObject* v6;
			RE::TESObjectREFR* a_refr = a_ref->get();

			if (a_refr && a_refr->Get3D())
			{
				if (a_refr->IsMarker() || a_refr->extraList.get()->GetPrimitive())
				{
					objectReference = a_refr ->data.objectReference;
					if (objectReference && objectReference->formType == RE::ENUM_FORM_ID::kACTI)
					{
						activator = a_refr->Get3D();
						activator->SetAppCulled(a_func->cull);
					}
				}

				objectReference = a_refr ->data.objectReference;
				v6 = a_refr->Get3D();
				if ((v6->flags.flags & 0x4000) != 0)
				{
					RE::BSFadeNode* fadeNode = v6->IsFadeNode();
					if (fadeNode)
					{
						fadeNode->IncRefCount();
						fadeNode->flags.flags &= 0xFFFFFFFFBFFFFFFF;
						if (!fadeNode->DecRefCount())
						{
							fadeNode->DeleteThis();
						}
					}
				}

			}
			return RE::BSContainer::ForEachResult::kStop;
		}
	};
}