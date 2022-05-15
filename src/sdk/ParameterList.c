#include "ParameterList.h"
#include <string.h>

/**
 * @brief Singleton record of all managed Parameters. Can contain Parameters with values of different types.
 *
 */

/** @brief List of ParameterBase pointers. */
static Parameter_st m_parameters[64];

static uint32_t m_parameter_count;


/**
 * @brief Get the number of Parameters in the ParameterList.
 *
 * @return size_t Number of Parameters in the ParameterList.
 */
size_t parameterListGetSize(void) { return m_parameter_count; }


/**
 * @brief Add a new named Parameter to the ParameterList.
 *
 * @tparam T Type of the Parameter's value.
 * @param name Name of the Parameter.
 * @param value Value of the Parameter.
 */
void parameterListAddParameter(const char *const name, const void *const value, const ParameterType_en type, const uint32_t size)
{
    for (int i = 0; i < m_parameter_count; i++)
    {
        Parameter_st *parameter = &m_parameters[i];
        if (strncmp(parameter->m_name, name, strlen(name)) == 0)
        {
            printf("Parameter '%s' already registered\n", name);
            return;
        }
    }

    Parameter_st *newParameter = &m_parameters[m_parameter_count++];

    strncpy(newParameter->m_name, name, strlen(name));
    memcpy(newParameter->m_value, value, size);
    newParameter->m_type = type;
    newParameter->m_size = size;
}

/**
 * @brief Get the value of a named parameter.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @return T Current value of the Parameter.
 */
uint32_t parameterListGetParameter(const char *const name, void *const value, ParameterType_en *const type)
{
    for (int i = 0; i < m_parameter_count; i++)
    {
        Parameter_st *parameter = &m_parameters[i];
        if (strncmp(parameter->m_name, name, strlen(name)) == 0) {
            memcpy(value, parameter->m_value, parameter->m_size);
            *type = parameter->m_type;
            return parameter->m_size;
        }
    }

    printf("Parameter '%s' not found\n", name);
    return 0;
}


/**
 * @brief Set a new value for a named Parameter.
 *
 * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
 * @param name Name of the Parameter.
 * @param value New value of the Parameter.
 */
void parameterListSetParameter(const char *const name, const void *const value, const ParameterType_en type, const uint32_t size)
{
    for (int i = 0; i < m_parameter_count; i++)
    {
        Parameter_st *parameter = &m_parameters[i];
        if (strncmp(parameter->m_name, name, strlen(name)) == 0)
        {
            if (type != parameter->m_type)
            {
                printf("Parameter '%s' set to wrong type, explicitly set the typename template argument\n", name);
            }
            memcpy(parameter->m_value, value, size);
            parameter->m_type = type;
            parameter->m_size = size;
            return;
        }
    }

    printf("Parameter '%s' not found\n", name);
}

