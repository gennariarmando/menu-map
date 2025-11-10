#pragma once
#include "ModuleList.hpp"

class SaveExtender {
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

			return {};
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

			return {};
		}
	};

public:
	template <typename T>
	static inline void Serialize(const std::string& slot, T in, size_t size) {
		ApiCALL::Call(__FUNCTION__, slot, in, size);
	}

	template <typename T>
	static inline void Retrieve(const std::string& slot, T* out, size_t size) {
		ApiCALL::Call(__FUNCTION__, slot, out, size);
	}

	static inline void OnSavingEvent(void (*func)()) {
		ApiCALL::Call(__FUNCTION__, func);
	}

	static inline void OnLoadingEvent(void (*func)()) {
		ApiCALL::Call(__FUNCTION__, func);
	}
};
