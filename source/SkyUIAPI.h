#pragma once
#include "ModuleList.hpp"

struct ApiCALL {
	static inline HMODULE h = nullptr;

	static void* GetFunctionByName(const char* name) {
		if (!h)
			h = ModuleList().GetByPrefix(L"skyui");

		if (h) {
			auto a = (void* (*)())GetProcAddress(h, name);

			if (a) {
				return a;
			}
		}
		return NULL;
	}

	template <typename... Args>
	static void Call(const char* name, Args... args) {
		void* f = GetFunctionByName(name);
		if (f)
			reinterpret_cast<void(__cdecl*)(Args...)>(f)(args...);
	}

	template <typename Ret, typename... Args>
	static Ret CallAndReturn(const char* name, Args... args) {
		void* f = GetFunctionByName(name);

		if (f)
			return reinterpret_cast<Ret(__cdecl*)(Args...)>(f)(args...);

		return NULL;
	}

	template <typename... Args>
	static void CallMethod(const char* name, Args... args) {
		void* f = GetFunctionByName(name);
		if (f)
			reinterpret_cast<void(__thiscall*)(Args...)>(f)(args...);
	}

	template <typename Ret, typename... Args>
	static Ret CallMethodAndReturn(const char* name, Args... args) {
		void* f = GetFunctionByName(name);

		if (f)
			return reinterpret_cast<Ret(__thiscall*)(Args...)>(f)(args...);

		return NULL;
	}
};

class SkyUI {
public:
	static inline uint32_t GetAlpha(uint32_t a = 255) {
		return ApiCALL::CallAndReturn<uint32_t>(__FUNCTION__, a);
	}

	static inline float GetMenuOffsetX() {
		return ApiCALL::CallAndReturn<float>(__FUNCTION__);
	}

	static inline uint8_t GetCurrentInput() {
		return ApiCALL::CallAndReturn<uint8_t>(__FUNCTION__);
	}

	static inline int32_t GetTimeToWaitBeforeStateChange() {
		return ApiCALL::CallAndReturn<int32_t>(__FUNCTION__);
	}

	static inline uint8_t GetCheckHoverForStandardInput(CMenuManager* _this) {
		return ApiCALL::CallAndReturn<uint8_t, CMenuManager*>(__FUNCTION__, _this);
	}
};
