#pragma once

namespace RE
{
	namespace Cascadia
	{
		bool HasVMScript(TESForm* form, const char* scriptName);

		float ConvertPercentageToFloat(std::uint8_t percentage);

		std::string trim(const std::string& str);

		template <typename T>
		void RegisterFunction(Scaleform::GFx::Value* a_dest, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> a_movieRoot, const char* a_func_name)
		{
			Scaleform::GFx::Value fnValue;
			Scaleform::GFx::FunctionHandler* func = nullptr;
			func = new T;

			a_movieRoot->CreateFunction(&fnValue, func);
			a_dest->SetMember(a_func_name, fnValue);
		}

		template<class Ty>
		Ty SafeWrite64Function(uintptr_t a_addr, Ty a_data) {
			std::uint32_t oldProtect;
			void* _d[2];
			memcpy(_d, &a_data, sizeof(a_data));
			size_t len = sizeof(_d[0]);

			REX::W32::VirtualProtect((void*)a_addr, len, REX::W32::PAGE_EXECUTE_READWRITE, &oldProtect);
			Ty oldData;
			memset(&oldData, 0, sizeof(Ty));
			memcpy(&oldData, (void*)a_addr, len);
			memcpy((void*)a_addr, &_d[0], len);
			REX::W32::VirtualProtect((void*)a_addr, len, oldProtect, &oldProtect);
			return oldData;
		}

		extern bool InMenuMode();
		
		std::uint32_t GetAvailableComponentCount(BGSInventoryList* a_list, TESForm* a_form);

		bool IsXPMetervisible();

		BGSKeyword* GetAmmoKeywordStandard(TESAmmo* ammo);
	}
}
