#include "ParameterList.hpp"

System::Impl::ParameterBase *System::Impl::ParameterList::m_parameters[1000];
uint32_t System::Impl::ParameterList::m_parameter_count = 0;

// clang-format off

template void System::Impl::ParameterList::add_parameter<short int>(const char *name, const short int &value);
template void System::Impl::ParameterList::add_parameter<int>(const char *name, const int &value);
template void System::Impl::ParameterList::add_parameter<unsigned int>(const char *name, const unsigned int &value);
template void System::Impl::ParameterList::add_parameter<unsigned long int>(const char *name, const unsigned long int &value);
template void System::Impl::ParameterList::add_parameter<long long int>(const char *name, const long long int &value);
template void System::Impl::ParameterList::add_parameter<unsigned long long int>(const char *name, const unsigned long long int &value);
template void System::Impl::ParameterList::add_parameter<bool>(const char *name, const bool &value);
template void System::Impl::ParameterList::add_parameter<signed char>(const char *name, const signed char &value);
template void System::Impl::ParameterList::add_parameter<unsigned char>(const char *name, const unsigned char &value);
template void System::Impl::ParameterList::add_parameter<char>(const char *name, const char &value);
template void System::Impl::ParameterList::add_parameter<wchar_t>(const char *name, const wchar_t &value);
template void System::Impl::ParameterList::add_parameter<char16_t>(const char *name, const char16_t &value);
template void System::Impl::ParameterList::add_parameter<char32_t>(const char *name, const char32_t &value);
template void System::Impl::ParameterList::add_parameter<float>(const char *name, const float &value);
template void System::Impl::ParameterList::add_parameter<double>(const char *name, const double &value);
template void System::Impl::ParameterList::add_parameter<long double>(const char *name, const long double &value);
// template void System::Impl::ParameterList::add_parameter<const char *>(const char *name, const char *value);

template short int System::Impl::ParameterList::get_parameter<short int> (const char *name);
template int System::Impl::ParameterList::get_parameter<int> (const char *name);
template unsigned int System::Impl::ParameterList::get_parameter<unsigned int> (const char *name);
template unsigned long int System::Impl::ParameterList::get_parameter<unsigned long int> (const char *name);
template long long int System::Impl::ParameterList::get_parameter<long long int> (const char *name);
template unsigned long long int System::Impl::ParameterList::get_parameter<unsigned long long int> (const char *name);
template bool System::Impl::ParameterList::get_parameter<bool> (const char *name);
template signed char System::Impl::ParameterList::get_parameter<signed char> (const char *name);
template unsigned char System::Impl::ParameterList::get_parameter<unsigned char> (const char *name);
template char System::Impl::ParameterList::get_parameter<char> (const char *name);
template wchar_t System::Impl::ParameterList::get_parameter<wchar_t> (const char *name);
template char16_t System::Impl::ParameterList::get_parameter<char16_t> (const char *name);
template char32_t System::Impl::ParameterList::get_parameter<char32_t> (const char *name);
template float System::Impl::ParameterList::get_parameter<float> (const char *name);
template double System::Impl::ParameterList::get_parameter<double> (const char *name);
template long double System::Impl::ParameterList::get_parameter<long double> (const char *name);
template const char * System::Impl::ParameterList::get_parameter<const char *> (const char *name);

template void System::Impl::ParameterList::set_parameter<short int>(const char *name, const short int &value);
template void System::Impl::ParameterList::set_parameter<int>(const char *name, const int &value);
template void System::Impl::ParameterList::set_parameter<unsigned int>(const char *name, const unsigned int &value);
template void System::Impl::ParameterList::set_parameter<unsigned long int>(const char *name, const unsigned long int &value);
template void System::Impl::ParameterList::set_parameter<long long int>(const char *name, const long long int &value);
template void System::Impl::ParameterList::set_parameter<unsigned long long int>(const char *name, const unsigned long long int &value);
template void System::Impl::ParameterList::set_parameter<bool>(const char *name, const bool &value);
template void System::Impl::ParameterList::set_parameter<signed char>(const char *name, const signed char &value);
template void System::Impl::ParameterList::set_parameter<unsigned char>(const char *name, const unsigned char &value);
template void System::Impl::ParameterList::set_parameter<char>(const char *name, const char &value);
template void System::Impl::ParameterList::set_parameter<wchar_t>(const char *name, const wchar_t &value);
template void System::Impl::ParameterList::set_parameter<char16_t>(const char *name, const char16_t &value);
template void System::Impl::ParameterList::set_parameter<char32_t>(const char *name, const char32_t &value);
template void System::Impl::ParameterList::set_parameter<float>(const char *name, const float &value);
template void System::Impl::ParameterList::set_parameter<double>(const char *name, const double &value);
template void System::Impl::ParameterList::set_parameter<long double>(const char *name, const long double &value);
// template void System::Impl::ParameterList::set_parameter<const char *>(const char *name, const char *value);

// clang-format on
