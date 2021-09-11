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
    /**
     * @brief Get the single instance of the ParameterList object. Shouldn't be accessed outside the class.
     *
     * @return ParameterList* Pointer to the ParameterList object.
     */
    static ParameterList *get_instance(void)
    {
        static ParameterList instance; // Guaranteed to be destroyed.
                                       // Instantiated on first use.
        return &instance;
    }

    /** Rule of Six */

    /**
     * @brief Default constructor. Construct the ParameterList instance. Private because the class is a singleton.
     *
     */
    ParameterList(void) = default;

    /**
     * @brief Default destructor. Destroy the ParameterList instance. Private because the class is a singleton.
     *
     */
    ~ParameterList(void) = default;

public:
    /**
     * @brief Copy constructor. Deleted because the class is a singleton.
     *
     */
    ParameterList(ParameterList const &other) = delete;

    /**
     * @brief Copy assignment operator. Deleted because the class is a singleton.
     *
     */
    void operator=(ParameterList const &other) = delete;

    /**
     * @brief Move constructor. Deleted because the class is a singleton.
     *
     */
    ParameterList(ParameterList &&other) = delete;

    /**
     * @brief Move assignment operator. Deleted because the class is a singleton.
     *
     */
    ParameterList &operator=(ParameterList &&other) = delete;

private:
    /** Member variables */

    /**
     * @brief Utility function to get the internal list of Parameters (within class only).
     *
     * @return std::vector<ParameterBase *>& Mutable reference to the internal list of Parameters.
     */
    static std::vector<ParameterBase *> &get_list(void) { return ParameterList::get_instance()->m_parameters; }

    /** @brief List of ParameterBase pointers. */
    std::vector<ParameterBase *> m_parameters{};

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
     * @brief Return a constant reference to the internal list of Parameters.
     *
     * @return const std::vector<const ParameterBase *>& Constant reference to the internal list of Parameters.
     */
    static const std::vector<const ParameterBase *> &parameter_list(void)
    {
        return *reinterpret_cast<std::vector<const ParameterBase *> *>(&(get_instance()->m_parameters));
    }

    /**
     * @brief Get the number of Parameters in the ParameterList.
     *
     * @return size_t Number of Parameters in the ParameterList.
     */
    [[nodiscard]] static inline size_t get_size(void) { return ParameterList::get_instance()->m_parameters.size(); }
};

template <typename T> void ParameterList::add_parameter(const std::string &name, const T &value)
{
    for (auto *parameter : get_list())
    {
        if (parameter->get_name() == name) { throw std::runtime_error("Parameter '" + name + "' already registered."); }
    }
    auto *parameter = new Parameter<T>(name, value);
    get_list().push_back(parameter);
}

template <typename T> [[nodiscard]] T ParameterList::get_parameter(const std::string &name)
{
    for (auto *parameter : get_list())
    {
        if (parameter->get_name() == name) { return dynamic_cast<Parameter<T> *>(parameter)->get_value(); }
    }

    throw std::range_error("Parameter '" + name + "' not found.");
}

template <typename T> void ParameterList::set_parameter(const std::string &name, const T &value)
{
    for (auto *parameter : get_list())
    {
        if (parameter->get_name() == name)
        {
            if (type_hash<T>() != parameter->get_type())
            {
                throw std::runtime_error("Parameter '" + name
                                         + "' set to wrong type, explicitly set the typename template argument.");
            }
            dynamic_cast<Parameter<T> *>(parameter)->set_value(value);
            return;
        }
    }

    throw std::range_error("Parameter '" + name + "' not found.");
}

} // namespace Impl
} // namespace System
