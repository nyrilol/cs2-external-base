#pragma once

#include <cstddef>
#include <cstdint>

#include "../utils/memory/memory.h"

#include "offsets.h"

class i_global_vars
{
public:
    float real_time;
    int frame_count;
    float absolute_frame_time;
    float absolute_frame_start_time_std_dev;
    int max_clients;
    float interval_per_tick;

private:
    std::byte pad1[0x14];

public:
    float current_time;
    float current_time2;

private:
    std::byte pad2[0xC];

public:
    std::int32_t tick_count;
    float interval_per_tick2;

private:
    std::byte pad3[0x138];

public:
    std::uint64_t current_map;
    std::uint64_t current_map_name;
};

class globals
{
public:
    int curr_tick;
    int last_tick;

    i_global_vars global_vars = {};
public:
    const int get_max_clients()
    {
        return global_vars.max_clients;
    }
};  inline std::unique_ptr<globals> g_globals = std::make_unique<globals>();