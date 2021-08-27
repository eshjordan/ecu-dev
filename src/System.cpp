#include "System.hpp"
#include <cstddef>

void System::run_routines()
{
    for (int i = 0; i < m_tasks_count; ++i)
    {
        m_tasks[i]->FunctionBody(nullptr);
    }
}
