#include "System.hpp"
#include "ParameterList.hpp"
#include <iostream>

INIT_MODULE(test)
{
	System::add_parameter<double>("position", 0.0f);
	System::add_parameter<double>("velocity", 0.0f);
	System::add_parameter<double>("acceleration", 1.0f);
}

REGISTER_ROUTINE(point_mass_model, 10, 256)
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

REGISTER_ROUTINE(print_state, 1, 256)
{
	auto pos = System::get_parameter<double>("position");
	auto vel = System::get_parameter<double>("velocity");
	auto acc = System::get_parameter<double>("acceleration");

	printf("pos: %lf, vel: %lf, acc: %lf\n", pos, vel, acc);
	printf("Analogue Input 4: %umV\n",
	       System::IO::read_analogue_input((IOADCChannel_en)4));
}
