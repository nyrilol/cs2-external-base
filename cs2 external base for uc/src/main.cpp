#include <Windows.h>
#include <stdexcept>

#include "features/entities.h"

int main()
{
	while (!g_p_memory_system->initialize("cs2.exe"))
		std::this_thread::sleep_for(std::chrono::seconds(1));

	if (!g_netvars->initialize())
		throw std::runtime_error("failed to initialize netvar system");

	if (!g_offsets->initialize())
		throw std::runtime_error("failed to initialize offset system");

	while (true)
	{
		g_entity_list->update();

		for (const auto entity : g_entity_list->entity_vec)
		{
			const auto entity_pawn = entity.entity->pawn().get();

			printf("entity->%s->health->%d\n", entity.entity->name().c_str(), entity_pawn->health());

		}
		Sleep(1000);
	}
}