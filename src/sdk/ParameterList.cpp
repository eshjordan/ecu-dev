#include "ParameterList.hpp"

System::Impl::ParameterBase *System::Impl::ParameterList::m_parameters[1000];
uint32_t System::Impl::ParameterList::m_parameter_count = 0;

// clang-format off

template void System::Impl::ParameterList::add_parameter<short int>(const std::string &name, const short int &value);
template void System::Impl::ParameterList::add_parameter<int>(const std::string &name, const int &value);
template void System::Impl::ParameterList::add_parameter<unsigned int>(const std::string &name, const unsigned int &value);
template void System::Impl::ParameterList::add_parameter<unsigned long int>(const std::string &name, const unsigned long int &value);
template void System::Impl::ParameterList::add_parameter<long long int>(const std::string &name, const long long int &value);
template void System::Impl::ParameterList::add_parameter<unsigned long long int>(const std::string &name, const unsigned long long int &value);
template void System::Impl::ParameterList::add_parameter<bool>(const std::string &name, const bool &value);
template void System::Impl::ParameterList::add_parameter<signed char>(const std::string &name, const signed char &value);
template void System::Impl::ParameterList::add_parameter<unsigned char>(const std::string &name, const unsigned char &value);
template void System::Impl::ParameterList::add_parameter<char>(const std::string &name, const char &value);
template void System::Impl::ParameterList::add_parameter<wchar_t>(const std::string &name, const wchar_t &value);
template void System::Impl::ParameterList::add_parameter<char16_t>(const std::string &name, const char16_t &value);
template void System::Impl::ParameterList::add_parameter<char32_t>(const std::string &name, const char32_t &value);
template void System::Impl::ParameterList::add_parameter<float>(const std::string &name, const float &value);
template void System::Impl::ParameterList::add_parameter<double>(const std::string &name, const double &value);
template void System::Impl::ParameterList::add_parameter<long double>(const std::string &name, const long double &value);
template void System::Impl::ParameterList::add_parameter<std::string>(const std::string &name, const std::string &value);

template short int System::Impl::ParameterList::get_parameter<short int> (const std::string &name);
template int System::Impl::ParameterList::get_parameter<int> (const std::string &name);
template unsigned int System::Impl::ParameterList::get_parameter<unsigned int> (const std::string &name);
template unsigned long int System::Impl::ParameterList::get_parameter<unsigned long int> (const std::string &name);
template long long int System::Impl::ParameterList::get_parameter<long long int> (const std::string &name);
template unsigned long long int System::Impl::ParameterList::get_parameter<unsigned long long int> (const std::string &name);
template bool System::Impl::ParameterList::get_parameter<bool> (const std::string &name);
template signed char System::Impl::ParameterList::get_parameter<signed char> (const std::string &name);
template unsigned char System::Impl::ParameterList::get_parameter<unsigned char> (const std::string &name);
template char System::Impl::ParameterList::get_parameter<char> (const std::string &name);
template wchar_t System::Impl::ParameterList::get_parameter<wchar_t> (const std::string &name);
template char16_t System::Impl::ParameterList::get_parameter<char16_t> (const std::string &name);
template char32_t System::Impl::ParameterList::get_parameter<char32_t> (const std::string &name);
template float System::Impl::ParameterList::get_parameter<float> (const std::string &name);
template double System::Impl::ParameterList::get_parameter<double> (const std::string &name);
template long double System::Impl::ParameterList::get_parameter<long double> (const std::string &name);
template std::string System::Impl::ParameterList::get_parameter<std::string> (const std::string &name);

template void System::Impl::ParameterList::set_parameter<short int>(const std::string &name, const short int &value);
template void System::Impl::ParameterList::set_parameter<int>(const std::string &name, const int &value);
template void System::Impl::ParameterList::set_parameter<unsigned int>(const std::string &name, const unsigned int &value);
template void System::Impl::ParameterList::set_parameter<unsigned long int>(const std::string &name, const unsigned long int &value);
template void System::Impl::ParameterList::set_parameter<long long int>(const std::string &name, const long long int &value);
template void System::Impl::ParameterList::set_parameter<unsigned long long int>(const std::string &name, const unsigned long long int &value);
template void System::Impl::ParameterList::set_parameter<bool>(const std::string &name, const bool &value);
template void System::Impl::ParameterList::set_parameter<signed char>(const std::string &name, const signed char &value);
template void System::Impl::ParameterList::set_parameter<unsigned char>(const std::string &name, const unsigned char &value);
template void System::Impl::ParameterList::set_parameter<char>(const std::string &name, const char &value);
template void System::Impl::ParameterList::set_parameter<wchar_t>(const std::string &name, const wchar_t &value);
template void System::Impl::ParameterList::set_parameter<char16_t>(const std::string &name, const char16_t &value);
template void System::Impl::ParameterList::set_parameter<char32_t>(const std::string &name, const char32_t &value);
template void System::Impl::ParameterList::set_parameter<float>(const std::string &name, const float &value);
template void System::Impl::ParameterList::set_parameter<double>(const std::string &name, const double &value);
template void System::Impl::ParameterList::set_parameter<long double>(const std::string &name, const long double &value);
template void System::Impl::ParameterList::set_parameter<std::string>(const std::string &name, const std::string &value);

// clang-format on
