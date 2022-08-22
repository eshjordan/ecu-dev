#pragma once

#include "Parameter.hpp"
#include <stdint.h>

namespace System
{
namespace Impl
{

template <typename T> class Channel : public Parameter<T> {
    private:
	uint16_t m_log_rate;

    public:
	Channel(const char *name, const T &value, uint16_t log_rate)
		: Parameter<T>(name, value), m_log_rate(log_rate)
	{
	}

	[[nodiscard]] inline uint16_t get_log_rate(void) const
	{
		return m_log_rate;
	}
};

} // namespace Impl
} // namespace System
