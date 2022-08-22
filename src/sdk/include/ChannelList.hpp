#pragma once

#include "Channel.hpp"
#include "ChannelList.hpp"
#include "RTOS.hpp"
#include "portmacro.h"
#include "utils.hpp"
#include <string.h>

extern "C" void Error_Handler(void);

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
	static ChannelBase *m_channels[];

	static uint32_t m_channel_count;

	static TimerHandle_t m_timers[CHANNEL_LOG_COUNT - 1];

    static StaticTimer_t m_timer_buffers[CHANNEL_LOG_COUNT - 1];

	static void timer_cb(TimerHandle_t xTimer);

    public:
	/**
     * @brief Add a new named Channel to the ChannelList.
     *
     * @tparam T Type of the Channel's value.
     * @param name Name of the Channel.
     * @param value Value of the Channel.
     */
	template <typename T>
	static void add_channel(const char *name, const T &value, const ChannelLogRate &log_rate);

	/**
     * @brief Get the value of a named channel.
     *
     * @tparam T Type of the Channel's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Channel.
     * @return T Current value of the Channel.
     */
	template <typename T>
	[[nodiscard]] static T get_channel(const char *name);

	/**
     * @brief Set a new value for a named Channel.
     *
     * @tparam T Type of the Channel's value. Set explicitly to avoid runtime errors.
     * @param name Name of the Channel.
     * @param value New value of the Channel.
     */
	template <typename T>
	static void set_channel(const char *name, const T &value);

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

			m_timers[i] = xTimerCreateStatic(
				"", /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
				ticks, /* The period of the software timer in ticks. */
				true, /* xAutoReload is set to pdTRUE. */
				(void*)frequency, /* The timer's ID is used to store the frequency each callback is running for. */
				timer_cb, /* The function executed when the timer expires. */
                &m_timer_buffers[i]
			);

			if (xTimerStart(m_timers[i], 0) != pdTRUE) {
				printf("Channel logging %dHz timer start failed!\n",
				       frequency);
				Error_Handler();
			}
		}
	}
};

template <typename T>
void ChannelList::add_channel(const char *name, const T &value, const ChannelLogRate &log_rate)
{
	for (int i = 0; i < m_channel_count; i++) {
		auto *channel = m_channels[i];
		if (strcmp(channel->get_name(), name) == 0) {
			printf("Channel '%s' already registered\n", name);
			return;
		}
	}

	m_channels[m_channel_count++] = new Channel<T>(name, value, log_rate);
}

template <typename T> [[nodiscard]] T ChannelList::get_channel(const char *name)
{
	for (int i = 0; i < m_channel_count; i++) {
		auto *channel = m_channels[i];
		if (strcmp(channel->get_name(), name) == 0) {
			return ((Channel<T> *)(channel))->get_value();
		}
	}

	printf("Channel '%s' not found\n", name);
	return {};
}

template <typename T>
void ChannelList::set_channel(const char *name, const T &value)
{
	for (int i = 0; i < m_channel_count; i++) {
		auto *channel = m_channels[i];
		if (strcmp(channel->get_name(), name) == 0) {
			if (type_hash<T>() != channel->get_type()) {
				printf("Channel '%s' set to wrong type, explicitly set the typename template argument\n",
				       name);
			}
			((Channel<T> *)channel)->set_value(value);
			return;
		}
	}

	printf("Channel '%s' not found\n", name);
}

}
}

