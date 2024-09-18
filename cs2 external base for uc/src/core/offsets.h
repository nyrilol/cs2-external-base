#pragma once

#include "../utils/memory/memory.h"
#include <memory>
#include <string>
#include <iostream>

class offset_manager
{
private:
    std::uintptr_t entity_list_ = 0x0;
    std::uintptr_t global_vars_ = 0x0;
    std::uintptr_t view_matrix_ = 0x0;
    std::uintptr_t local_player_controller_ = 0x0;

    bool find_offset_from_signature(
        std::uintptr_t& return_offset,
        std::uintptr_t address,
        std::uintptr_t module_base_address,
        std::uintptr_t wanted_module_address
    ) const
    {
        return_offset = wanted_module_address + (address - module_base_address);
        if (return_offset == 0)
        {
            printf("failed to find offset");
            return false;
        }
        return true;
    }

public:
    std::uintptr_t get_entity_list_offset() const
    {
        return g_p_memory_system->read<std::uintptr_t>(entity_list_);
    }

    std::uintptr_t get_global_vars_offset() const
    {
        return g_p_memory_system->read<std::uintptr_t>(global_vars_);
    }

    std::uintptr_t get_local_player_controller_offset() const
    {
        return g_p_memory_system->read<std::uintptr_t>(local_player_controller_);
    }

    std::uintptr_t get_view_matrix_offset() const
    {
        return g_p_memory_system->read<std::uintptr_t>(view_matrix_);
    }

    bool initialize()
    {
        const auto client_dll_info = g_p_memory_system->get_module(fnv1a::hash("client.dll"));
        const std::string client_dll_path = client_dll_info.path;
        HMODULE h_client_dll = LoadLibraryExA(client_dll_path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);

        if (h_client_dll == nullptr)
        {
            printf("failed to load client_dll");
            return false;
        }

        const std::uintptr_t client_address = reinterpret_cast<std::uintptr_t>(h_client_dll);

        const auto pattern_scan_result = g_p_memory_system->pattern_scan(h_client_dll, "48 8B 0D ? ? ? ? 48 89 7C 24 ? 8B FA C1 EB");
        if (!find_offset_from_signature(
            entity_list_,
            g_p_memory_system->relative_address(pattern_scan_result, 0x3, 0x7),
            client_address,
            client_dll_info.address
        ))
        {
            FreeLibrary(h_client_dll);
            return false;
        }

        const auto pattern_scan_result_1 = g_p_memory_system->pattern_scan(h_client_dll, "48 89 0D ? ? ? ? 48 89 41");
        if (!find_offset_from_signature(
            global_vars_,
            g_p_memory_system->relative_address(pattern_scan_result_1, 0x3, 0x7),
            client_address,
            client_dll_info.address
        ))
        {
            FreeLibrary(h_client_dll);
            return false;
        }

        const auto pattern_scan_result_2 = g_p_memory_system->pattern_scan(h_client_dll, "48 8B 05 ? ? ? ? 48 85 C0 74 53");
        if (!find_offset_from_signature(
            local_player_controller_,
            g_p_memory_system->relative_address(pattern_scan_result_2, 0x3, 0x7),
            client_address,
            client_dll_info.address
        ))
        {
            FreeLibrary(h_client_dll);
            return false;
        }

        const auto pattern_scan_result_3 = g_p_memory_system->pattern_scan(h_client_dll, "48 8D 0D ? ? ? ? 48 C1 E0 06");
        if (!find_offset_from_signature(
            view_matrix_,
            g_p_memory_system->relative_address(pattern_scan_result_3, 0x3, 0x7),
            client_address,
            client_dll_info.address
        ))
        {
            FreeLibrary(h_client_dll);
            return false;
        }

        FreeLibrary(h_client_dll);
        return true;
    }
};

inline std::unique_ptr<offset_manager> g_offsets = std::make_unique<offset_manager>();