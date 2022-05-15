#include "System.h"
#include <stdio.h>

REGISTER_ROUTINE(point_mass_model, 10)
{
    // read parameters by name
    double pos = *((double*)systemGetParameter("position"));
    double vel = *((double*)systemGetParameter("velocity"));
    double acc = *((double*)systemGetParameter("acceleration"));

    // perform a calculation with the parameters
    pos = pos + vel * (1 / 10.0);
    vel = vel + acc * (1 / 10.0);

    // update parameter 2 by name
    systemSetParameter("position", &pos, PARAM_DOUBLE);
    systemSetParameter("velocity", &vel, PARAM_DOUBLE);
}

REGISTER_ROUTINE(print_state, 1)
{
    double pos = *((double*)systemGetParameter("position"));
    double vel = *((double*)systemGetParameter("velocity"));
    double acc = *((double*)systemGetParameter("acceleration"));

    printf("pos: %lf, vel: %lf, acc: %lf\n", pos, vel, acc);
    printf("Analogue Input 4: %umV\n", read_analogue_input(4));
}
