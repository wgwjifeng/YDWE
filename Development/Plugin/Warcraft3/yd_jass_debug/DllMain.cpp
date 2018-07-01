#include <windows.h>
#include <base/warcraft3/war3_searcher.h>
#include <base/hook/inline.h>
#include <base/hook/fp_call.h>
#include <base/warcraft3/hashtable.h>
#include <base/warcraft3/jass.h>
#include <base/warcraft3/jass/opcode.h>
#include <base/util/console.h>
#include <base/util/format.h>
#include <iostream>

namespace base { namespace warcraft3 { namespace jdebug {
	
	uintptr_t search_jass_vmmain()
	{
		war3_searcher& s = get_war3_searcher();
		uintptr_t ptr = 0;

		//=========================================
		//  (1)
		//
		//    push    493E0h
		//    push    1
		//    push    1
		//    push    0
		//    mov     edx, offset s_Config ; "config"
		//    mov     ecx, esi
		//    call    UnknowFunc  <----
		//=========================================
		ptr = s.search_string("config");
		ptr += sizeof uintptr_t;
		ptr = next_opcode(ptr, 0xE8, 5);
		ptr = convert_function(ptr);
		//=========================================
		//  (2)
		//
		//  UnknowFunc:
		//    push    esi
		//    mov     esi, edx
		//    call    GetVMInstance
		//    cmp     [esp+4+arg_8], 0
		//    mov     ecx, eax
		//    jz      short loc_6F44C170
		//    cmp     dword ptr [ecx+20h], 0
		//    jz      short loc_6F44C170
		//    call    UnknowFunc2         <----
		//=========================================
		ptr = next_opcode(ptr, 0xE8, 5);
		ptr += 5;
		ptr = next_opcode(ptr, 0xE8, 5);
		ptr = convert_function(ptr);
		//=========================================
		//  (3)
		//
		//  UnknowFunc2:
		//    mov     eax, [ecx+20h]
		//    push    0
		//    push    493E0h
		//    push    0
		//    push    eax
		//    call    JassVMMain    <----
		//    retn
		//=========================================
		ptr = next_opcode(ptr, 0xE8, 5);
		ptr = convert_function(ptr);
		return ptr;
	}

	uintptr_t real_jass_vmmain = 0;

	struct jass::opcode* current_opcode(base::warcraft3::jass_vm_t* vm)
	{
		return *(struct jass::opcode**)((int)vm + 0x20) - 1;
	}

	struct jass::opcode* show_pos(struct jass::opcode* current_op)
	{
		struct jass::opcode *op;
		for (op = current_op; op->opcode_type != jass::OPTYPE_FUNCTION; --op)
		{ }

		std::cout << "    [" << jass::from_stringid(op->arg) << ":" << current_op  - op << "]" << std::endl;
		return op;
	}

	void show_error(base::warcraft3::jass_vm_t* vm, const std::string& msg)
	{
		base::console::enable();
		std::cout << "---------------------------------------" << std::endl;
		std::cout << "               Jass����                " << std::endl;
		std::cout << "---------------------------------------" << std::endl;
		std::cout << msg << std::endl;
		std::cout << std::endl;
		std::cout << "stack traceback:" << std::endl;

		base::warcraft3::stackframe_t* frame = vm->stackframe;
		jass::opcode* op = current_opcode(vm);
		while (op)
		{
			op = show_pos(op);
			if (op->arg == 1) {
				break;
			}
			frame = frame->next;
			uintptr_t code = frame->codes[frame->index]->code;
			op = (jass::opcode*)(vm->symbol_table->unk0 + code * 4);
		}

		std::cout << "---------------------------------------" << std::endl;
	}

	uint32_t __fastcall fake_jass_vmmain(base::warcraft3::jass_vm_t* vm, uint32_t edx, uint32_t unk1, uint32_t unk2, uint32_t limit, uint32_t unk4)
	{
		uint32_t result = base::fast_call<uint32_t>(real_jass_vmmain, vm, edx, unk1, unk2, limit, unk4);

		switch (result)
		{
		case 1:
		case 3:
		case 4:
			break;
		case 2:
			show_error(vm, "�������ֽ�������");
			break;
		case 6:
		{
			jass::opcode* op = current_opcode(vm);
			if (op->opcode_type == jass::OPTYPE_PUSH)
			{
				show_error(vm, base::format("ջ [0x02X] û�г�ʼ����ʹ��", op->r3));
			}
			else
			{
				assert(op->opcode_type == jass::OPTYPE_GETVAR);
				show_error(vm, base::format("���� '%s' û�г�ʼ����ʹ��", jass::from_stringid(op->arg)));
			}
			break;
		}
		case 7:
			show_error(vm, "ʹ������Ϊ����");
			break;
		default:
			show_error(vm, base::format("δ֪���� (%d).", result));
			break;
		}
		return result;
	}

	bool initialize()
	{
		real_jass_vmmain = search_jass_vmmain();
		return base::hook::install(&real_jass_vmmain, (uintptr_t)fake_jass_vmmain);
	}
}}}

void Initialize()
{
	base::warcraft3::jdebug::initialize();
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID pReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
	}

	return TRUE;
}
