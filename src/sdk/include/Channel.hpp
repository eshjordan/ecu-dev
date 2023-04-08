#pragma once

#include "Parameter.hpp"
#include <stdint.h>

enum ChannelLogRate : uint16_t {
	CHANNEL_LOG_OFF = 0,
	CHANNEL_LOG_1HZ = 1,
	CHANNEL_LOG_10HZ = 10,
	CHANNEL_LOG_20HZ = 20,
	CHANNEL_LOG_50HZ = 50,
	CHANNEL_LOG_100HZ = 100,
	CHANNEL_LOG_200HZ = 200,
	CHANNEL_LOG_500HZ = 500,
	CHANNEL_LOG_1000HZ = 1000,
	CHANNEL_LOG_COUNT = 9
};

namespace System
{
namespace Impl
{

/**
 * @brief Base class for all templated Channels to inherit from, used for polymorphism.
 *
 */
class ChannelBase {
    public:
	/**
     * @brief Get the name of the channel.
     *
     * @return const char * Channel name.
     */
	[[nodiscard]] virtual const char *get_name(void) const = 0;

	/**
     * @brief Get the type enum of the channel.
     *
     * @return TYPE_ID Type ID value.
     */
	[[nodiscard]] virtual ParameterType get_type(void) const = 0;

	/**
     * @brief Get the log rate of the channel.
     *
     * @return ChannelLogRate Logging rate.
     */
	[[nodiscard]] virtual ChannelLogRate get_log_rate(void) const = 0;
};

/**
 * @brief Class to store named values, to be logged at a set frequency.
 *
 */
class Channel  {

private:
	/** @brief Value stored by the Channel. */
	Parameter m_parameter{};

	/** @brief Logging rate of the Channel. */
	const ChannelLogRate m_log_rate = ChannelLogRate::CHANNEL_LOG_OFF;

public:
	Channel();

	explicit Channel(const char *name) : m_parameter(name) {}

	/**
     * @brief Construct a new named Channel object
     *
     * @param name Channel name.
     * @param value Initial value of the Channel.
     * @param log_rate Logging rate of the Channel.
     */
	Channel(const char *name, const ParameterValue &value,
		   const ChannelLogRate &log_rate)
		: m_parameter(name, value), m_log_rate(log_rate)
	{}

	/// Construct with given name and given parameter value.
	template<typename ValueTypeT>
	Channel(const char *name, ValueTypeT value, const ChannelLogRate &log_rate)
	: Channel(name, ParameterValue(value), log_rate)
	{}

	/** Getters and Setters */

	/**
     * @brief Get the Channel's name.
     *
     * @return const char * Channel name.
     */
	void get_name(char *out) const
	{
		m_parameter.get_name(out);
	}

	/**
     * @brief Get the Channel's type.
     *
     * @return TYPE_ID Channel type.
     */
	[[nodiscard]] ParameterType get_type(void) const
	{
		return m_parameter.get_type();
	}

	/// Get value of parameter using rclcpp::ParameterType as template argument.
	/**
	* \throws rclcpp::exceptions::InvalidParameterTypeException if the type doesn't match
	*/
	template<ParameterType ParamT>
	decltype(auto)
	get_value() const
	{
		return m_parameter.get_value<ParamT>();
	}

	/// Get value of parameter using c++ types as template argument.
	template<typename T>
	decltype(auto)
	get_value() const
	{
		return m_parameter.get_value<T>();
	}

	/**
	 * @brief Get the Channel's log rate.
	 *
	 * @return ChannelLogRate Logging rate.
	 */
	[[nodiscard]] ChannelLogRate get_log_rate(void) const
	{
		return m_log_rate;
	}

	/**
     * @brief Set the Channel's value.
     *
     * @param value New value of the Channel.
     */
	template<typename T>
	void set_value(const T &value) noexcept
	{
		ParameterValue new_value(value);
		if (new_value.get_type() != m_parameter.get_type())
		{
			ecu_fatal_error("Cannot set channel to a different type");
		}

		m_parameter.value_ = new_value;
	}
};

} // namespace Impl
} // namespace System
