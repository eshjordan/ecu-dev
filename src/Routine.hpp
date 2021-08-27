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

public:
    // void run(void *params);

    Routine(const std::string &name);

    ~Routine();

    virtual void FunctionBody(void *) = 0;
};

using RoutineInfo = struct RoutineInfo;

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

    Routine *create_routine() override { return new RoutineClass; }
};

// clang-format off
#define REGISTER_ROUTINE(name, param_name)                                                                          \
class name##_t : public Routine                                                                                     \
{                                                                                                                   \
public:                                                                                                             \
    name##_t() : Routine(#name) {}                                                                                  \
                                                                                                                    \
private:                                                                                                            \
    void FunctionBody(param_name) override;                                                                         \
    static RoutineInfo *const result_;                                                                              \
};                                                                                                                  \
RoutineInfo *const name##_t::result_ = System::register_routine(#name, new RoutineFactory<name##_t>);               \
void name##_t::FunctionBody(param_name)
// clang-format on
