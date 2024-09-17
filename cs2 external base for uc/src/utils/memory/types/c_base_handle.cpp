#include "c_base_handle.h"

#include "../../../valve/classes/c_entity.h"

c_entity* c_base_handle::get() const
{
    if (!is_valid())
        return nullptr;

    c_entity* ent = c_entity::get_entity_from_index(get_entry_index());
    if (!ent || ent->get_ref_handle() != *this)
        return nullptr;

    return ent;
}