#pragma once

#include "../valve/classes/c_entity.h"
#include "../core/globals.h"
#include <unordered_map>

class entity_list
{
private:
    struct entity_t
    {
        c_player_controller* entity = nullptr;
        int index = 0;
        std::string name = "unknown";

        entity_t() = default;

        entity_t(c_player_controller* e, int i, const std::string& n)
            : entity(e), index(i), name(n) {}
    };

    std::unordered_map<int, entity_t> entity_map;

    std::string get_entity_name(c_player_controller* ent)
    {
        static std::unordered_map<c_player_controller*, std::string> name_cache;

        if (name_cache.find(ent) == name_cache.end())
        {
            name_cache[ent] = ent->name();
        }

        return name_cache[ent];
    }

    bool is_entity_alive(c_player_controller* ent)
    {
        return ent != nullptr && ent->is_alive();
    }

public:
    std::vector<entity_t> entity_vec;

    void update()
    {
        entity_map.clear();
        entity_vec.clear();

        g_globals->global_vars = g_p_memory_system->read<i_global_vars>(g_offsets->get_global_vars_offset());

        for (int idx = 1; idx < g_globals->global_vars.max_clients; ++idx)
        {
            c_player_controller* ent = c_player_controller::get_entity_from_index(idx);
            if (!ent || !is_entity_alive(ent)) continue;

            if (ent->get_schema_name() == "CCSPlayerController")
            {
                auto it = entity_map.find(idx);
                if (it == entity_map.end())
                {
                    entity_map[idx] = entity_t(ent, idx, get_entity_name(ent));
                }
                else
                {
                    entity_t& existing = it->second;
                    if (existing.entity != ent || existing.name != get_entity_name(ent))
                    {
                        existing.entity = ent;
                        existing.name = get_entity_name(ent);
                    }
                }
            }
        }

        for (const auto& [index, entity] : entity_map)
        {
            entity_vec.push_back(entity);
        }
    }
};

inline std::unique_ptr<entity_list> g_entity_list = std::make_unique<entity_list>();