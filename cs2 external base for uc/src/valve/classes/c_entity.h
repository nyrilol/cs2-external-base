#pragma once
#include "../netvars/netvars.h"
#include "../../core/offsets.h"
#include "../../utils/memory/types/c_base_handle.h"
#include <string>

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

class c_game_scene_node
{
public:
    // NETVAR(vector, origin, "CGameSceneNode->m_vecAbsOrigin");
};

class c_player_controller
{
public:
    static c_player_controller* get_entity_from_index(int index)
    {
        uintptr_t entry = g_p_memory_system->read<uint64_t>(g_offsets->get_entity_list_offset() + (0x8 * ((index & 0x7FFF) >> 0x9)) + 0x10);
        if (entry == 0) return nullptr;

        return g_p_memory_system->read<c_player_controller*>(entry + 0x78 * (index & 0x1FF));
    }

    [[nodiscard]] std::string name() const
    {
        DWORD64 sanitized_player_name_ptr = g_p_memory_system->read<DWORD64>(reinterpret_cast<DWORD64>(this) + g_netvars->get_offset(fnv1a::hash("CCSPlayerController->m_sSanitizedPlayerName")));
        if (sanitized_player_name_ptr == 0)
            return {};

        return g_p_memory_system->read_string(sanitized_player_name_ptr);
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

    std::string clan_tag()
    {
        DWORD64 clan_tag_ptr = g_p_memory_system->read<DWORD64>(reinterpret_cast<DWORD64>(this) + g_netvars->get_offset(fnv1a::hash("CCSPlayerController->m_szClan")));
        if (clan_tag_ptr == 0)
            return {};

        return g_p_memory_system->read_string(clan_tag_ptr);
    }

    NETVAR(c_handle<c_player_pawn>, pawn, "CCSPlayerController->m_hPlayerPawn");
    NETVAR(bool, is_alive, "CCSPlayerController->m_bPawnIsAlive");
    NETVAR(std::uint32_t, tick_base, "CBasePlayerController->m_nTickBase");
};

class c_player_pawn : public c_entity_instance
{
public:
    static c_player_pawn* get_entity_from_index(int index)
    {
        uintptr_t entry = g_p_memory_system->read<uint64_t>(g_offsets->get_entity_list_offset() + (0x8 * ((index & 0x7FFF) >> 0x9)) + 0x10);
        if (entry == 0) return nullptr;

        return g_p_memory_system->read<c_player_pawn*>(entry + 0x78 * (index & 0x1FF));
    }

    [[nodiscard]] c_base_handle get_ref_handle() const
    {
        c_entity_identity* identity = get_entity();
        if (identity == nullptr)
            return c_base_handle();

        return c_base_handle(identity->get_entry_index(), identity->get_serial_number() - (identity->flags() & 1));
    }

    NETVAR(std::int32_t, health, "C_BaseEntity->m_iHealth");
    NETVAR(c_game_scene_node*, game_scene_node, "C_BaseEntity->m_pGameSceneNode");
    NETVAR(std::int32_t, last_think_tick, "C_BaseEntity->m_nLastThinkTick");
    NETVAR(std::int32_t, max_health, "C_BaseEntity->m_iMaxHealth");
    NETVAR(uint8_t, life_state, "C_BaseEntity->m_lifeState");
    NETVAR(bool, takes_damage, "C_BaseEntity->m_bTakesDamage");
    NETVAR(std::int32_t, take_damage_flags, "C_BaseEntity->m_nTakeDamageFlags");
    NETVAR(bool, is_platform, "C_BaseEntity->m_bIsPlatform");
    NETVAR(float, proxy_random_value, "C_BaseEntity->m_flProxyRandomValue");
    NETVAR(std::int32_t, e_flags, "C_BaseEntity->m_iEFlags");
    NETVAR(uint8_t, water_type, "C_BaseEntity->m_nWaterType");
    NETVAR(bool, interpolate_even_with_no_model, "C_BaseEntity->m_bInterpolateEvenWithNoModel");
    NETVAR(c_handle<c_player_pawn>, scene_object_controller, "C_BaseEntity->m_hSceneObjectController");
    NETVAR(std::int32_t, no_interpolation_tick, "C_BaseEntity->m_nNoInterpolationTick");
    NETVAR(std::int32_t, visibility_no_interpolation_tick, "C_BaseEntity->m_nVisibilityNoInterpolationTick");
    NETVAR(float, anim_time, "C_BaseEntity->m_flAnimTime");
    NETVAR(float, simulation_time, "C_BaseEntity->m_flSimulationTime");
    NETVAR(uint8_t, scene_object_override_flags, "C_BaseEntity->m_nSceneObjectOverrideFlags");
    NETVAR(bool, has_successfully_interpolated, "C_BaseEntity->m_bHasSuccessfullyInterpolated");
    NETVAR(bool, has_added_vars_to_interpolation, "C_BaseEntity->m_bHasAddedVarsToInterpolation");
    NETVAR(bool, render_even_when_not_successfully_interpolated, "C_BaseEntity->m_bRenderEvenWhenNotSuccessfullyInterpolated");
    NETVAR(float, speed, "C_BaseEntity->m_flSpeed");
    NETVAR(uint16_t, ent_client_flags, "C_BaseEntity->m_EntClientFlags");
    NETVAR(bool, client_side_ragdoll, "C_BaseEntity->m_bClientSideRagdoll");
    NETVAR(uint8_t, team_num, "C_BaseEntity->m_iTeamNum");
    NETVAR(std::int32_t, spawn_flags, "C_BaseEntity->m_spawnflags");
    NETVAR(std::int32_t, flags, "C_BaseEntity->m_fFlags");
    NETVAR(c_handle<c_player_pawn>, effect_entity, "C_BaseEntity->m_hEffectEntity");
    NETVAR(c_handle<c_player_pawn>, owner_entity, "C_BaseEntity->m_hOwnerEntity");
    NETVAR(float, water_level, "C_BaseEntity->m_flWaterLevel");
    NETVAR(std::uint32_t, effects, "C_BaseEntity->m_fEffects");
    NETVAR(std::int32_t, ground_body_index, "C_BaseEntity->m_nGroundBodyIndex");
    NETVAR(bool, animated_every_tick, "C_BaseEntity->m_bAnimatedEveryTick");
    NETVAR(uint16_t, think, "C_BaseEntity->m_hThink");
    NETVAR(uint8_t, bbox_vis_flags, "C_BaseEntity->m_fBBoxVisFlags");
    NETVAR(bool, predictable, "C_BaseEntity->m_bPredictable");
    NETVAR(bool, render_with_view_models, "C_BaseEntity->m_bRenderWithViewModels");
    NETVAR(std::int32_t, first_predictable_command, "C_BaseEntity->m_nFirstPredictableCommand");
    NETVAR(std::int32_t, last_predictable_command, "C_BaseEntity->m_nLastPredictableCommand");
    NETVAR(c_handle<c_player_pawn>, old_move_parent, "C_BaseEntity->m_hOldMoveParent");
    NETVAR(std::int32_t, next_script_var_record_id, "C_BaseEntity->m_nNextScriptVarRecordID");
    NETVAR(std::int32_t, data_change_event_ref, "C_BaseEntity->m_DataChangeEventRef");
};