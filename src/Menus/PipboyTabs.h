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