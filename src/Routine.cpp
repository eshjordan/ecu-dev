#include "Routine.hpp"

Routine::Routine(const std::string &name, const uint32_t frequency) : m_name(name), m_frequency(frequency) {}

Routine::~Routine() {}

// void Routine::run(void *params) { this->FunctionBody(params); }

using RoutineInfo = struct RoutineInfo {
    int status = 0;
};

RoutineFactoryBase::RoutineFactoryBase() {}

RoutineFactoryBase::~RoutineFactoryBase() {}
