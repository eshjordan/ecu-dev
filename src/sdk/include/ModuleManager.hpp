#pragma once

#include <stdint.h>

namespace System
{
namespace Impl
{

using Module = void (*)(void);

class ModuleManager {
    private:
	static uint32_t m_module_count;
	static Module m_module_init_fns[];

    public:
	static uint32_t register_module_init(Module init_fn);

	static void init_modules(void);
};

} // namespace Impl
} // namespace System
