#pragma once

#include <string>
#include <type_traits>
#include <utility>

namespace System {
namespace Impl {

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
    }
    return type_id;
}

class ParameterBase
{
public:
    [[nodiscard]] virtual std::string get_name(void) const = 0;
    [[nodiscard]] virtual TYPE_ID get_type(void) const     = 0;
};

template <typename T> class Parameter : public ParameterBase
{
private:
    T m_value{};

    const std::string m_name{};

    const TYPE_ID m_type = TYPE_ID::UNDEFINED;

public:
    Parameter<T>(std::string name, const T &value) : m_type(type_hash<T>()), m_name(std::move(name))
    {
        set_value(value);
    }

    ~Parameter<T>(void) = default;

    Parameter<T>(const Parameter<T> &other) = default;

    Parameter<T> &operator=(const Parameter<T> &other) = default;

    Parameter<T>(Parameter<T> &&other) noexcept = default;

    Parameter<T> &operator=(Parameter<T> &&other) noexcept = default;

    [[nodiscard]] std::string get_name(void) const override { return m_name; }

    [[nodiscard]] TYPE_ID get_type(void) const override { return m_type; }

    [[nodiscard]] T get_value(void) const { return m_value; }

    void set_value(const T &value) { m_value = value; }
};

} // namespace Impl
} // namespace System
