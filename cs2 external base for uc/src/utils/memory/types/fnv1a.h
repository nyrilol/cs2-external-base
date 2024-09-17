// source: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

#pragma once
#include <string>
#include <cstdint>

using fnv1a_t = std::uint64_t;

namespace fnv1a
{
    constexpr std::uint64_t prime = 0x00000100000001B3;
    constexpr std::uint64_t offset = 0xcbf29ce484222325;

    inline fnv1a_t hash(const std::string_view& string)
    {
        fnv1a_t hash = offset;
        for (char c : string)
        {
            hash ^= static_cast<fnv1a_t>(c);
            hash *= prime;
        }
        return hash;
    }
}