#pragma once
#include <cstdint>
#include <cassert>

#define INVALID_EHANDLE_INDEX 0xFFFFFFFF
#define ENT_ENTRY_MASK 0x7FFF
#define NUM_SERIAL_NUM_SHIFT_BITS 15
#define ENT_MAX_NETWORKED_ENTRY 16384

class c_player_pawn;
class c_base_handle
{
public:
    c_base_handle() noexcept :
        index_(INVALID_EHANDLE_INDEX) { }

    c_base_handle(int entry_index, int serial_number) noexcept
    {
        assert(entry_index >= 0 && (entry_index & ENT_ENTRY_MASK) == entry_index);
        assert(serial_number >= 0 && serial_number < (1 << NUM_SERIAL_NUM_SHIFT_BITS));

        index_ = entry_index | (serial_number << NUM_SERIAL_NUM_SHIFT_BITS);
    }

    bool operator!=(const c_base_handle& other) const noexcept
    {
        return index_ != other.index_;
    }

    bool operator==(const c_base_handle& other) const noexcept
    {
        return index_ == other.index_;
    }

    bool operator<(const c_base_handle& other) const noexcept
    {
        return index_ < other.index_;
    }

    [[nodiscard]] bool is_valid() const noexcept
    {
        return index_ != INVALID_EHANDLE_INDEX;
    }

    [[nodiscard]] int get_entry_index() const noexcept
    {
        return static_cast<int>(index_ & ENT_ENTRY_MASK);
    }

    [[nodiscard]] int get_serial_number() const noexcept
    {
        return static_cast<int>(index_ >> NUM_SERIAL_NUM_SHIFT_BITS);
    }

    [[nodiscard]] c_player_pawn* get() const;

private:
    uint32_t index_;
};

template <typename T>
class c_handle : public c_base_handle
{
public:
    using c_base_handle::c_base_handle; // Inherit constructors

    [[nodiscard]] T* get() const noexcept
    {
        return static_cast<T*>(c_base_handle::get());
    }
};