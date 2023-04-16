#pragma once

#include "Channel.hpp"
#include "ChannelList.hpp"
#include "Error.hpp"
#include "RTOS.hpp"
#include "portmacro.h"
#include "utils.hpp"
#include <stdio.h>
#include <string.h>

namespace System
{
namespace Impl
{

/**
 * @brief Singleton record of all managed Channels. Can contain Channels with values of different types.
 *
 */
class ChannelList {
    private:
	/** Member variables */

	/** @brief List of ChannelBase pointers. */
	static Channel *m_channels[];

	static uint32_t m_channel_count;

	static TimerHandle_t m_timers[CHANNEL_LOG_COUNT - 1];

    static StaticTimer_t m_timer_buffers[CHANNEL_LOG_COUNT - 1];

	static void timer_cb(TimerHandle_t xTimer);

public:

	static void add_channel(const char *name, const ParameterValue & value, const ChannelLogRate &log_rate)
	{
		char tmp_name[64];
		for (int i = 0; i < m_channel_count; i++) {
			auto *channel = m_channels[i];
			channel->get_name(tmp_name);
			if (strcmp(tmp_name, name) == 0) {
				ecu_fatal_error("Channel '%s' already registered\n", name);
				return;
			}
		}

		m_channels[m_channel_count++] = new Channel(name, value, log_rate);
	}

	/**
     * @brief Add a new named Channel to the ChannelList.
     *
     * @tparam T Type of the Channel's value.
     * @param name Name of the Channel.
     * @param value Value of the Channel.
     */
	template <typename T>
	static void add_channel(const char *name, T value, const ChannelLogRate &log_rate) {
		add_channel(name, ParameterValue(value), log_rate);
	}

	/**
     * @brief Get the value of a named channel.
     *
     * @tparam T Type of the Channel's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Channel.
     * @return T Current value of the Channel.
     */
	template<typename T>
	[[nodiscard]]
	static
	decltype(auto)
	get_channel_value(const char *name)
	{
		char tmp_name[64];
		for (int i = 0; i < m_channel_count; i++) {
			auto *channel = m_channels[i];
			channel->get_name(tmp_name);
			if (strcmp(tmp_name, name) == 0) {
				return channel->get_value<T>();
			}
		}

		ecu_fatal_error("Channel '%s' not found\n", name);

		// Should never actually return, but needed to prevent type determination compilation errors
		return m_channels[0]->get_value<T>();
	}

	/**
     * @brief Set a new value for a named Channel.
     *
     * @tparam T Type of the Channel's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Channel.
     * @param value New value of the Channel.
     */
	template<typename T>
	static
	void
	set_channel_value_(const char *name, const ParameterValue &value)
	{
		char tmp_name[64];
		for (int i = 0; i < m_channel_count; i++) {
			auto *channel = m_channels[i];
			channel->get_name(tmp_name);
			if (strcmp(tmp_name, name) == 0) {
				channel->set_value(value);
				return;
			}
		}

		ecu_fatal_error("Channel '%s' not found\n", name);
		return;
	}

	/**
     * @brief Set a new value for a named Channel.
     *
     * @tparam T Type of the Channel's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Channel.
     * @param value New value of the Channel.
     */
	template<typename T>
	static
	void
	set_channel_value(const char *name, const ParameterValue &value)
	{
		set_channel_value_<T>(name, value);
	}

	/**
     * @brief Set a new value for a named Channel.
     *
     * @tparam T Type of the Channel's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Channel.
     * @param value New value of the Channel.
     */
	template<typename T>
	static
	void
	set_channel_value(const char *name, T value)
	{
		set_channel_value_<T>(name, ParameterValue(value));
	}

	/**
     * @brief Get the number of Channels in the ChannelList.
     *
     * @return size_t Number of Channels in the ChannelList.
     */
	[[nodiscard]] static inline size_t get_size(void)
	{
		return m_channel_count;
	}

	static inline void start_logging(void)
	{
		uint16_t timer_values[] = {
			CHANNEL_LOG_1HZ,   CHANNEL_LOG_10HZ,  CHANNEL_LOG_20HZ,
			CHANNEL_LOG_50HZ,  CHANNEL_LOG_100HZ, CHANNEL_LOG_200HZ,
			CHANNEL_LOG_500HZ, CHANNEL_LOG_1000HZ
		};

		for (int i = 0; i < ARRAY_SIZE(timer_values); i++) {
			uint32_t frequency = timer_values[i];
			const auto period = (size_t)(1000.0 / frequency);
			const TickType_t ticks = pdMS_TO_TICKS(period);
			char timer_name[12];

			switch (frequency) {
			case CHANNEL_LOG_1HZ: {
				strcpy(timer_name, "1Hz_tmr");
				break;
			} case CHANNEL_LOG_10HZ: {
				strcpy(timer_name, "10Hz_tmr");
				break;
			} case CHANNEL_LOG_20HZ: {
				strcpy(timer_name, "20Hz_tmr");
				break;
			} case CHANNEL_LOG_50HZ: {
				strcpy(timer_name, "50Hz_tmr");
				break;
			} case CHANNEL_LOG_100HZ: {
				strcpy(timer_name, "100Hz_tmr");
				break;
			} case CHANNEL_LOG_200HZ: {
				strcpy(timer_name, "200Hz_tmr");
				break;
			} case CHANNEL_LOG_500HZ: {
				strcpy(timer_name, "500Hz_tmr");
				break;
			} case CHANNEL_LOG_1000HZ: {
				strcpy(timer_name, "1000Hz_tmr");
				break;
			} default: {
				break;
			}
			}

			m_timers[i] = xTimerCreateStatic(
				timer_name, /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
				ticks, /* The period of the software timer in ticks. */
				true, /* xAutoReload is set to pdTRUE. */
				(void*)frequency, /* The timer's ID is used to store the frequency each callback is running for. */
				timer_cb, /* The function executed when the timer expires. */
                &m_timer_buffers[i]
			);

			if (xTimerStart(m_timers[i], 0) != pdTRUE) {
				ecu_fatal_error("Channel logging %ldHz timer start failed!\n", frequency);
			}
		}
	}
};


}
}
