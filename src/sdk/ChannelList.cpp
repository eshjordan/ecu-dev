#include "ChannelList.hpp"
#include "stdlib.h"

System::Impl::Channel *System::Impl::ChannelList::m_channels[32];
uint32_t System::Impl::ChannelList::m_channel_count = 0;
TimerHandle_t System::Impl::ChannelList::m_timers[];
StaticTimer_t System::Impl::ChannelList::m_timer_buffers[];


void System::Impl::ChannelList::timer_cb(TimerHandle_t xTimer)
{
    auto frequency = reinterpret_cast<uint64_t>(pvTimerGetTimerID(xTimer));
    switch (frequency) {
    case ChannelLogRate::CHANNEL_LOG_1HZ:
    case ChannelLogRate::CHANNEL_LOG_10HZ:
    case ChannelLogRate::CHANNEL_LOG_20HZ:
    case ChannelLogRate::CHANNEL_LOG_50HZ:
    case ChannelLogRate::CHANNEL_LOG_100HZ:
    case ChannelLogRate::CHANNEL_LOG_200HZ:
    case ChannelLogRate::CHANNEL_LOG_500HZ:
    case ChannelLogRate::CHANNEL_LOG_1000HZ: {
        break;
    }
    case ChannelLogRate::CHANNEL_LOG_OFF:
    case ChannelLogRate::CHANNEL_LOG_COUNT:
    default: {
        ecu_fatal_error("Invalid channel log rate set");
        break;
    }
    }

    for (uint32_t i = 0; i < m_channel_count; i++) {
        if (m_channels[i]->get_log_rate() == frequency) {
//            char buffer[128] = {0};
//            sprintf(buffer, "%s - %s - %s", m_channels[i]->get_name(), type_id_to_str(m_channels[i]->get_type()), type_fmt_spc(m_channels[i]->get_type()));
//            switch(m_channels[i]->get_type()) {
//                case TYPE_ID::SHORT_INT: {
//                    auto channel = reinterpret_cast<Channel<short int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::INT: {
//                    auto channel = reinterpret_cast<Channel<int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::LONG_INT: {
//                    auto channel = reinterpret_cast<Channel<long int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::LONG_LONG_INT: {
//                    auto channel = reinterpret_cast<Channel<long long int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::UNSIGNED_SHORT_INT: {
//                    auto channel = reinterpret_cast<Channel<unsigned short int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::UNSIGNED_INT: {
//                    auto channel = reinterpret_cast<Channel<unsigned int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::UNSIGNED_LONG_INT: {
//                    auto channel = reinterpret_cast<Channel<unsigned long int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::UNSIGNED_LONG_LONG_INT: {
//                    auto channel = reinterpret_cast<Channel<unsigned long long int>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::BOOL: {
//                    auto channel = reinterpret_cast<Channel<bool>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::SIGNED_CHAR: {
//                    auto channel = reinterpret_cast<Channel<signed char>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::UNSIGNED_CHAR: {
//                    auto channel = reinterpret_cast<Channel<unsigned char>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::CHAR: {
//                    auto channel = reinterpret_cast<Channel<char>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::WCHAR: {
//                    auto channel = reinterpret_cast<Channel<wchar_t>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::CHAR16: {
//                    auto channel = reinterpret_cast<Channel<char16_t>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::CHAR32: {
//                    auto channel = reinterpret_cast<Channel<char32_t>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::FLOAT: {
//                    auto channel = reinterpret_cast<Channel<float>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::DOUBLE: {
//                    auto channel = reinterpret_cast<Channel<double>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                case TYPE_ID::LONG_DOUBLE: {
//                    auto channel = reinterpret_cast<Channel<long double>*>(m_channels[i]);
//                    // printf(buffer, channel->get_value());
//                    break;
//                }
//                // case TYPE_ID::STRING: {
//                //     auto channel = reinterpret_cast<Channel<const char *>*>(m_channels[i]);
//                //     // printf(buffer, channel->get_value());
//                //     break;
//                // }
//                case TYPE_ID::STRING:
//                default: {
//                    Error_Handler();
//                    break;
//                }
//            }
        }
    }
}
