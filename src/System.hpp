#pragma once

#include "RTOS.hpp"
#include "Routine.hpp"
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>

class System
{
public:
    static System &get_instance()
    {
        static System instance; // Guaranteed to be destroyed.
                                // Instantiated on first use.
        return instance;
    }

    using MEMBER_FN = void (System::*)(void *);
    using VOID_FN   = void (*)(void *);
    using CB_FN     = void(void *);
    using CB_LIST   = std::function<CB_FN>;

private:
    /**
     * @brief Construct a System instance. Private to force singleton.
     *
     */
    System() {}

    /**
     * @brief List of routines to be executed.
     *
     */
    Routine **m_tasks = static_cast<Routine **>(pvPortMalloc(1000 * sizeof(Routine *)));

    /**
     * @brief The number of tasks in the system.
     *
     */
    uint32_t m_tasks_count{};

public:
    /**
     * @brief Copy constructor does nothing, because the class is a singleton.
     *
     */
    System(System const &) {}

    /**
     * @brief Copy assignment operator does nothing, because the class is a singleton.
     *
     */
    void operator=(System const &) {}

    template <typename T> static RoutineInfo *const register_routine(const std::string &name, RoutineFactory<T> *factory)
    {
        System sys                       = System::get_instance();
        sys.m_tasks[sys.m_tasks_count++] = factory->create_routine();
        return nullptr;
    }

    void run_routines();
};
