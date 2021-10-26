#include "RoutineManager.hpp"

System::Impl::Routine *System::Impl::RoutineManager::m_routines[1000] = {};
TaskHandle_t *System::Impl::RoutineManager::m_tasks[1000]             = {};
TimerHandle_t *System::Impl::RoutineManager::m_timers[1000]           = {};
uint32_t System::Impl::RoutineManager::m_routines_count               = 0;
uint32_t System::Impl::RoutineManager::m_tasks_count                  = 0;
uint32_t System::Impl::RoutineManager::m_timers_count                 = 0;
