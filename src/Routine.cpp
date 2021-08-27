#include "Routine.hpp"

Routine::Routine(const std::string &name) : m_name(name) {}

Routine::~Routine() {}

// void Routine::run(void *params) { this->FunctionBody(params); }

using RoutineInfo = struct RoutineInfo {
    int status = 0;
};

RoutineFactoryBase::RoutineFactoryBase() {}

RoutineFactoryBase::~RoutineFactoryBase() {}
