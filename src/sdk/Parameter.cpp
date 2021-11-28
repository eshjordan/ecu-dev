#include "Parameter.hpp"
#include <cstdint>
#include <string>

namespace System {
namespace Impl {

// All int types
template class Parameter<short int>;
template class Parameter<int>;
template class Parameter<unsigned int>;
template class Parameter<unsigned long int>;
template class Parameter<long long int>;
template class Parameter<unsigned long long int>;

// Boolean type
template class Parameter<bool>;

// Char types
template class Parameter<signed char>;
template class Parameter<unsigned char>;
template class Parameter<char>;
template class Parameter<wchar_t>;
template class Parameter<char16_t>;
template class Parameter<char32_t>;

// Floating point types
template class Parameter<float>;
template class Parameter<double>;
template class Parameter<long double>;

// Other useful types
template class Parameter<const char *>;

} // namespace Impl
} // namespace System
