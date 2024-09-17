#pragma once

// fnv1a hash
#include "types/fnv1a.h"

// thread sleep
#include <thread>

// needed for memory operations
#include <Windows.h>
#include <TlHelp32.h>

// just needed for return types, etc.
#include <string>

// for unique
#include <memory>

// map for modules
#include <unordered_map>

// include printing only if debug
#ifdef _DEBUG
#include <cstdio>
#endif

// module struct (https://github.com/Exlodium/CS2-External-Base/blob/d02cc0eeb0fef30d56cbca78a4905288cd301b77/src/memory/Memory.h#L3-L20)
struct module_t
{
    module_t() = default;
    module_t(std::uintptr_t address, std::string path)
        : address(address), path(std::move(path)) {}

    std::uintptr_t address = NULL;
    std::string path = "";

    bool operator==(const module_t& other) const
    {
        return address == other.address && path == other.path;
    }

    bool operator!=(const module_t& other) const
    {
        return !(*this == other);
    }
};

// inspired by cazz
class memory_system
{
private:
    // private used to store variables and functions that aren't needed outside of here for cleanliness
    HANDLE process_handle{ 0 };
    int pid{ 0 };

    // map for modules
    std::unordered_map<std::uint64_t, module_t> modules;

public:
    ~memory_system()
    {
        if (process_handle != NULL)
        {
            CloseHandle(process_handle);
        }
    }

    bool initialize(const char* process_name)
    {
        PROCESSENTRY32 process_entry{ sizeof(PROCESSENTRY32) };

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot == INVALID_HANDLE_VALUE)
        {
            printf("snapshot handle value invalid\n");
            return false;
        }

        if (!Process32First(snapshot, &process_entry))
        {
            printf("Process32First failed\n");
            CloseHandle(snapshot);
            return false;
        }

        do {
            if (strcmp(process_entry.szExeFile, process_name) == 0)
            {
                pid = process_entry.th32ProcessID;
                printf("found %s\n", process_name);
                break;
            }
        } while (Process32Next(snapshot, &process_entry));

        CloseHandle(snapshot);

        if (pid == 0)
        {
            printf("process not found\n");
            return false;
        }

        process_handle = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, false, pid);

        if (process_handle == NULL)
        {
            printf("OpenProcess failed\n");
            return false;
        }

        if (!collect_modules())
        {
            return false;
        }

        return !modules.empty();
    }

    module_t get_module(std::uint64_t hashed_module) const
    {
        if (auto it = modules.find(hashed_module); it != modules.end())
        {
            return it->second;
        }
        printf("module not found: %llu\n", hashed_module);
        return module_t();
    }

    // the reason we're checking for navsystem.dll is because it's the last one to be loaded when cs2 opens
    // also this isn't good because it makes this memory class specific for cs2 (kinda?)
    // but it's easily able to fix, for example for unity games, just put GameAssembly.dll for example
    // inspired by LCShasH (https://www.unknowncheats.me/forum/members/3893134.html)
    bool collect_modules()
    {
        MODULEENTRY32 module_entry{ sizeof(MODULEENTRY32) };

        while (true)
        {
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

            if (snapshot == INVALID_HANDLE_VALUE || snapshot == 0)
            {
                if (snapshot != INVALID_HANDLE_VALUE && snapshot != 0) {
                    CloseHandle(snapshot);
                }
                return false;
            }

            bool found = false;

            if (Module32First(snapshot, &module_entry))
            {
                do
                {
                    auto module_hash = fnv1a::hash(module_entry.szModule);
                    modules.emplace(module_hash, module_t(reinterpret_cast<std::uintptr_t>(module_entry.modBaseAddr), module_entry.szExePath));

                    if (module_hash == fnv1a::hash("navsystem.dll"))
                    {
                        found = true;
                    }

                } while (Module32Next(snapshot, &module_entry));
            }

            CloseHandle(snapshot);

            if (found)
            {
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));

            modules.clear();
        }

        return true;
    }

    template <typename T>
    T read(std::uintptr_t address) const
    {
        T buffer{};
        if (!ReadProcessMemory(process_handle, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(buffer), 0))
        {
            return T{};
        }
        return buffer;
    }

    bool read_raw(uintptr_t address, void* buffer, size_t size)
    {
        SIZE_T bytes_read;
        if (ReadProcessMemory(process_handle, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read))
        {
            return bytes_read == size;
        }
        return false;
    }

    std::string read_string(std::uint64_t address)
    {
        if (!address)
            return "invalid";

        char buf[256] = {};
        if (read_raw(address, &buf, sizeof(buf)))
        {
            buf[sizeof(buf) - 1] = '\0';
            return std::string(buf);
        }
        return "invalid";
    }

    std::uintptr_t relative_address(std::uintptr_t address_bytes, std::uint32_t rva_offset, std::uint32_t rip_offset, std::uint32_t offset = 0) {
        const std::uintptr_t rva = *reinterpret_cast<PLONG>(address_bytes + rva_offset);
        const std::uintptr_t rip = address_bytes + rip_offset;

        if (offset)
            return read<std::uintptr_t>(rva + rip) + offset;

        return rva + rip;
    }

    std::uintptr_t pattern_scan(void* module, const char* signature)
    {
        static auto pattern_to_bytes = [](const char* pattern)
            {
                auto vec_bytes = std::vector<int>{};
                auto start = const_cast<char*>(pattern);
                auto end = const_cast<char*>(pattern) + strlen(pattern);

                for (auto current = start; current < end; ++current)
                {
                    if (*current == '?')
                    {
                        ++current;

                        if (*current == '?')
                            ++current;

                        vec_bytes.push_back(-1);
                    }
                    else
                        vec_bytes.push_back(strtoul(current, &current, 16));
                }
                return vec_bytes;
            };

        PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
        PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(module) + dos_header->e_lfanew);

        DWORD size_of_image = nt_headers->OptionalHeader.SizeOfImage;
        std::vector<int> pattern_bytes = pattern_to_bytes(signature);
        std::uint8_t* scan_bytes = reinterpret_cast<std::uint8_t*>(module);

        size_t size = pattern_bytes.size();
        int* data = pattern_bytes.data();

        for (unsigned long i = 0ul; i < size_of_image - size; ++i)
        {
            bool found = true;
            for (unsigned long j = 0ul; j < size; ++j)
            {
                if (scan_bytes[i + j] != data[j] && data[j] != -1)
                {
                    found = false;
                    break;
                }
            }

            if (found)
                return reinterpret_cast<std::uintptr_t>(&scan_bytes[i]);
        }

        return NULL;
    }

    template <typename T>
    bool write(std::uintptr_t address, const T& value) {
        SIZE_T bytes_written = 0;
        if (!WriteProcessMemory(process_handle, reinterpret_cast<LPVOID>(address), &value, sizeof(value), &bytes_written) || bytes_written != sizeof(value)) {
            return false;
        }
        return true;
    }
};

inline std::unique_ptr<memory_system> g_p_memory_system = std::make_unique<memory_system>();