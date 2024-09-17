#pragma once
#include "../netvars/netvars.h"
#include "../../core/offsets.h"
#include "../../utils/memory/types/c_base_handle.h"
#include <string>

// Entity Identity Class
class c_entity_identity
{
public:
    std::string get_designer_name() noexcept
    {
        std::string buffer(32, '\0');

        uint64_t designer_name_ptr = g_p_memory_system->read<uint64_t>(reinterpret_cast<uint64_t>(this) + 0x20);
        if (designer_name_ptr == 0)
            return {};

        buffer = g_p_memory_system->read_string(designer_name_ptr);
        return buffer;
    }

    [[nodiscard]] bool is_valid() const
    {
        return get_index() != INVALID_EHANDLE_INDEX;
    }

    [[nodiscard]] int get_entry_index() const
    {
        if (!is_valid())
            return ENT_ENTRY_MASK;

        return get_index() & ENT_ENTRY_MASK;
    }

    [[nodiscard]] int get_serial_number() const
    {
        return get_index() >> NUM_SERIAL_NUM_SHIFT_BITS;
    }

    OFFSET(std::uint32_t, get_index, 0x10);
    NETVAR(std::uint32_t, flags, "CEntityIdentity->m_flags");

private:
    [[nodiscard]] std::uint32_t get_index() const
    {
        return g_p_memory_system->read<std::uint32_t>(reinterpret_cast<uint64_t>(this) + 0x10);
    }
};

// Entity Instance Class
class c_entity_instance
{
public:
    [[nodiscard]] c_entity_identity* get_entity() const
    {
        return g_p_memory_system->read<c_entity_identity*>(reinterpret_cast<uint64_t>(this) + 0x10);
    }

    std::string get_schema_name()
    {
        std::uintptr_t entity_identity_ptr = g_p_memory_system->read<std::uintptr_t>(reinterpret_cast<uint64_t>(this) + 0x10);
        if (entity_identity_ptr == 0)
            return "invalid";

        std::uintptr_t entity_class_info_ptr = g_p_memory_system->read<std::uintptr_t>(entity_identity_ptr + 0x08);
        std::uintptr_t pointer1 = g_p_memory_system->read<std::uintptr_t>(entity_class_info_ptr + 0x28);
        std::uintptr_t pointer2 = g_p_memory_system->read<std::uintptr_t>(pointer1 + 0x08);

        std::string schema_name = g_p_memory_system->read_string(pointer2);
        return schema_name.empty() ? "invalid" : schema_name;
    }

    OFFSET(c_entity_identity*, get_entity, 0x10);
};

// Entity Class
class c_entity : public c_entity_instance
{
public:
    static c_entity* get_entity_from_index(int index)
    {
        uintptr_t entry = g_p_memory_system->read<uint64_t>(g_offsets->get_entity_list_offset() + (0x8 * ((index & 0x7FFF) >> 0x9)) + 0x10);
        if (entry == 0) return nullptr;

        return g_p_memory_system->read<c_entity*>(entry + 0x78 * (index & 0x1FF));
    }

    std::string get_schema_name()
    {
        uintptr_t entity_identity_ptr = g_p_memory_system->read<std::uintptr_t>(reinterpret_cast<uint64_t>(this) + 0x10);
        if (entity_identity_ptr == 0)
            return "invalid";

        uintptr_t entity_class_info_ptr = g_p_memory_system->read<std::uintptr_t>(entity_identity_ptr + 0x08);
        uintptr_t pointer1 = g_p_memory_system->read<std::uintptr_t>(entity_class_info_ptr + 0x28);
        uintptr_t pointer2 = g_p_memory_system->read<std::uintptr_t>(pointer1 + 0x08);

        std::string schema_name = g_p_memory_system->read_string(pointer2);
        return schema_name.empty() ? "invalid" : schema_name;
    }

    [[nodiscard]] c_base_handle get_ref_handle() const
    {
        c_entity_identity* identity = get_entity();
        if (identity == nullptr)
            return c_base_handle();

        return c_base_handle(identity->get_entry_index(), identity->get_serial_number() - (identity->flags() & 1));
    }

    [[nodiscard]] std::string name() const
    {
        DWORD64 sanitized_player_name_ptr = g_p_memory_system->read<DWORD64>(reinterpret_cast<DWORD64>(this) + g_netvars->get_offset(fnv1a::hash("CCSPlayerController->m_sSanitizedPlayerName")));
        if (sanitized_player_name_ptr == 0)
            return {};

        return g_p_memory_system->read_string(sanitized_player_name_ptr);
    }

    // Controller related
    NETVAR(c_handle<c_entity>, pawn, "CCSPlayerController->m_hPlayerPawn");

    // Pawn related
    NETVAR(std::int32_t, health, "C_BaseEntity->m_iHealth");
};