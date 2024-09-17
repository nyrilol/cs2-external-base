#pragma once

// unique ptr
#include <memory>

// utlhash or something thanks exlodium or mhalaider or whoever the fuck made this
#include "../../utils/memory/types/utlhash.h"

// memory
#include "../../utils/memory/memory.h"

class schema_entity {
public:
    std::string get_name(std::uintptr_t offset = 0x8) const {
        auto address = g_p_memory_system->read<std::uintptr_t>(reinterpret_cast<std::uintptr_t>(this) + offset);
        return g_p_memory_system->read_string(address);
    }

    schema_entity* get_schema_type(std::uintptr_t offset = 0x8) const {
        return g_p_memory_system->read<schema_entity*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

    std::uint32_t get_offset(std::uintptr_t offset = 0x10) const {
        return g_p_memory_system->read<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

    std::uint32_t get_size(std::uintptr_t offset = 0x18) const {
        return g_p_memory_system->read<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

    std::uint16_t get_num_fields(std::uintptr_t offset = 0x1C) const {
        return g_p_memory_system->read<std::uint16_t>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

    schema_entity* get_fields(std::uintptr_t offset = 0x28) const {
        return g_p_memory_system->read<schema_entity*>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }

    std::string get_project_name(std::uintptr_t offset = 0x10) const {
        return g_p_memory_system->read_string(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
};

class schema_system_type_scope {
public:
    utl_ts_hash<schema_entity*, 256, unsigned int> get_bindings_table(std::uintptr_t offset = 0x500) const {
        return g_p_memory_system->read<utl_ts_hash<schema_entity*, 256, unsigned int>>(reinterpret_cast<std::uintptr_t>(this) + offset);
    }
};

class netvar_manager {
private:
    std::unordered_map<fnv1a_t, std::uintptr_t> netvars_storage;

public:
    bool initialize();

    [[nodiscard]] std::uintptr_t get_offset(const fnv1a_t hashed_name) const {
        const auto it = netvars_storage.find(hashed_name);
        return it != netvars_storage.end() ? it->second : 0;
    }
};  inline std::unique_ptr<netvar_manager> g_netvars = std::make_unique<netvar_manager>();

#define NETVAR(type, name, str) \
[[nodiscard]] __forceinline type name() noexcept { \
    static const std::uintptr_t offset = g_netvars->get_offset(fnv1a::hash(str)); \
    return g_p_memory_system->read<type>(reinterpret_cast<std::uintptr_t>(this) + offset); \
}

#define ADD_OFFSET(type, name, offset) \
[[nodiscard]] __forceinline type name() noexcept { \
    static const std::uintptr_t internal_offset = offset; \
    return g_p_memory_system->read<type>(reinterpret_cast<std::uintptr_t>(this) + internal_offset); \
}

#define OFFSET(type, name, offset) ADD_OFFSET(type, name, offset)