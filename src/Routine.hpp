#pragma once

#include "RTOS.hpp"
#include <string>

class Routine
{
private:
    const std::string m_name{};
    const uint32_t m_frequency{};
    friend class System;

public:
    Routine(const std::string &name, uint32_t frequency);

    ~Routine(void);
};

class RoutineFactoryBase
{
public:
    RoutineFactoryBase(void);

    ~RoutineFactoryBase(void);

    /**
     * @brief Creates a test instance to run. The instance is both created and destroyed within TestInfoImpl::Run()
     *
     * @return Routine*
     */
    virtual Routine *create_routine(void) = 0;
};

template <class RoutineClass> class RoutineFactory : RoutineFactoryBase
{
public:
    RoutineFactory(void) : RoutineFactoryBase() {}

    RoutineClass *create_routine(void) override { return new RoutineClass; }
};

// clang-format off
#define REGISTER_ROUTINE(name, frequency)                                                               \
class name##_t : public Routine                                                                         \
{                                                                                                       \
public:                                                                                                 \
    name##_t(void) : Routine((#name), (frequency)) {}                                                       \
                                                                                                        \
private:                                                                                                \
    static void FunctionBody(TimerHandle_t xTimer);                                                     \
    const static Routine *const result_;                                                                \
    friend class System;                                                                                \
};                                                                                                      \
const Routine *const name##_t::result_ = System::register_routine((#name), new RoutineFactory<name##_t>); \
void name##_t::FunctionBody(TimerHandle_t xTimer)
// clang-format on
