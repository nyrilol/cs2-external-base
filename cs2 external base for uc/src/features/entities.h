#pragma once

#include "../valve/classes/c_entity.h"

class entity_list
{
private:
	struct entity_t
	{
		c_entity* entity = nullptr;
	    int		  index = 0;
	};
public:
	std::vector<entity_t> entity_vec;

	void update()
	{
		entity_vec.clear();

		for (int idx = 1; idx < 64; ++idx)
		{
			c_entity* ent = c_entity::get_entity_from_index(idx);
			if (!ent) continue;

			if (ent->get_schema_name() == "CCSPlayerController")
				entity_vec.emplace_back(entity_t(ent, idx));
			else
				continue;
		}
	}
};	inline std::unique_ptr<entity_list> g_entity_list = std::make_unique<entity_list>();