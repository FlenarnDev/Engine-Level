#include "Events/BSAnimationGraphEvent.h"
#include "Events/LevelIncreaseEvent.h"
#include "Events/MenuOpenCloseEvent.h"
#include "Events/TESCellFullyLoadedEvent.h"
#include "Events/TESHavestEvent.h"
#include "Events/TESHitEvent.h"
#include "Events/TESInitScriptEvent.h"
#include "Events/TESLoadGameEvent.h"
#include "Menus/ExamineConfirmMenu.h"
#include "Menus/PipboyMenu.h"
#include "Patches/Patches.h"
#include "Scripts/ObScript.h"
#include "Serialization/Serialization.h"
#include "Shared/SharedDeclarations.h"
#include "Systems/Skills.h"

namespace RE
{
	namespace Cascadia
	{
		namespace Hooks
		{
			extern void Install();
			extern void RegisterHooks();
		}
		
		extern void DefineItemDegradationFormsFromGame();
	}
}

namespace
{
	void MessageHandler(F4SE::MessagingInterface::Message* a_msg)
	{
		if (!a_msg)
		{
			return;
		}

		switch (a_msg->type)
		{
		case F4SE::MessagingInterface::kGameDataReady:
			RE::Cascadia::AmmoSwitch::DefineAmmoLists();
			RE::Cascadia::Shared::InitializeSharedGameVariables();
			if (RE::Cascadia::Skills::DefineSkillsFormsFromGame())
			{
				RE::Cascadia::Skills::RegisterForSkillLink();
			}
			else
			{
				REX::CRITICAL("Failed to define skills.");
			}

			RE::Cascadia::DefineItemDegradationFormsFromGame();
			RE::Cascadia::BSAnimationGraphEventWatcher::Install();
			RE::Cascadia::RegisterTESInitScriptEventSink();
			RE::Cascadia::RegisterTESHarvestEventSink();
			RE::Cascadia::RegisterTESHitEventSink();
			RE::Cascadia::RegisterForCellFullyLoaded();
			RE::Cascadia::RegisterLevelIncreaseEventSink();
			RE::Cascadia::RegisterTESLoadGameEventSink();
			RE::Cascadia::Skills::GetLevelUpFormsFromGame();

			REX::INFO("{:s} - kGameDataReady", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kGameLoaded:
			REX::INFO("{:s} - 'kGameLoaded'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kPostLoad:
			REX::INFO("{:s} - 'kPostLoad'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kPostLoadGame:
			RE::Cascadia::AmmoSwitch::PostLoadGameAmmoFix();
			REX::INFO("{:s} - 'kPostLoadGame'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kPostPostLoad:
			REX::INFO("{:s} - 'kPostPostLoad'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kPostSaveGame:
			REX::INFO("{:s} - 'kPostSaveGame'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kDeleteGame:
			REX::INFO("{:s} - 'kDeleteGame'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kNewGame:
			REX::INFO("{:s} - 'kNewGame'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kInputLoaded:
			REX::INFO("{:s} - 'kInputLoaded'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kPreLoadGame:
			REX::INFO("{:s} - 'kPreLoadGame'.", "Cascadia Gameplay Systems");
			break;

		case F4SE::MessagingInterface::kPreSaveGame:
			REX::INFO("{:s} - 'kPreSaveGame'.", "Cascadia Gameplay Systems");
			break;

		default:
			break;
		}
	}
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, { .trampoline = true, .trampolineSize = 0x8000 });


	RE::Cascadia::Shared::InitializeSharedVariables();
	RE::Cascadia::Hooks::Install();

	const F4SE::SerializationInterface* serialization = F4SE::GetSerializationInterface();
	if (!serialization)
	{
		REX::CRITICAL("Failed to register serialization interface, marking as incompatible."sv);
		return false;
	}
	else
	{
		serialization->SetUniqueID('CAS');
		serialization->SetRevertCallback(RE::Cascadia::Serialization::RevertCallback);
		serialization->SetSaveCallback(RE::Cascadia::Serialization::SaveCallback);
		serialization->SetLoadCallback(RE::Cascadia::Serialization::LoadCallback);
	}

	const F4SE::MessagingInterface* messaging = F4SE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(MessageHandler))
	{
		REX::CRITICAL("Failed to register messaging handler, marking as incompatible."sv);
		return false;
	}

	const F4SE::ScaleformInterface* scaleform = F4SE::GetScaleformInterface();
	if (!scaleform)
	{
		REX::CRITICAL("Failed to register Scaleform callback, marking as incompatible."sv);
		return false;
	}
	else
	{
		if (!scaleform->Register("Cascadia-DialogueMenu", RE::Cascadia::DialogueMenu::RegisterScaleform))
		{
			REX::CRITICAL("Failed to register 'DialogueMenu', marking as incompatible."sv);
			return false;
		}

		if (!scaleform->Register("Cascadia-ExamineMenu", RE::Cascadia::ExamineMenu::RegisterScaleform))
		{
			REX::CRITICAL("Failed to register 'ExamineMenu', marking as incompatible."sv);
			return false;
		}

		/*if (!scaleform->Register("Cascadia-PipboyMenu", RE::Cascadia::PipboyMenu::RegisterScaleform))
		{
			REX::CRITICAL("Failed to register 'PipboyMenu', marking as incompatible.");
			return false;
		}*/

		if (!scaleform->Register("Cascadia-ExamineConfirmMenu", RE::Cascadia::ExamineConfirmMenu::RegisterScaleform))
		{
			REX::CRITICAL("Failed to register 'ExamineConfirmMenu', marking as incompatible."sv);
			return false;
		}


		RE::Cascadia::LevelUpMenu::RegisterMenu();
		if (!scaleform->Register("CASLevelUpMenu", RE::Cascadia::LevelUpMenu::RegisterScaleform))
		{
			REX::CRITICAL("Failed to register 'LevelUpMenu', marking as incompatible."sv);
			return false;
		}
	}

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	if (!papyrus)
	{
		REX::CRITICAL("Failed to register Papyrus handler, marking as incompatible."sv);
		return false;
	}
	else
	{

	}

	RE::Cascadia::RegisterMenuOpenCloseEventSink();
	RE::Cascadia::ExamineMenu::hkOnButtonEvent::InstallHook();
	RE::Cascadia::Hooks::RegisterHooks();
	RE::Cascadia::Patches::Install();
	ObScript::Install();

	REX::INFO(("{:s} finished loading."), "Cascadia Gameplay Systems");

	return true;
}