#pragma once

#include "Parameter.hpp"
#include <cstddef>
#include <stdexcept>
#include <vector>

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
    static ParameterBase *m_parameters[];

    static uint32_t m_parameter_count;

public:
    /**
     * @brief Add a new named Parameter to the ParameterList.
     *
     * @tparam T Type of the Parameter's value.
     * @param name Name of the Parameter.
     * @param value Value of the Parameter.
     */
    template <typename T> static void add_parameter(const std::string &name, const T &value);

    /**
     * @brief Get the value of a named parameter.
     *
     * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Parameter.
     * @return T Current value of the Parameter.
     */
    template <typename T> [[nodiscard]] static T get_parameter(const std::string &name);

    /**
     * @brief Set a new value for a named Parameter.
     *
     * @tparam T Type of the Parameter's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Parameter.
     * @param value New value of the Parameter.
     */
    template <typename T> static void set_parameter(const std::string &name, const T &value);

    /**
     * @brief Get the number of Parameters in the ParameterList.
     *
     * @return size_t Number of Parameters in the ParameterList.
     */
    [[nodiscard]] static inline size_t get_size(void) { return m_parameter_count; }
};

template <typename T> void ParameterList::add_parameter(const std::string &name, const T &value)
{
    for (int i = 0; i < m_parameter_count; i++)
    {
        auto *parameter = m_parameters[i];
        if (parameter->get_name() == name)
        {
            printf("Parameter '%s' already registered.", name.c_str());
            return;
        }
    }

    m_parameters[m_parameter_count++] = new Parameter<T>(name, value);
}

template <typename T> [[nodiscard]] T ParameterList::get_parameter(const std::string &name)
{
    for (int i = 0; i < m_parameter_count; i++)
    {
        auto *parameter = m_parameters[i];
        if (parameter->get_name() == name) { return ((Parameter<T> *)(parameter))->get_value(); }
    }

    printf("Parameter '%s' not found.", name.c_str());
    return {};
}

template <typename T> void ParameterList::set_parameter(const std::string &name, const T &value)
{
    for (int i = 0; i < m_parameter_count; i++)
    {
        auto *parameter = m_parameters[i];
        if (parameter->get_name() == name)
        {
            if (type_hash<T>() != parameter->get_type())
            {
                printf("Parameter '%s' set to wrong type, explicitly set the typename template argument.",
                       name.c_str());
            }
            ((Parameter<T> *)parameter)->set_value(value);
            return;
        }
    }

    printf("Parameter '%s' not found.", name.c_str());
}

} // namespace Impl
} // namespace System
