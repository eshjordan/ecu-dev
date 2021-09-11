#pragma once

#include "Parameter.hpp"
#include <iterator>
#include <stdexcept>
#include <vector>

/**
 * @brief Singleton.
 *
 */
class ParameterList
{
public:
    static ParameterList *get_instance(void)
    {
        static ParameterList instance; // Guaranteed to be destroyed.
                                       // Instantiated on first use.
        return &instance;
    }

    /**
     * @brief Copy constructor does nothing, because the class is a singleton.
     *
     */
    ParameterList(ParameterList const &other) {}

    /**
     * @brief Copy assignment operator does nothing, because the class is a singleton.
     *
     */
    void operator=(ParameterList const &other) {}

    template <typename T> static void add_parameter(const std::string &name, const T &value);

    template <typename T> [[nodiscard]] static T get_parameter(const std::string &name);

    template <typename T> static void set_parameter(const std::string &name, const T &value);

    static const std::vector<const ParameterBase *> &parameter_list()
    {
        return *reinterpret_cast<std::vector<const ParameterBase *> *>(&(get_instance()->m_parameters));
    }

    [[nodiscard]] static inline auto get_size() { return ParameterList::get_instance()->m_parameters.size(); }

private:
    /**
     * @brief Construct a System instance. Private to force singleton.
     *
     */
    ParameterList(void) {}

    static std::vector<ParameterBase *> &get_list(void) { return get_instance()->m_parameters; }

    std::vector<ParameterBase *> m_parameters{};
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
