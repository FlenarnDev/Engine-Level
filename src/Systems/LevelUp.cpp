#include "Systems/LevelUp.h"

#include "Shared/SharedFunctions.h"

using namespace RE;

namespace Cascadia
{
	namespace LevelUp
	{
		void Controller::Install()
		{
			if (const auto* task = F4SE::GetTaskInterface())
			{
				task->AddTaskPermanent([]() { Controller::GetSingleton().Update(); });
			}
			else
			{
				REX::CRITICAL("F4SE task interface unavailable; level-up menu will never open.");
			}
		}

		void Controller::QueueLevelUp(std::uint32_t a_count)
		{
			if (a_count == 0)
			{
				return;
			}

			pendingCount_ += a_count;
			Serialization::SetReadyToLevelUp(true);

			if (state_ == State::idle)
			{
				state_ = State::pending;
			}

			ScheduleNextCheck(std::chrono::steady_clock::now());
		}

		void Controller::RestorePending(std::uint32_t a_count)
		{
			if (a_count == 0)
			{
				return;
			}

			pendingCount_ = 1;
			state_ = State::pending;

			ScheduleNextCheck(std::chrono::steady_clock::now());
		}

		void Controller::Update()
		{
			if (state_ == State::idle)
			{
				return;
			}

			const auto now = std::chrono::steady_clock::now();

			switch (state_)
			{
			case State::idle:
				return;

			case State::pending:
				if (now < nextCheck_)
				{
					return;
				}

				if (CanOpen())
				{
					if (RequestOpen())
					{
						state_ = State::opening;
						openingDeadline_ = now + kOpeningTimeout;
					}
					else
					{
						ScheduleNextCheck(now);
					}
				}
				else
				{
					ScheduleNextCheck(now);
				}
				break;

			case State::opening:
				if (now >= openingDeadline_)
				{
					REX::WARN("CASLevelUpMenu did not confirm open within {}s of being queued; retrying.", kOpeningTimeout.count());
					state_ = State::pending;
					ScheduleNextCheck(now);
				}
				break;

			case State::open:
				break;
			}
		}

		void Controller::OnMenuOpenClose(bool a_opened)
		{
			if (a_opened)
			{
				if (state_ == State::opening)
				{
					state_ = State::open;
				}
				return;
			}

			if (pendingCount_ > 0)
			{
				state_ = State::pending;
				ScheduleNextCheck(std::chrono::steady_clock::now());
			}
			else
			{
				state_ = State::idle;
				Serialization::SetReadyToLevelUp(false);
			}
		}

		void Controller::CompleteOneLevelUp()
		{
			if (pendingCount_ > 0)
			{
				--pendingCount_;
			}
		}

		bool Controller::CanOpen() const
		{
			PlayerCharacter* player = PlayerCharacter::GetSingleton();
			if (!player)
			{
				return false;
			}

			if (player->playerInCombat)
			{
				return false;
			}

			if (RE::IsPlayerInDialogue())
			{
				return false;
			}


			if (VATS* vats = VATS::GetSingleton())
			{
				if (vats->mode.any(VATS::VATS_MODE_ENUM::kPlayback))
				{
					return false;
				}
			}

			if (Shared::IsXPMetervisible())
			{
				return false;
			}

			UI* ui = UI::GetSingleton();
			if (!ui)
			{
				return false;
			}

			if (ui->GetMenuOpen("PipboyMenu"))
			{
				return false;
			}

			if (ui->menuMode > 0
				|| ui->GetMenuOpen("CookingMenu")
				|| ui->GetMenuOpen("FaderMenu")
				|| ui->GetMenuOpen("FavoritesMenu")
				|| ui->GetMenuOpen("PowerArmorModMenu")
				|| ui->GetMenuOpen("RobotModMenu")
				|| ui->GetMenuOpen("VATSMenu")
				|| ui->GetMenuOpen("WorkshopMenu"))
			{
				return false;
			}

			if (!ui->menuMap.contains("CASLevelUpMenu"))
			{
				return false;
			}

			if (ui->GetMenuOpen("CASLevelUpMenu"))
			{
				return false;
			}

			return true;
		}

		bool Controller::RequestOpen() const
		{
			UIMessageQueue* uiMessageQueue = UIMessageQueue::GetSingleton();
			if (!uiMessageQueue)
			{
				return false;
			}

			uiMessageQueue->AddMessage("CASLevelUpMenu", UI_MESSAGE_TYPE::kShow);
			return true;
		}

		void Controller::ScheduleNextCheck(std::chrono::steady_clock::time_point a_now)
		{
			nextCheck_ = a_now + kRetryInterval;
		}
	}
}
