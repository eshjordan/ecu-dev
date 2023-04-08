#include "System.hpp"
#include "ParameterList.hpp"
#include <iostream>

INIT_MODULE(test)
{
	System::add_parameter<double>("acceleration", 1.0f);
	System::add_channel<double>("velocity", 0.0f, CHANNEL_LOG_100HZ);
	System::add_channel<double>("position", 0.0f, CHANNEL_LOG_100HZ);
	// System::Impl::ChannelList::start_logging();
}

REGISTER_ROUTINE(point_mass_model, 10, 256)
{
	// read parameters/channels by name
	auto acc = System::get_parameter_value<double>("acceleration");
	auto vel = System::get_channel_value<double>("velocity");
	auto pos = System::get_channel_value<double>("position");

	// perform a calculation with the parameters/channels
	pos = pos + vel * (1 / 10.0);
	vel = vel + acc * (1 / 10.0);

	// update channels by name
	System::set_channel_value("velocity", vel);
	System::set_channel_value("position", pos);
}

REGISTER_ROUTINE(print_state, 1, 256)
{
	auto acc = System::get_parameter_value<double>("acceleration");
	auto vel = System::get_channel_value<double>("velocity");
	auto pos = System::get_channel_value<double>("position");

	printf("pos: %lf, vel: %lf, acc: %lf\n", pos, vel, acc);
	printf("Analogue Input 4: %umV\n",
	       System::IO::read_analogue_input((IOADCChannel_en)4));
}
