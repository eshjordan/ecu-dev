#pragma once

#include "RTOS.hpp"
#include "types.hpp"
#include <cstdio>
#include <string>
#include <utility>

class Routine
{
private:
    const std::string &m_name;
    const uint32_t m_frequency{};
    friend class System;

public:
    // void run(void *params);

    Routine(const std::string &name, uint32_t frequency);

    ~Routine();

    static void FunctionBody(void *) {}
};

class RoutineFactoryBase
{
public:
    RoutineFactoryBase();

    ~RoutineFactoryBase();

    // Creates a test instance to run. The instance is both created and destroyed
    // within TestInfoImpl::Run()
    virtual Routine *create_routine() = 0;
};

template <class RoutineClass> class RoutineFactory : RoutineFactoryBase
{
public:
    RoutineFactory() : RoutineFactoryBase() {}

    RoutineClass *create_routine() override { return new RoutineClass; }
};

// clang-format off
#define REGISTER_ROUTINE(name, frequency)                                                               \
class name##_t : public Routine                                                                         \
{                                                                                                       \
public:                                                                                                 \
    name##_t() : Routine(#name, frequency) {}                                                           \
                                                                                                        \
private:                                                                                                \
    static void FunctionBody(TimerHandle_t);                                                            \
    const static Routine *const result_;                                                                \
    friend class System;                                                                                \
};                                                                                                      \
const Routine *const name##_t::result_ = System::register_routine(#name, new RoutineFactory<name##_t>); \
void name##_t::FunctionBody(TimerHandle_t)
// clang-format on
