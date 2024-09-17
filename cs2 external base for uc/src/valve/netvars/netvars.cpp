#include "netvars.h"

bool netvar_manager::initialize()
{
    module_t schema_module = g_p_memory_system->get_module(fnv1a::hash("schemasystem.dll"));
    HMODULE schema_system_dll = LoadLibraryExA(schema_module.path.c_str(), 0, DONT_RESOLVE_DLL_REFERENCES);
    if (!schema_system_dll) {
        printf("failed to load schema system dll\n");
        return false;
    }

    std::uintptr_t base_interface_ptr = schema_module.address +
        (g_p_memory_system->relative_address(
            g_p_memory_system->pattern_scan(schema_system_dll, "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 4C 89 74 24"), 0x3, 0x7)
            - reinterpret_cast<std::uintptr_t>(schema_system_dll));

    FreeLibrary(schema_system_dll);

    if (!base_interface_ptr) {
        printf("failed to get base interface pointer\n");
        return false;
    }

    std::uintptr_t list_element_ptr = g_p_memory_system->read<std::uintptr_t>(base_interface_ptr + 0x190);

    auto type_scope = g_p_memory_system->read<schema_system_type_scope*>(list_element_ptr + (0x10 * 8));
    auto table = type_scope->get_bindings_table();

    auto elements = std::make_unique<utl_ts_hash_handle_t[]>(table.count());
    const int element_count = table.get_elements(0, table.count(), elements.get());
        
    for (int element_index = 0; element_index < element_count; ++element_index) {
        const auto element = elements[element_index];
        if (!element) continue;

        const auto class_binding = table.element(element);
        if (!class_binding || !class_binding->get_num_fields()) continue;

        auto fields = class_binding->get_fields();
        std::string class_name = class_binding->get_name();

        for (int field_index = 0, num_fields = class_binding->get_num_fields(); field_index < num_fields; ++field_index) {
            if (!fields) continue;

            std::string field_class = class_name + "->" + fields->get_name(0x0);
            netvars_storage.emplace(fnv1a::hash(field_class), fields->get_offset());

            // printf("class: %s, field: %s, offset: %lu\n", class_name.c_str(), fields->get_name(0x0).c_str(), fields->get_offset());

            fields = reinterpret_cast<schema_entity*>(reinterpret_cast<std::uintptr_t>(fields) + 0x20);
        }
    }

    return true;
}