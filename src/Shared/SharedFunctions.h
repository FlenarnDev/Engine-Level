#pragma once

namespace RE
{
	namespace Cascadia
	{
		std::string FloatToPreciseString(float value, std::uint32_t precision);

		bool HasVMScript(TESForm* form, const char* scriptName);

		float ConvertPercentageToFloat(std::uint8_t percentage);

		std::string trim(const std::string& str);

		template <typename T>
		void RegisterFunction(Scaleform::GFx::Value* dest, Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> movieRoot, const char* func_name)
		{
			Scaleform::GFx::Value fnValue;
			Scaleform::GFx::FunctionHandler* func = nullptr;
			func = new T;

			movieRoot->CreateFunction(&fnValue, func);
			dest->SetMember(func_name, fnValue);
		}

		template<class Ty>
		Ty SafeWrite64Function(uintptr_t addr, Ty data) {
			std::uint32_t oldProtect;
			void* _d[2];
			memcpy(_d, &data, sizeof(data));
			size_t len = sizeof(_d[0]);

			REX::W32::VirtualProtect((void*)addr, len, REX::W32::PAGE_EXECUTE_READWRITE, &oldProtect);
			Ty olddata;
			memset(&olddata, 0, sizeof(Ty));
			memcpy(&olddata, (void*)addr, len);
			memcpy((void*)addr, &_d[0], len);
			REX::W32::VirtualProtect((void*)addr, len, oldProtect, &oldProtect);
			return olddata;
		}

		template <class... Ts>
		static Scaleform::GFx::Value InvokeAS3(Scaleform::GFx::Value& value, const std::string& funcName, Ts&&... inputs)
		{
			Scaleform::GFx::Value result;

			std::array<Scaleform::GFx::Value, sizeof...(inputs)> args;

			int i = 0;
			([&] {args[i++] = inputs; }(), ...);

			value.Invoke(funcName.c_str(), &result, args.data(), sizeof...(inputs));

			return result;
		}


		template <class... Ts>
		inline Scaleform::GFx::Value InvokeAS3(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> root, const std::string& funcName, Ts&&... inputs)
		{
			Scaleform::GFx::Value result;

			std::array<Scaleform::GFx::Value, sizeof...(inputs)> args;

			int i = 0;
			([&] { args[i++] = inputs; }(), ...);

			root->Invoke(funcName.c_str(), &result, args.data(), sizeof...(inputs));

			return result;
		}

		inline Scaleform::GFx::Value GetMemberAS3(Scaleform::GFx::Value value, const std::string& name)
		{
			Scaleform::GFx::Value result;
			value.GetMember(name.c_str(), &result);

			return result;
		}

		inline Scaleform::GFx::Value GetVariableAS3(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> root, const std::string& name)
		{
			Scaleform::GFx::Value result;
			root->GetVariable(&result, name.c_str());

			return result;
		}

		inline void SetMemberAS3(Scaleform::GFx::Value value, const std::string& name, Scaleform::GFx::Value arg)
		{
			value.SetMember(name.c_str(), arg);
		}

		inline void SetVariableAS3(Scaleform::Ptr<Scaleform::GFx::ASMovieRootBase> root, const std::string& name, Scaleform::GFx::Value arg)
		{
			root->SetVariable(name.c_str(), arg);
		}

		extern bool InMenuMode();
		
		std::uint32_t GetAvailableComponentCount(BGSInventoryList* a_list, TESForm* a_form);

		bool IsXPMetervisible();

		BGSKeyword* GetAmmoKeywordStandard(TESAmmo* ammo);
	}
}
