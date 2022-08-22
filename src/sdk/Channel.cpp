#include "Channel.hpp"
#include <cstdint>
#include <string>

namespace System {
namespace Impl {

// All int types
template class Channel<short int>;
template class Channel<int>;
template class Channel<unsigned int>;
template class Channel<unsigned long int>;
template class Channel<long long int>;
template class Channel<unsigned long long int>;

// Boolean type
template class Channel<bool>;

// Char types
template class Channel<signed char>;
template class Channel<unsigned char>;
template class Channel<char>;
template class Channel<wchar_t>;
template class Channel<char16_t>;
template class Channel<char32_t>;

// Floating point types
template class Channel<float>;
template class Channel<double>;
template class Channel<long double>;

// Other useful types
template class Channel<const char *>;

} // namespace Impl
} // namespace System
