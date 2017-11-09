#include "STDInclude.hpp"

namespace Utils
{
	namespace NT
	{
		Module Module::Load(std::string module)
		{
			return Module(module, true);
		}

		Module Module::GetByAddress(void* address)
		{
			HMODULE module = nullptr;
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(address), &module);
			return Module(module);
		}

		void Module::AddLoadPath(std::string path)
		{
			char outBuffer[MAX_PATH] = { 0 };
			if (_fullpath(outBuffer, path.data(), sizeof(outBuffer)))
			{
				path = outBuffer;
				AddDllDirectory(std::wstring(path.begin(), path.end()).data());
				SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
			}
		}

		Module::Module()
		{
			this->module = GetModuleHandleA(nullptr);
		}

		Module::Module(std::string name, bool load)
		{
			if (load) this->module = LoadLibraryA(name.data());
			else this->module = GetModuleHandleA(name.data());
		}

		Module::Module(HMODULE handle)
		{
			this->module = handle;
		}

		bool Module::operator==(const Module &obj) const
		{
			return this->module == obj.module;
		}

		PIMAGE_NT_HEADERS Module::getNTHeaders()
		{
			if (!this->isValid()) return nullptr;
			return reinterpret_cast<PIMAGE_NT_HEADERS>(this->getPtr() + this->getDOSHeader()->e_lfanew);
		}

		PIMAGE_DOS_HEADER Module::getDOSHeader()
		{
			return reinterpret_cast<PIMAGE_DOS_HEADER>(this->getPtr());
		}

		PIMAGE_OPTIONAL_HEADER Module::getOptionalHeader()
		{
			if (!this->isValid()) return nullptr;
			return &this->getNTHeaders()->OptionalHeader;
		}

		std::vector<PIMAGE_SECTION_HEADER> Module::getSectionHeaders()
		{
			std::vector<PIMAGE_SECTION_HEADER> headers;

			auto ntheaders = this->getNTHeaders();
			auto section = IMAGE_FIRST_SECTION(ntheaders);

			for (uint16_t i = 0; i < ntheaders->FileHeader.NumberOfSections; ++i, ++section)
			{
				if (section) headers.push_back(section);
				else OutputDebugStringA("There was an invalid section :O");
			}

			return headers;
		}

		std::uint8_t* Module::getPtr()
		{
			return reinterpret_cast<std::uint8_t*>(this->module);
		}

		void Module::unprotect()
		{
			if (!this->isValid()) return;

			DWORD protection;
			VirtualProtect(this->getPtr(), this->getOptionalHeader()->SizeOfImage, PAGE_EXECUTE_READWRITE, &protection);
		}

		size_t Module::getRelativeEntryPoint()
		{
			if (!this->isValid()) return 0;
			return this->getNTHeaders()->OptionalHeader.AddressOfEntryPoint;
		}

		void* Module::getEntryPoint()
		{
			if (!this->isValid()) return nullptr;
			return this->getPtr() + this->getRelativeEntryPoint();
		}

		bool Module::isValid()
		{
			return this->module != nullptr && this->getDOSHeader()->e_magic == IMAGE_DOS_SIGNATURE;
		}

		std::string Module::getName()
		{
			if (!this->isValid()) return "";

			std::string path = this->getPath();
			auto pos = path.find_last_of("/\\");
			if (pos == std::string::npos) return path;

			return path.substr(pos + 1);
		}

		std::string Module::getFolder()
		{
			if (!this->isValid()) return "";

			std::string path = this->getPath();
			auto pos = path.find_last_of("/\\");
			if (pos == std::string::npos) return ".\\";

			return path.substr(0, pos + 1);
		}

		std::string Module::getPath()
		{
			if (!this->isValid()) return "";

			char name[MAX_PATH] = { 0 };
			GetModuleFileNameA(this->module, name, sizeof name);

			return name;
		}

		void Module::free()
		{
			if (this->isValid())
			{
				FreeLibrary(this->module);
				this->module = nullptr;
			}
		}

		HMODULE Module::getHandle()
		{
			return this->module;
		}

		size_t Module::getCodeSize()
		{
			if (this->isValid())
			{
				auto headers = this->getSectionHeaders();
				for (auto& header : headers)
				{
					if (LPSTR(header->Name) == ".text"s)
					{
						return header->SizeOfRawData;
					}
				}
			}

			return 0;
		}

		void* Module::getCodeStart()
		{
			if (this->isValid())
			{
				auto headers = this->getSectionHeaders();
				for (auto& header : headers)
				{
					if (LPSTR(header->Name) == ".text"s)
					{
						return this->getPtr() + header->VirtualAddress;
					}
				}
			}

			return nullptr;
		}

		void** Module::getIATEntry(std::string moduleName, std::string procName)
		{
			if (!this->isValid()) return nullptr;

			Module otherModule(moduleName);
			if (!otherModule.isValid()) return nullptr;

			void* targetFunction = otherModule.getProc<void*>(procName);
			if (!targetFunction) return nullptr;

			auto* header = this->getOptionalHeader();
			if (!header) return nullptr;

			auto* importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(this->getPtr() + header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

			while (importDescriptor->Name)
			{
				if (!_stricmp(reinterpret_cast<char*>(this->getPtr() + importDescriptor->Name), moduleName.data()))
				{
					auto* originalThunkData = reinterpret_cast<PIMAGE_THUNK_DATA>(importDescriptor->OriginalFirstThunk + this->getPtr());
					auto* thunkData = reinterpret_cast<PIMAGE_THUNK_DATA>(importDescriptor->FirstThunk + this->getPtr());

					while(originalThunkData->u1.AddressOfData)
					{
						LPSTR name = nullptr;

						if (IMAGE_SNAP_BY_ORDINAL(originalThunkData->u1.Ordinal))
						{
							name = MAKEINTRESOURCEA(IMAGE_ORDINAL(originalThunkData->u1.Ordinal));
						}
						else
						{
							name = PIMAGE_IMPORT_BY_NAME(this->getPtr() + originalThunkData->u1.ForwarderString)->Name;
						}

						if (GetProcAddress(otherModule.module, name) == targetFunction)
						{
							return reinterpret_cast<void**>(&thunkData->u1.Function);
						}

						++originalThunkData;
						++thunkData;
					}

					break;
				}

				++importDescriptor;
			}

			return nullptr;
		}

#ifdef _DELAY_IMP_VER
		bool Module::delayImport()
		{
			return SUCCEEDED(__HrLoadAllImportsForDll(this->getName().data()));
		}
#endif

		void RaiseHardException()
		{
			int data = false;
			Utils::NT::Module ntdll("ntdll.dll");
			ntdll.invokePascal<NTSTATUS>("RtlAdjustPrivilege", 19, true, false, &data);
			ntdll.invokePascal<NTSTATUS>("NtRaiseHardError", 0xC000007B, 0, nullptr, nullptr, 6, &data);
		}
	}
}
