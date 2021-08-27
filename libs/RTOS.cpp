#include "RTOS.hpp"

void * operator new( size_t size )
{
    return pvPortMalloc( size );
}

void * operator new[]( size_t size )
{
    return pvPortMalloc(size);
}

void operator delete( void * ptr ) noexcept
{
    vPortFree ( ptr );
}

void operator delete[]( void * ptr ) noexcept
{
    vPortFree ( ptr );
}
