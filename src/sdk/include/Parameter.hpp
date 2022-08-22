#pragma once

#include <string>
#include <type_traits>
#include <utility>

namespace System
{
namespace Impl
{

/**
 * @brief Enum class to represent each of the generic C types, so we can record types in member
 * variables.
 *
 */
enum class TYPE_ID {
	UNDEFINED,
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
	STRING
};

inline const char *type_id_to_str(const TYPE_ID &type)
{
	switch (type) {
	case TYPE_ID::UNDEFINED:
		return "UNDEFINED";
	case TYPE_ID::SHORT_INT:
		return "SHORT_INT";
	case TYPE_ID::INT:
		return "INT";
	case TYPE_ID::LONG_INT:
		return "LONG_INT";
	case TYPE_ID::LONG_LONG_INT:
		return "LONG_LONG_INT";
	case TYPE_ID::UNSIGNED_SHORT_INT:
		return "UNSIGNED_SHORT_INT";
	case TYPE_ID::UNSIGNED_INT:
		return "UNSIGNED_INT";
	case TYPE_ID::UNSIGNED_LONG_INT:
		return "UNSIGNED_LONG_INT";
	case TYPE_ID::UNSIGNED_LONG_LONG_INT:
		return "UNSIGNED_LONG_LONG_INT";
	case TYPE_ID::BOOL:
		return "BOOL";
	case TYPE_ID::SIGNED_CHAR:
		return "SIGNED_CHAR";
	case TYPE_ID::UNSIGNED_CHAR:
		return "UNSIGNED_CHAR";
	case TYPE_ID::CHAR:
		return "CHAR";
	case TYPE_ID::WCHAR:
		return "WCHAR";
	case TYPE_ID::CHAR16:
		return "CHAR16";
	case TYPE_ID::CHAR32:
		return "CHAR32";
	case TYPE_ID::FLOAT:
		return "FLOAT";
	case TYPE_ID::DOUBLE:
		return "DOUBLE";
	case TYPE_ID::LONG_DOUBLE:
		return "LONG_DOUBLE";
	case TYPE_ID::STRING:
		return "STRING";
	default:
		return "UNDEFINED";
	}
}

// clang-format off

/**
 * @brief Given a type, return the corresponding TYPE_ID.
 *
 * @tparam T
 * @return constexpr TYPE_ID
 */
template <typename T> constexpr TYPE_ID type_hash(void)
{
    return ((bool)std::is_same<T, short int>::value) ? TYPE_ID::SHORT_INT : (
        ((bool)std::is_same<T, int>::value) ? TYPE_ID::INT : (
            ((bool)std::is_same<T, unsigned int>::value) ? TYPE_ID::UNSIGNED_INT : (
                ((bool)std::is_same<T, unsigned long int>::value) ? TYPE_ID::UNSIGNED_LONG_INT : (
                    ((bool)std::is_same<T, long long int>::value) ? TYPE_ID::LONG_LONG_INT : (
                        ((bool)std::is_same<T, unsigned long long int>::value) ? TYPE_ID::UNSIGNED_LONG_LONG_INT : (
                            ((bool)std::is_same<T, bool>::value) ? TYPE_ID::BOOL : (
                                ((bool)std::is_same<T, signed char>::value) ? TYPE_ID::SIGNED_CHAR : (
                                    ((bool)std::is_same<T, unsigned char>::value) ? TYPE_ID::UNSIGNED_CHAR : (
                                        ((bool)std::is_same<T, char>::value) ? TYPE_ID::CHAR : (
                                            ((bool)std::is_same<T, wchar_t>::value) ? TYPE_ID::WCHAR : (
                                                ((bool)std::is_same<T, char16_t>::value) ? TYPE_ID::CHAR16 : (
                                                    ((bool)std::is_same<T, char32_t>::value) ? TYPE_ID::CHAR32 : (
                                                        ((bool)std::is_same<T, float>::value) ? TYPE_ID::FLOAT : (
                                                            ((bool)std::is_same<T, double>::value) ? TYPE_ID::DOUBLE : (
                                                                ((bool)std::is_same<T, long double>::value) ? TYPE_ID::LONG_DOUBLE : (
                                                                    ((bool)std::is_same<T, const char *>::value) ? TYPE_ID::STRING : (
                                                                        TYPE_ID::UNDEFINED
                                                                    )
                                                                )
                                                            )
                                                        )
                                                    )
                                                )
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        )
    );
}

/**
 * @brief Given a type, return the corresponding format specifier.
 *
 * @tparam T
 * @return constexpr TYPE_ID
 */
constexpr const char * type_fmt_spc(const TYPE_ID &type)
{
    return (TYPE_ID::SHORT_INT == type) ? "%hd" : (
        (TYPE_ID::INT == type) ? "%d" : (
            (TYPE_ID::UNSIGNED_INT == type) ? "%u" : (
                (TYPE_ID::UNSIGNED_LONG_INT == type) ? "%lu" : (
                    (TYPE_ID::LONG_LONG_INT == type) ? "%lld" : (
                        (TYPE_ID::UNSIGNED_LONG_LONG_INT == type) ? "%llu" : (
                            (TYPE_ID::BOOL == type) ? "%hu" : (
                                (TYPE_ID::SIGNED_CHAR == type) ? "%c" : (
                                    (TYPE_ID::UNSIGNED_CHAR == type) ? "%hu" : (
                                        (TYPE_ID::CHAR == type) ? "%c" : (
                                            (TYPE_ID::WCHAR == type) ? "%c" : (
                                                (TYPE_ID::CHAR16 == type) ? "%c" : (
                                                    (TYPE_ID::CHAR32 == type) ? "%c" : (
                                                        (TYPE_ID::FLOAT == type) ? "%f" : (
                                                            (TYPE_ID::DOUBLE == type) ? "%lf" : (
                                                                (TYPE_ID::LONG_DOUBLE == type) ? "%llf" : (
                                                                    (TYPE_ID::STRING == type) ? "%s" : (
                                                                        ""
                                                                    )
                                                                )
                                                            )
                                                        )
                                                    )
                                                )
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        )
    );
}

// clang-format on

/**
 * @brief Base class for all templated Parameters to inherit from, used for polymorphism.
 *
 */
class ParameterBase {
    public:
	/**
     * @brief Get the name of the parameter.
     *
     * @return const char * Parameter name.
     */
	[[nodiscard]] virtual const char *get_name(void) const = 0;

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
template <typename T> class Parameter : public ParameterBase {
    private:
	/** @brief Value stored by the Parameter. */
	T m_value{};

	/** @brief Name of the Parameter. */
	const char *m_name{};

	/** @brief Type of the Parameter. */
	const TYPE_ID m_type = TYPE_ID::UNDEFINED;

    public:
	/**
     * @brief Construct a new named Parameter object
     *
     * @param name Parameter name.
     * @param value Initial value of the Parameter.
     */
	Parameter<T>(const char *name, const T &value)
		: m_type(type_hash<T>()), m_name(name)
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
     * @return const char * Parameter name.
     */
	[[nodiscard]] const char *get_name(void) const override
	{
		return m_name;
	}

	/**
     * @brief Get the Parameter's type.
     *
     * @return TYPE_ID Parameter type.
     */
	[[nodiscard]] TYPE_ID get_type(void) const override
	{
		return m_type;
	}

	/**
     * @brief Get the Parameter's value.
     *
     * @return T Parameter value.
     */
	[[nodiscard]] T get_value(void) const
	{
		return m_value;
	}

	/**
     * @brief Set the Parameter's value.
     *
     * @param value New value of the Parameter.
     */
	void set_value(const T &value) noexcept
	{
		m_value = value;
	}
};

} // namespace Impl
} // namespace System
