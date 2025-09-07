#pragma once

#include "Shared/SharedFunctions.h"
#include "Systems/Skills.h"

// Massive thanks to Neanka - the OG interface god.
// https://github.com/Neanka/f4se/tree/master/f4se/PipboyTabs

namespace RE
{
	namespace Cascadia
	{
		namespace PipboyTabs
		{
			struct avifStruct
			{
				std::uint32_t type;
				std::uint32_t showListOnly;
				std::string id;
				std::string name;
				std::uint32_t page;
				BSTArray<TESForm*> list1;
				BSTArray<TESForm*> list2;
				std::vector<std::string> stringList;
				BSTArray<TESForm*> filters;
				std::vector<float> maxValues;
				std::uint32_t precision;
			};
			std::vector<avifStruct> avifPages;

			Scaleform::GFx::Value loader;

			void PopulateSkillEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, ActorValueInfo* a_skill, std::vector<std::string> a_stringValue)
			{
				Scaleform::GFx::Value skillEntry;
				a_movieRoot->CreateObject(&skillEntry);
				PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

				float value = playerCharacter->GetBaseActorValue(*a_skill);
				float buffedValue = playerCharacter->GetActorValue(*a_skill);
				BSStringT<char> description;
				a_skill->GetDescription(description);

				skillEntry.SetMember("text", a_skill->fullName.c_str());
				skillEntry.SetMember("qname", a_skill->GetFormEditorID());
				skillEntry.SetMember("description", description.c_str());
				if (a_stringValue.size() > (int)value)
				{
					skillEntry.SetMember("stringValue", a_stringValue[(int)value].c_str());
				}
				else
				{
					skillEntry.SetMember("stringValue", "");
				}
				skillEntry.SetMember("readed", false);
				skillEntry.SetMember("formid", a_skill->formID);
				skillEntry.SetMember("value", buffedValue);
				skillEntry.SetMember("maxVal", 0.0);
				skillEntry.SetMember("basevalue", value);
				skillEntry.SetMember("modifier", buffedValue - value);
				skillEntry.SetMember("buffervalue", buffedValue);

				std::uint32_t filter = 1;
				skillEntry.SetMember("filterFlag", filter);
				a_destination->PushBack(skillEntry);
			}

			void PopulateEntry(Scaleform::GFx::Value* a_destination, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, TESForm* a_form1, TESForm* a_form2, std::uint32_t filter, std::vector<std::string> a_stringValue, float a_maxValue)
			{
				Scaleform::GFx::Value entry;
				a_movieRoot->CreateObject(&entry);
				float value = 0;
				float buffedValue = 0;

				std::string nameStr = "";
				std::string descriptionStr = "";

				if (a_form1->formType == ENUM_FORM_ID::kAVIF)
				{
					PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();
					ActorValueInfo* actorValue = dynamic_cast<ActorValueInfo*>(a_form1);
					value = playerCharacter->GetBaseActorValue(*actorValue);
					buffedValue = playerCharacter->GetActorValue(*actorValue);
					nameStr = actorValue->fullName.c_str();
					
					BSStringT<char> description;
					actorValue->GetDescription(description);
					descriptionStr = description.c_str();

				}
				else if (a_form1->formType == ENUM_FORM_ID::kGLOB)
				{
					value = static_cast<TESGlobal*>(a_form1)->value;
					buffedValue = value;
				}

				if (a_form2)
				{
					if (a_form2->formType == ENUM_FORM_ID::kAVIF)
					{


						ActorValueInfo* actorValue = dynamic_cast<ActorValueInfo*>(a_form2);
						std::string tempNameStr = actorValue->fullName.c_str();
						if (tempNameStr.length() > 0) {
							nameStr = tempNameStr;
						}

						BSStringT<char> description;
						actorValue->GetDescription(description);
						std::string tempDescriptionStr = description.c_str();
						if (tempDescriptionStr.length() > 0)
						{
							descriptionStr = tempDescriptionStr;
						}

					}
				}

				entry.SetMember("readed", false);
				if (nameStr.length() > 0)
				{
					entry.SetMember("text", nameStr.c_str());
				}
				else
				{
					entry.SetMember("text", a_form1->GetFormEditorID());
				}
				entry.SetMember("qname", a_form1->GetFormEditorID());
				entry.SetMember("description", descriptionStr.c_str());
				if (a_stringValue.size() > (std::uint32_t)value)
				{
					entry.SetMember("stringValue", a_stringValue.at((std::uint32_t)value).c_str());
				}
				else
				{
					entry.SetMember("stringValue", "");
				}
				entry.SetMember("formid", a_form1->formID);

				entry.SetMember("value", buffedValue);
				entry.SetMember("maxVal", a_maxValue);
				entry.SetMember("baseValue", value);
				entry.SetMember("modifier", buffedValue - value);
				entry.SetMember("filterFlag", filter);

				a_destination->PushBack(entry);

			}

			void UpdateMenu_Internal(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot)
			{
				if (Cascadia::Skills::CascadiaSkillsLevelUp.size() > 0)
				{
					Scaleform::GFx::Value argumentsArray[7];
					a_movieRoot->CreateString(&argumentsArray[0], "skills");
					a_movieRoot->CreateString(&argumentsArray[1], "$CAS_SKILLS");
					argumentsArray[2] = 0;
					argumentsArray[3] = 0;
					a_movieRoot->CreateObject(&argumentsArray[4]);
					Scaleform::GFx::Value skillArray;
					a_movieRoot->CreateArray(&skillArray);
					std::vector<std::string> stringArray;

					for (std::int32_t skillEntry = static_cast<std::int32_t>(Cascadia::Skills::CascadiaSkillsLevelUp.size()) - 1; skillEntry >= 0; skillEntry--)
					{
						ActorValueInfo* baseSkill = Cascadia::Skills::CascadiaSkillsLevelUp.at(skillEntry);
						PopulateSkillEntry(&skillArray, a_movieRoot, baseSkill, stringArray);
					}

					argumentsArray[4].SetMember("skillsList", skillArray);
					argumentsArray[5] = 0;
					argumentsArray[6] = 0;

					a_movieRoot->Invoke("root.Menu_mc.PipboyTabs_loader.content.registerTab", nullptr, argumentsArray, 7);
				}

				for (avifStruct avif : avifPages)
				{
					Scaleform::GFx::Value argumentsArray[7];
					a_movieRoot->CreateString(&argumentsArray[0], avif.id.c_str());
					a_movieRoot->CreateString(&argumentsArray[1], avif.name.c_str());
					argumentsArray[2] = avif.page;
					argumentsArray[3] = avif.type;
					a_movieRoot->CreateObject(&argumentsArray[4]);
					Scaleform::GFx::Value avifArray;
					a_movieRoot->CreateArray(&avifArray);

					for (std::int32_t avifEntry = 0; avifEntry < avif.list1.size(); avifEntry++)
					{
						TESForm* baseForm = avif.list1.at(avifEntry);

						std::uint32_t filterValue = 1;
						if (avif.filters.size() > avifEntry)
						{
							TESForm* filterForm = avif.filters[avifEntry];
							if (filterForm->formType == ENUM_FORM_ID::kGLOB)
							{
								TESGlobal* global = dynamic_cast<TESGlobal*>(filterForm);
								filterValue = (std::uint32_t)global->value;
							}
							else if (filterForm->formType == ENUM_FORM_ID::kAVIF)
							{
								PlayerCharacter* playerCharacter = PlayerCharacter::GetSingleton();

								ActorValueInfo* actorValue = dynamic_cast<ActorValueInfo*>(filterForm);
								filterValue = (std::uint32_t)playerCharacter->GetBaseActorValue(*actorValue);
							}
						}

						float maxValue;
						if (avif.maxValues.size() > avifEntry)
						{
							maxValue = avif.maxValues.at(avifEntry);
						}
						else
						{
							maxValue = avif.maxValues.at(0);
						}

						if (avif.list2.size() > avifEntry)
						{
							PopulateEntry(&avifArray, a_movieRoot, baseForm, avif.list2.at(avifEntry), filterValue, avif.stringList, maxValue);
						}
						else
						{
							PopulateEntry(&avifArray, a_movieRoot, baseForm, nullptr, filterValue, avif.stringList, maxValue);
						}
					}

					Scaleform::GFx::Value charArray;
					a_movieRoot->CreateArray(&charArray);
					for (std::uint32_t stringEntry = 0; stringEntry < avif.stringList.size(); stringEntry++)
					{
						Scaleform::GFx::Value stringValue;
						a_movieRoot->CreateString(&stringValue, avif.stringList.at(stringEntry).c_str());
						charArray.PushBack(&stringValue);
					}

					argumentsArray[4].SetMember("StringArray", charArray);
					argumentsArray[5] = avif.showListOnly;
					argumentsArray[6] = avif.precision;

					a_movieRoot->Invoke("root.Menu_mc.PipboyTabs_loader.content.registerTab", nullptr, argumentsArray, 7);
				}
			}

			class Ready : public Scaleform::GFx::FunctionHandler
			{
			public:
				virtual void Call(const Params& a_params)
				{
					UpdateMenu_Internal(a_params.movie->asMovieRoot);
				}
			};

			bool RegisterScaleform(Scaleform::GFx::Movie* a_view, Scaleform::GFx::Value* a_value)
			{
				Scaleform::GFx::Value currentSWFPath;
				// Register native code handlers.
				if (a_view->asMovieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url"))
				{
					if (_stricmp(currentSWFPath.GetString(), "Interface/PipboyMenu.swf") == 0)
					{
						Scaleform::GFx::Value root, urlRequest, loader;

						Scaleform::GFx::Value SWFDeclaration = "PipboyTabs.swf";

						a_view->asMovieRoot->GetVariable(&root, "root");
						a_view->asMovieRoot->CreateObject(&loader, "flash.display.Loader");
						a_view->asMovieRoot->CreateObject(&urlRequest, "flash.net.URLRequest", &SWFDeclaration, 1);

						loader.SetMember("name", "PipboyTabs_loader");
						root.SetMember("PipboyTabs_loader", &loader);

						Scaleform::GFx::Value pipboyTabs;
						a_view->asMovieRoot->CreateObject(&pipboyTabs);
						root.SetMember("pbt", pipboyTabs);

						RegisterFunction<Ready>(&pipboyTabs, a_view->asMovieRoot, "ready");

						loader.Invoke("load", nullptr, &urlRequest, 1);
						a_view->asMovieRoot->Invoke("root.Menu_mc.addChild", nullptr, &loader, 1);
					}
					return true;
				}
				return false;
			}
		}
	}
}