#include "System.hpp"
#include <iostream>

REGISTER_ROUTINE(point_mass_model, 10)
{
    // read parameters by name
    auto pos = System::get_parameter<double>("position");
    auto vel = System::get_parameter<double>("velocity");
    auto acc = System::get_parameter<double>("acceleration");

    // perform a calculation with the parameters
    pos = pos + vel * (double)(1 / 10.0);
    vel = vel + acc * (double)(1 / 10.0);

    // update parameter 2 by name
    System::set_parameter("position", pos);
    System::set_parameter("velocity", vel);
}

REGISTER_ROUTINE(print_state, 1)
{
    auto pos = System::get_parameter<double>("position");
    auto vel = System::get_parameter<double>("velocity");
    auto acc = System::get_parameter<double>("acceleration");

    std::cout << "pos: " << pos << " vel: " << vel << " acc: " << acc << std::endl;
}
