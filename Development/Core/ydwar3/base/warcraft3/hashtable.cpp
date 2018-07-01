#include <base/warcraft3/hashtable.h>
#include <base/warcraft3/war3_searcher.h>
#include <base/warcraft3/version.h>		   
#include <base/warcraft3/jass/opcode.h>

namespace base { namespace warcraft3 {

	jass_vm_t* get_jass_vm(int index)
	{
		uintptr_t ptr = get_war3_searcher().get_instance(5);
		uintptr_t vm = *(uintptr_t*)(*(uintptr_t*)(ptr + 0x90) + 0x04 * index);
		return vm ? (jass_vm_t*)(vm - ((get_war3_searcher().get_version() > version_120e) ? 0 : 4)) : nullptr;
	}

	uintptr_t get_jass_thread()
	{
		uintptr_t ptr = get_war3_searcher().get_instance(5);
		return *(uintptr_t*)(*(uintptr_t*)(ptr + 0x0C) + 4 * *(uintptr_t*)(ptr + 0x14) - 4);
	}

	hashtable::native_func_table* get_native_function_hashtable()
	{
		return (hashtable::native_func_table*)(get_war3_searcher().get_instance(5)+0x18);
	}

	uintptr_t get_current_jass_pos()
	{
		uintptr_t thread = get_jass_thread();
		return (uintptr_t)(*(jass::opcode**)(thread + 0x20) - 1);
	}
}}
