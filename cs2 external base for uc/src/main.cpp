#include <Windows.h>
#include <stdexcept>
#include <thread>

// entitylist
#include "features/entities.h"

// globals
#include "core/globals.h"

int main()
{
    // Initialize memory system and other systems
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
            const auto pawn = entity.entity->pawn().get();

            printf("entity->%s->%d\n", entity.entity->clan_tag().c_str(), pawn->health());
        }

        Sleep(100);
    }

    return 0;
}