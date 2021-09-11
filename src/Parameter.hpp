#pragma once

#include <string>
#include <type_traits>
#include <utility>

// // All int types
// template class Parameter<short int>;
// template class Parameter<>;
// template class Parameter<>;
// template class Parameter<>;
// template class Parameter<>;
// template class Parameter<>;

// // Boolean type
// template class Parameter<bool>;

// // Char types
// template class Parameter<signed char>;
// template class Parameter<unsigned char>;
// template class Parameter<char>;
// template class Parameter<wchar_t>;
// template class Parameter<char16_t>;
// template class Parameter<char32_t>;

// // Floating point types
// template class Parameter<float>;
// template class Parameter<double>;
// template class Parameter<long double>;

// // Other useful types
// template class Parameter<std::string>;

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
    if (std::is_same<T, short int>::value) { return TYPE_ID::SHORT_INT; }
    if (std::is_same<T, int>::value) { return TYPE_ID::INT; }
    if (std::is_same<T, unsigned int>::value) { return TYPE_ID::UNSIGNED_INT; }
    if (std::is_same<T, unsigned long int>::value) { return TYPE_ID::UNSIGNED_LONG_INT; }
    if (std::is_same<T, long long int>::value) { return TYPE_ID::LONG_LONG_INT; }
    if (std::is_same<T, unsigned long long int>::value) { return TYPE_ID::UNSIGNED_LONG_LONG_INT; }
    if (std::is_same<T, bool>::value) { return TYPE_ID::BOOL; }
    if (std::is_same<T, signed char>::value) { return TYPE_ID::SIGNED_CHAR; }
    if (std::is_same<T, unsigned char>::value) { return TYPE_ID::UNSIGNED_CHAR; }
    if (std::is_same<T, char>::value) { return TYPE_ID::CHAR; }
    if (std::is_same<T, wchar_t>::value) { return TYPE_ID::WCHAR; }
    if (std::is_same<T, char16_t>::value) { return TYPE_ID::CHAR16; }
    if (std::is_same<T, char32_t>::value) { return TYPE_ID::CHAR32; }
    if (std::is_same<T, float>::value) { return TYPE_ID::FLOAT; }
    if (std::is_same<T, double>::value) { return TYPE_ID::DOUBLE; }
    if (std::is_same<T, long double>::value) { return TYPE_ID::LONG_DOUBLE; }
    if (std::is_same<T, std::string>::value) { return TYPE_ID::STRING; }
    return TYPE_ID::UNDEFINED;
}

class ParameterBase
{
public:
    [[nodiscard]] virtual std::string get_name() const = 0;
    [[nodiscard]] virtual TYPE_ID get_type() const     = 0;
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

    ~Parameter<T>() = default;

    Parameter<T>(const Parameter<T> &) = default;

    Parameter<T> &operator=(const Parameter<T> &) = default;

    Parameter<T>(Parameter<T> &&) noexcept = default;

    Parameter<T> &operator=(Parameter<T> &&) noexcept = default;

    [[nodiscard]] std::string get_name() const override { return m_name; }

    [[nodiscard]] TYPE_ID get_type() const override { return m_type; }

    [[nodiscard]] T get_value() const { return m_value; }

    void set_value(const T &value) { m_value = value; }
};
