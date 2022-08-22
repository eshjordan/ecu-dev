#include "ChannelList.hpp"

System::Impl::ChannelBase *System::Impl::ChannelList::m_channels[32];
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
        Error_Handler();
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


// clang-format off

template void System::Impl::ChannelList::add_channel<short int>(const char *name, const short int &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<int>(const char *name, const int &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<unsigned int>(const char *name, const unsigned int &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<unsigned long int>(const char *name, const unsigned long int &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<long long int>(const char *name, const long long int &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<unsigned long long int>(const char *name, const unsigned long long int &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<bool>(const char *name, const bool &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<signed char>(const char *name, const signed char &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<unsigned char>(const char *name, const unsigned char &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<char>(const char *name, const char &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<wchar_t>(const char *name, const wchar_t &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<char16_t>(const char *name, const char16_t &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<char32_t>(const char *name, const char32_t &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<float>(const char *name, const float &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<double>(const char *name, const double &value, const ChannelLogRate &log_rate);
template void System::Impl::ChannelList::add_channel<long double>(const char *name, const long double &value, const ChannelLogRate &log_rate);
// template void System::Impl::ChannelList::add_channel<const char *>(const char *name, const char *value, const ChannelLogRate &log_rate);

template short int System::Impl::ChannelList::get_channel<short int> (const char *name);
template int System::Impl::ChannelList::get_channel<int> (const char *name);
template unsigned int System::Impl::ChannelList::get_channel<unsigned int> (const char *name);
template unsigned long int System::Impl::ChannelList::get_channel<unsigned long int> (const char *name);
template long long int System::Impl::ChannelList::get_channel<long long int> (const char *name);
template unsigned long long int System::Impl::ChannelList::get_channel<unsigned long long int> (const char *name);
template bool System::Impl::ChannelList::get_channel<bool> (const char *name);
template signed char System::Impl::ChannelList::get_channel<signed char> (const char *name);
template unsigned char System::Impl::ChannelList::get_channel<unsigned char> (const char *name);
template char System::Impl::ChannelList::get_channel<char> (const char *name);
template wchar_t System::Impl::ChannelList::get_channel<wchar_t> (const char *name);
template char16_t System::Impl::ChannelList::get_channel<char16_t> (const char *name);
template char32_t System::Impl::ChannelList::get_channel<char32_t> (const char *name);
template float System::Impl::ChannelList::get_channel<float> (const char *name);
template double System::Impl::ChannelList::get_channel<double> (const char *name);
template long double System::Impl::ChannelList::get_channel<long double> (const char *name);
template const char * System::Impl::ChannelList::get_channel<const char *> (const char *name);

template void System::Impl::ChannelList::set_channel<short int>(const char *name, const short int &value);
template void System::Impl::ChannelList::set_channel<int>(const char *name, const int &value);
template void System::Impl::ChannelList::set_channel<unsigned int>(const char *name, const unsigned int &value);
template void System::Impl::ChannelList::set_channel<unsigned long int>(const char *name, const unsigned long int &value);
template void System::Impl::ChannelList::set_channel<long long int>(const char *name, const long long int &value);
template void System::Impl::ChannelList::set_channel<unsigned long long int>(const char *name, const unsigned long long int &value);
template void System::Impl::ChannelList::set_channel<bool>(const char *name, const bool &value);
template void System::Impl::ChannelList::set_channel<signed char>(const char *name, const signed char &value);
template void System::Impl::ChannelList::set_channel<unsigned char>(const char *name, const unsigned char &value);
template void System::Impl::ChannelList::set_channel<char>(const char *name, const char &value);
template void System::Impl::ChannelList::set_channel<wchar_t>(const char *name, const wchar_t &value);
template void System::Impl::ChannelList::set_channel<char16_t>(const char *name, const char16_t &value);
template void System::Impl::ChannelList::set_channel<char32_t>(const char *name, const char32_t &value);
template void System::Impl::ChannelList::set_channel<float>(const char *name, const float &value);
template void System::Impl::ChannelList::set_channel<double>(const char *name, const double &value);
template void System::Impl::ChannelList::set_channel<long double>(const char *name, const long double &value);
// template void System::Impl::ChannelList::set_channel<const char *>(const char *name, const char *value);

// clang-format on
