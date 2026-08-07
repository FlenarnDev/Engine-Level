#pragma once

namespace Cascadia
{
	namespace CameraFrustum
	{
		void Install()
		{
			constexpr std::uint8_t bytes[] = { 0x00, 0x24, 0x74, 0x49 }; // 1000000.0f, LE

			REL::Relocation<std::uintptr_t> ratioImm{ ID::NiCamera::ctor, 0x9C };
			REL::WriteSafe(ratioImm.address(), &bytes, sizeof(bytes));
		}
	}
}
