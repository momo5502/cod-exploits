#pragma once

#ifndef STD_INCLUDED
#error "Missing standard header"
#endif

typedef PVOID *PPVOID;

namespace Utils
{
	namespace NT
	{
		class Module
		{
		public:
			static Module Load(std::string module);
			static Module GetByAddress(void* address);
			static void AddLoadPath(std::string path);

			Module();
			Module(std::string name, bool load = false);
			Module(HMODULE handle);

			Module(const Module& a) : module(a.module) {}

			bool operator!=(const Module &obj) const { return !(*this == obj); };
			bool operator==(const Module &obj) const;

			void unprotect();
			void* getEntryPoint();
			size_t getRelativeEntryPoint();

			bool isValid();
			std::string getName();
			std::string getPath();
			std::string getFolder();
			std::uint8_t* getPtr();
			void free();

			HMODULE getHandle();

			template <typename T>
			T getProc(std::string process)
			{
				if (!this->isValid()) nullptr;
				return reinterpret_cast<T>(GetProcAddress(this->module, process.data()));
			}

			template <typename T>
			std::function<T> get(std::string process)
			{
				if (!this->isValid()) std::function<T>();
				return reinterpret_cast<T*>(this->getProc<void*>(process));
			}

			template<typename T, typename... Args>
			T invoke(std::string process, Args... args)
			{
				auto method = this->get<T(__cdecl)(Args...)>(process);
				if (method) return method(args...);
				return T();
			}

			template<typename T, typename... Args>
			T invokePascal(std::string process, Args... args)
			{
				auto method = this->get<T(__stdcall)(Args...)>(process);
				if (method) return method(args...);
				return T();
			}

			template<typename T, typename... Args>
			T invokeThis(std::string process, void* thisPtr, Args... args)
			{
				auto method = this->get<T(__thiscall)(void*,Args...)>(thisPtr, process);
				if (method) return method(args...);
				return T();
			}

			std::vector<PIMAGE_SECTION_HEADER> getSectionHeaders();

			PIMAGE_NT_HEADERS getNTHeaders();
			PIMAGE_DOS_HEADER getDOSHeader();
			PIMAGE_OPTIONAL_HEADER getOptionalHeader();

			size_t getCodeSize();
			void* getCodeStart();

			void** getIATEntry(std::string moduleName, std::string procName);

#ifdef _DELAY_IMP_VER
			bool delayImport();
#endif

		private:
			HMODULE module;
		};

		void RaiseHardException();
	}
}
