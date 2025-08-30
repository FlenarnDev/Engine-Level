#pragma once

namespace RE
{
	namespace Cascadia
	{
		namespace MoonRotation
		{
			void Install()
			{
				constexpr std::uint8_t bytes[] = { 0x03 };

				REL::Relocation<std::uintptr_t> moonRotationAddress{ ID::Moon::Init, 0x1F7 };
				REL::WriteSafe(moonRotationAddress.address(), &bytes, sizeof(bytes));
			}
		}
	}
}