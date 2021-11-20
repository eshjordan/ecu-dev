#include "System.hpp"
#include <iostream>

REGISTER_ROUTINE(point_mass_model, 10)
{
    // read parameters by name
    auto pos = System::get_parameter<double>("position");
    auto vel = System::get_parameter<double>("velocity");
    auto acc = System::get_parameter<double>("acceleration");

    // perform a calculation with the parameters
    pos = pos + vel * (1 / 10.0);
    vel = vel + acc * (1 / 10.0);

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

REGISTER_ROUTINE(print_esp, 1)
{
    std::cout << "din[0]: " << System::IO::read_digital_input(0) << " din[1]: " << System::IO::read_digital_input(1)
              << " din[2]: " << System::IO::read_digital_input(2) << std::endl;
}
