#pragma once
#include "ModuleList.hpp"

class ScmExtender {
private:
struct ApiCALL {
	static inline HMODULE h = nullptr;

	static void* GetFunctionByName(const char* name) {
		if (!h)
			h = ModuleList().GetByPrefix(L"ScriptCommandExtender");

		if (h) {
			auto a = (void* (*)())GetProcAddress(h, name);

			if (a) {
				return a;
			}
		}
		return nullptr;
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

		return nullptr;
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

		return nullptr;
	}
};

public:
	static inline void AddOneCommand(int32_t command, int8_t (*func)(int32_t*)) {
		ApiCALL::Call(__FUNCTION__, command, func);
	}

	static inline void CollectParams(int16_t count) {
		ApiCALL::Call(__FUNCTION__, count);
	}

	static inline void StoreParams(int16_t count) {
		ApiCALL::Call(__FUNCTION__, count);
	}

	static inline void UpdateCompareFlag(uint8_t flag) {
		ApiCALL::Call(__FUNCTION__, flag);
	}

	static inline bool IsMissionScript() {
		return ApiCALL::CallAndReturn<bool>(__FUNCTION__);
	}
};
