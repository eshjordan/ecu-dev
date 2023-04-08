#pragma once

#include "Parameter.hpp"
#include <stddef.h>
#include <string.h>

namespace System {
namespace Impl {

/**
 * @brief Singleton record of all managed Parameters. Can contain Parameters with values of different types.
 *
 */
class ParameterList
{
private:
    /** Member variables */

    /** @brief List of ParameterBase pointers. */
    static Parameter *m_parameters[];

    static uint32_t m_parameter_count;

public:
    /**
     * @brief Add a new named Parameter to the ParameterList.
     *
     * @tparam T Type of the Parameter's value.
     * @param name Name of the Parameter.
     * @param value Value of the Parameter.
     */
    static void add_parameter(const char *name, const ParameterValue &value)
    {
        char tmp_name[64];
        for (int i = 0; i < m_parameter_count; i++)
        {
            auto *parameter = m_parameters[i];
            parameter->get_name(tmp_name);
            if (strcmp(tmp_name, name) == 0)
            {
                ecu_fatal_error("Parameter '%s' already registered\n", name);
                return;
            }
        }

        m_parameters[m_parameter_count++] = new Parameter(name, value);
    }


	/**
     * @brief Add a new named Channel to the ChannelList.
     *
     * @tparam T Type of the Channel's value.
     * @param name Name of the Channel.
     * @param value Value of the Channel.
     */
	template <typename T>
	static void add_parameter(const char *name, T value) {
		add_parameter(name, ParameterValue(value));
	}

    /**
     * @brief Get the value of a named parameter.
     *
     * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Parameter.
     * @return T Current value of the Parameter.
     */
    template<typename T>
	[[nodiscard]]
	static
	decltype(auto)
    get_parameter_value(const char *name)
    {
        char tmp_name[64];
        for (int i = 0; i < m_parameter_count; i++)
        {
            auto *parameter = m_parameters[i];
            parameter->get_name(tmp_name);
            if (strcmp(tmp_name, name) == 0) {
                return parameter->get_value<T>();
            }
        }

        ecu_fatal_error("Parameter '%s' not found\n", name);

        // Should never actually return, but needed to prevent type determination compilation errors
        return m_parameters[0]->get_value<T>();
    }

    /**
     * @brief Set a new value for a named Parameter.
     *
     * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Parameter.
     * @param value New value of the Parameter.
     */
    template<typename T>
    static
    void
    set_parameter_value_(const char *name, const ParameterValue &value)
    {
        char tmp_name[64];
        for (int i = 0; i < m_parameter_count; i++)
        {
            auto *parameter = m_parameters[i];
            parameter->get_name(tmp_name);
            if (strcmp(tmp_name, name) == 0)
            {
                if (value.get_type() != parameter->get_type())
                {
                    ecu_fatal_error("Parameter '%s' set to wrong type, explicitly set the typename template argument\n", name);
                }
                parameter->value_ = value;
                return;
            }
        }

        ecu_fatal_error("Parameter '%s' not found\n", name);
    }

    /**
     * @brief Set a new value for a named Parameter.
     *
     * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Parameter.
     * @param value New value of the Parameter.
     */
    template<typename T>
    static
    void
    set_parameter_value(const char *name, const ParameterValue &value)
    {
        set_parameter_value_<T>(name, value);
    }

    /**
     * @brief Set a new value for a named Parameter.
     *
     * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Parameter.
     * @param value New value of the Parameter.
     */
    template<typename T>
    static
    void
    set_parameter_value(const char *name, T value)
    {
        set_parameter_value_<T>(name, ParameterValue(value));
    }

    /**
     * @brief Get the number of Parameters in the ParameterList.
     *
     * @return size_t Number of Parameters in the ParameterList.
     */
    [[nodiscard]] static inline size_t get_size(void) { return m_parameter_count; }
};


} // namespace Impl
} // namespace System
