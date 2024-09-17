#pragma once

#include <cstdint>
#include <limits>
#include <cstddef>

#include "../memory.h"

class utl_memory_pool
{
public:
    int count() const
    {
        return blocks_allocated_;
    }

    int peak_count() const
    {
        return peak_alloc_;
    }

    int block_size() const
    {
        return block_size_;
    }

protected:
    class blob
    {
    public:
        blob* prev;
        blob* next;
        int num_bytes;
        char data[1];
        char padding[3];
    };

    int block_size_;
    int blocks_per_blob_;
    int grow_mode_;
    int blocks_allocated_;
    int peak_alloc_;
    unsigned short alignment_;
    unsigned short num_blobs_;
    void* head_of_free_list_;
    void* alloc_owner_;
    blob blob_head_;
};

using utl_ts_hash_handle_t = std::uintptr_t;

inline unsigned hash_int_conventional(const int n)
{
    unsigned hash = 0xAAAAAAAA + (n & 0xFF);
    hash = (hash << 5) + hash + ((n >> 8) & 0xFF);
    hash = (hash << 5) + hash + ((n >> 16) & 0xFF);
    hash = (hash << 5) + hash + ((n >> 24) & 0xFF);
    return hash;
}

template<int bucket_count, class key_t = std::uintptr_t>
class utl_ts_hash_generic_hash
{
public:
    static int hash(const key_t& key, int bucket_mask)
    {
        int hash_value = hash_int_conventional(std::uintptr_t(key));
        if (bucket_count <= UINT16_MAX)
        {
            hash_value ^= (hash_value >> 16);
        }
        if (bucket_count <= UINT8_MAX)
        {
            hash_value ^= (hash_value >> 8);
        }
        return (hash_value & bucket_mask);
    }

    static bool compare(const key_t& lhs, const key_t& rhs)
    {
        return lhs == rhs;
    }
};

template<class element_t, int bucket_count, class key_t = std::uintptr_t,
    class hash_funcs = utl_ts_hash_generic_hash<bucket_count, key_t>, int alignment = 0>
class utl_ts_hash
{
    static constexpr int bucket_mask = bucket_count - 1;
public:
    static constexpr utl_ts_hash_handle_t invalid_handle()
    {
        return static_cast<utl_ts_hash_handle_t>(0);
    }

    utl_ts_hash_handle_t find(key_t key)
    {
        int bucket_index = hash_funcs::hash(key, bucket_count);
        const hash_bucket_t& hash_bucket = buckets_[bucket_index];
        utl_ts_hash_handle_t hash_handle = find_internal(key, hash_bucket.first, nullptr);
        return hash_handle ? hash_handle : find_internal(key, hash_bucket.first_uncommitted, hash_bucket.first);
    }

    int count() const
    {
        return entry_memory_.count();
    }

    int get_elements(int first_element, int element_count, utl_ts_hash_handle_t* handles) const
    {
        int index = 0;
        for (int bucket_index = 0; bucket_index < bucket_count; bucket_index++)
        {
            const hash_bucket_t& hash_bucket = buckets_[bucket_index];
            hash_fixed_data_t* element = hash_bucket.first_uncommitted;

            for (; element; element = element->get_next())
            {
                if (--first_element >= 0)
                    continue;

                handles[index++] = reinterpret_cast<utl_ts_hash_handle_t>(element);

                if (index >= element_count)
                    return index;
            }
        }
        return index;
    }

    element_t element(utl_ts_hash_handle_t hash_handle)
    {
        return reinterpret_cast<hash_fixed_data_t*>(hash_handle)->get_data();
    }

    const element_t& element(utl_ts_hash_handle_t hash_handle) const
    {
        return reinterpret_cast<hash_fixed_data_t*>(hash_handle)->data_;
    }

    element_t& operator[](utl_ts_hash_handle_t hash_handle)
    {
        return reinterpret_cast<hash_fixed_data_t*>(hash_handle)->data_;
    }

    const element_t& operator[](utl_ts_hash_handle_t hash_handle) const
    {
        return reinterpret_cast<hash_fixed_data_t*>(hash_handle)->data_;
    }

    key_t get_id(utl_ts_hash_handle_t hash_handle) const
    {
        return reinterpret_cast<hash_fixed_data_t*>(hash_handle)->key_;
    }

private:
    template<typename data_t>
    struct hash_fixed_data_internal_t
    {
        key_t key_;
        hash_fixed_data_internal_t<data_t>* next_;
        data_t data_;

        data_t get_data()
        {
            return g_p_memory_system->read<data_t>(reinterpret_cast<std::uintptr_t>(this) + 0x10);
        }

        hash_fixed_data_internal_t<data_t>* get_next()
        {
            return g_p_memory_system->read<hash_fixed_data_internal_t<data_t>*>(reinterpret_cast<std::uintptr_t>(this) + 0x8);
        }
    };

    using hash_fixed_data_t = hash_fixed_data_internal_t<element_t>;

    struct hash_bucket_t
    {
    private:
        [[maybe_unused]]
        std::byte padding[0x18];
    public:
        hash_fixed_data_t* first;
        hash_fixed_data_t* first_uncommitted;
    };

    utl_ts_hash_handle_t find_internal(key_t key, hash_fixed_data_t* first_element, hash_fixed_data_t* last_element)
    {
        for (hash_fixed_data_t* element = first_element; element != last_element; element = element->next_)
        {
            if (hash_funcs::compare(element->key_, key))
                return reinterpret_cast<utl_ts_hash_handle_t>(element);
        }
        return invalid_handle();
    }

    utl_memory_pool entry_memory_;
    char padding_[0x40];
    hash_bucket_t buckets_[bucket_count];
    bool needs_commit_;
};