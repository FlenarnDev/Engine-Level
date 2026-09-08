#pragma once

#include "Serialization/Serialization.h"

namespace Cascadia
{
	namespace LevelUp
	{
		class Controller final
		{
		public:
			static Controller& GetSingleton()
			{
				static Controller singleton;
				return singleton;
			}

			Controller(const Controller&) = delete;
			Controller& operator=(const Controller&) = delete;

			static void Install();

			void QueueLevelUp(std::uint32_t a_count = 1);
			void RestorePending(std::uint32_t a_count);
			void Update();
			void OnMenuOpenClose(bool a_opened);
			void CompleteOneLevelUp();

		private:
			enum class State
			{
				idle,
				pending,
				opening,
				open
			};

			Controller() = default;

			[[nodiscard]] bool CanOpen() const;
			[[nodiscard]] bool RequestOpen() const;

			void ScheduleNextCheck(std::chrono::steady_clock::time_point a_now);

			State state_{ State::idle };
			std::uint32_t pendingCount_{ 0 };
			std::chrono::steady_clock::time_point nextCheck_{};
			std::chrono::steady_clock::time_point openingDeadline_{};

			static constexpr std::chrono::milliseconds kRetryInterval{ 500 };
			static constexpr std::chrono::seconds kOpeningTimeout{ 3 };
		};
	}
}
