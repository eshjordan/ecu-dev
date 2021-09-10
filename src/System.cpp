#include "System.hpp"
#include <cstddef>

void System::run_routines()
{
    System *sys = System::get_instance();
    for (int i = 0; i < sys->m_routines_count; ++i)
    {
        // sys->m_routines[i]->FunctionBody(nullptr);
    }
}
