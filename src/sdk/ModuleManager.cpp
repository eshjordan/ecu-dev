#include "ModuleManager.hpp"

System::Impl::Module System::Impl::ModuleManager::m_module_init_fns[16];
uint32_t System::Impl::ModuleManager::m_module_count = 0;

uint32_t System::Impl::ModuleManager::register_module_init(Module init_fn)
{
	m_module_init_fns[m_module_count++] = init_fn;
	return 0;
}

void System::Impl::ModuleManager::init_modules(void)
{
	for (uint32_t i = 0; i < m_module_count; i++) {
		m_module_init_fns[i]();
	}
}
