#include "RoutineManager.hpp"

TaskHandle_t System::Impl::RoutineManager::m_tasks[128] = {};
TimerHandle_t System::Impl::RoutineManager::m_timers[128] = {};
uint32_t System::Impl::RoutineManager::m_routines_count = 0;
uint32_t System::Impl::RoutineManager::m_tasks_count = 0;
uint32_t System::Impl::RoutineManager::m_timers_count = 0;
