#pragma once

#include <string>
#include <type_traits>
#include <utility>

namespace System {
namespace Impl {

/**
 * @brief Enum class to represent each of the generic C types (and std::string), so we can record types in member
 * variables.
 *
 */
enum class TYPE_ID {
    SHORT_INT,
    INT,
    LONG_INT,
    LONG_LONG_INT,
    UNSIGNED_SHORT_INT,
    UNSIGNED_INT,
    UNSIGNED_LONG_INT,
    UNSIGNED_LONG_LONG_INT,
    BOOL,
    SIGNED_CHAR,
    UNSIGNED_CHAR,
    CHAR,
    WCHAR,
    CHAR16,
    CHAR32,
    FLOAT,
    DOUBLE,
    LONG_DOUBLE,
    STRING,
    UNDEFINED
};

/**
 * @brief Given a type, return the corresponding TYPE_ID.
 *
 * @tparam T
 * @return constexpr TYPE_ID
 */
template <typename T> constexpr TYPE_ID type_hash(void)
{
    TYPE_ID type_id = TYPE_ID::UNDEFINED;
    if ((bool)std::is_same<T, short int>::value)
    {
        type_id = TYPE_ID::SHORT_INT;
    } else if ((bool)std::is_same<T, int>::value)
    {
        type_id = TYPE_ID::INT;
    } else if ((bool)std::is_same<T, unsigned int>::value)
    {
        type_id = TYPE_ID::UNSIGNED_INT;
    } else if ((bool)std::is_same<T, unsigned long int>::value)
    {
        type_id = TYPE_ID::UNSIGNED_LONG_INT;
    } else if ((bool)std::is_same<T, long long int>::value)
    {
        type_id = TYPE_ID::LONG_LONG_INT;
    } else if ((bool)std::is_same<T, unsigned long long int>::value)
    {
        type_id = TYPE_ID::UNSIGNED_LONG_LONG_INT;
    } else if ((bool)std::is_same<T, bool>::value)
    {
        type_id = TYPE_ID::BOOL;
    } else if ((bool)std::is_same<T, signed char>::value)
    {
        type_id = TYPE_ID::SIGNED_CHAR;
    } else if ((bool)std::is_same<T, unsigned char>::value)
    {
        type_id = TYPE_ID::UNSIGNED_CHAR;
    } else if ((bool)std::is_same<T, char>::value)
    {
        type_id = TYPE_ID::CHAR;
    } else if ((bool)std::is_same<T, wchar_t>::value)
    {
        type_id = TYPE_ID::WCHAR;
    } else if ((bool)std::is_same<T, char16_t>::value)
    {
        type_id = TYPE_ID::CHAR16;
    } else if ((bool)std::is_same<T, char32_t>::value)
    {
        type_id = TYPE_ID::CHAR32;
    } else if ((bool)std::is_same<T, float>::value)
    {
        type_id = TYPE_ID::FLOAT;
    } else if ((bool)std::is_same<T, double>::value)
    {
        type_id = TYPE_ID::DOUBLE;
    } else if ((bool)std::is_same<T, long double>::value)
    {
        type_id = TYPE_ID::LONG_DOUBLE;
    } else if ((bool)std::is_same<T, std::string>::value)
    {
        type_id = TYPE_ID::STRING;
    } else
    {
        type_id = TYPE_ID::UNDEFINED;
    }
    return type_id;
}

/**
 * @brief Base class for all templated Parameters to inherit from, used for polymorphism.
 *
 */
class ParameterBase
{
public:
    /**
     * @brief Get the name of the parameter.
     *
     * @return std::string Parameter name.
     */
    [[nodiscard]] virtual std::string get_name(void) const = 0;

    /**
     * @brief Get the type enum of the parameter.
     *
     * @return TYPE_ID Type ID value.
     */
    [[nodiscard]] virtual TYPE_ID get_type(void) const = 0;
};

/**
 * @brief Template class to store named values.
 *
 * @tparam T Type of the parameter's value. Set explicitly if possible.
 */
template <typename T> class Parameter : public ParameterBase
{
private:
    /** @brief Value stored by the Parameter. */
    T m_value{};

    /** @brief Name of the Parameter. */
    const std::string m_name{};

    /** @brief Type of the Parameter. */
    const TYPE_ID m_type = TYPE_ID::UNDEFINED;

public:
    /**
     * @brief Construct a new named Parameter object
     *
     * @param name Parameter name.
     * @param value Initial value of the Parameter.
     */
    Parameter<T>(std::string name, const T &value) : m_type(type_hash<T>()), m_name(std::move(name))
    {
        (void)name;
        set_value(value);
    }

    /** Rule of Six */

    /**
     * @brief Default constructor. Deleted as all Parameters must be named at creation.
     *
     */
    Parameter(void) = delete;

    /**
     * @brief Destroy the Parameter object.
     *
     */
    ~Parameter<T>(void) = default;

    /**
     * @brief Copy constructor.
     *
     * @param other Parameter to copy.
     */
    Parameter<T>(const Parameter<T> &other) = default;

    /**
     * @brief Copy assignment operator.
     *
     * @param other Parameter to copy.
     * @return Parameter<T>& Reference to this Parameter.
     */
    Parameter<T> &operator=(const Parameter<T> &other) = default;

    /**
     * @brief Move constructor.
     *
     * @param other Parameter to move.
     */
    Parameter<T>(Parameter<T> &&other) noexcept = default;

    /**
     * @brief Move assignment operator.
     *
     * @param other Parameter to move.
     * @return Parameter<T>& Reference to this Parameter.
     */
    Parameter<T> &operator=(Parameter<T> &&other) noexcept = default;

    /** Getters and Setters */

    /**
     * @brief Get the Parameter's name.
     *
     * @return std::string Parameter name.
     */
    [[nodiscard]] std::string get_name(void) const override { return m_name; }

    /**
     * @brief Get the Parameter's type.
     *
     * @return TYPE_ID Parameter type.
     */
    [[nodiscard]] TYPE_ID get_type(void) const override { return m_type; }

    /**
     * @brief Get the Parameter's value.
     *
     * @return T Parameter value.
     */
    [[nodiscard]] T get_value(void) const { return m_value; }

    /**
     * @brief Set the Parameter's value.
     *
     * @param value New value of the Parameter.
     */
    void set_value(const T &value) noexcept { m_value = value; }
};

} // namespace Impl
} // namespace System
