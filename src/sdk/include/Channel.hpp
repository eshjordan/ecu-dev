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
	[[nodiscard]] virtual TYPE_ID get_type(void) const = 0;

	/**
     * @brief Get the log rate of the channel.
     *
     * @return ChannelLogRate Logging rate.
     */
	[[nodiscard]] virtual ChannelLogRate get_log_rate(void) const = 0;
};

/**
 * @brief Template class to store named values.
 *
 * @tparam T Type of the channel's value. Set explicitly if possible.
 */
template <typename T> class Channel : public ChannelBase {
    private:
	/** @brief Value stored by the Channel. */
	T m_value{};

	/** @brief Name of the Channel. */
	const char *m_name{};

	/** @brief Type of the Channel. */
	const TYPE_ID m_type = TYPE_ID::UNDEFINED;

	/** @brief Logging rate of the Channel. */
	const ChannelLogRate m_log_rate = ChannelLogRate::CHANNEL_LOG_OFF;

    public:
	/**
     * @brief Construct a new named Channel object
     *
     * @param name Channel name.
     * @param value Initial value of the Channel.
     */
	Channel<T>(const char *name, const T &value,
		   const ChannelLogRate &log_rate)
		: m_type(type_hash<T>()), m_name(name), m_log_rate(log_rate)
	{
		(void)name;
		(void)log_rate;
		set_value(value);
	}

	/** Rule of Six */

	/**
     * @brief Default constructor. Deleted as all Channels must be named at creation.
     *
     */
	Channel(void) = delete;

	/**
     * @brief Destroy the Channel object.
     *
     */
	~Channel<T>(void) = default;

	/**
     * @brief Copy constructor.
     *
     * @param other Channel to copy.
     */
	Channel<T>(const Channel<T> &other) = default;

	/**
     * @brief Copy assignment operator.
     *
     * @param other Channel to copy.
     * @return Channel<T>& Reference to this Channel.
     */
	Channel<T> &operator=(const Channel<T> &other) = default;

	/**
     * @brief Move constructor.
     *
     * @param other Channel to move.
     */
	Channel<T>(Channel<T> &&other) noexcept = default;

	/**
     * @brief Move assignment operator.
     *
     * @param other Channel to move.
     * @return Channel<T>& Reference to this Channel.
     */
	Channel<T> &operator=(Channel<T> &&other) noexcept = default;

	/** Getters and Setters */

	/**
     * @brief Get the Channel's name.
     *
     * @return const char * Channel name.
     */
	[[nodiscard]] const char *get_name(void) const override
	{
		return m_name;
	}

	/**
     * @brief Get the Channel's type.
     *
     * @return TYPE_ID Channel type.
     */
	[[nodiscard]] TYPE_ID get_type(void) const override
	{
		return m_type;
	}

	/**
     * @brief Get the Channel's value.
     *
     * @return T Channel value.
     */
	[[nodiscard]] T get_value(void) const
	{
		return m_value;
	}

	/**
	 * @brief Get the Channel's log rate.
	 *
	 * @return ChannelLogRate Logging rate.
	 */
	[[nodiscard]] ChannelLogRate get_log_rate(void) const override
	{
		return m_log_rate;
	}

	/**
     * @brief Set the Channel's value.
     *
     * @param value New value of the Channel.
     */
	void set_value(const T &value) noexcept
	{
		m_value = value;
	}
};

} // namespace Impl
} // namespace System
