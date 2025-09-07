#pragma once

namespace RE
{
	namespace Cascadia
	{
		namespace SprintPatch
		{
			void Install()
			{
				REX::DEBUG("Installed Patch: 'SprintLimit'.");

				float limit = 1100.0f;

				REL::Relocation<std::uintptr_t> sprintLimit{ REL::ID(2664490) };
				REL::WriteSafe(sprintLimit.address(), &limit, sizeof(float));
			}
		}
	}
}