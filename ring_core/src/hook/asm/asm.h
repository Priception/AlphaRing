#ifndef ALPHA_RING_ASM_H
#define ALPHA_RING_ASM_H


extern "C" __int64 this_thread_get_teb();

//extern "C" void asm_hook_engine_init(void* f,void* ori);
//extern "C" void asm_hook_engine_entry();

extern "C" void asm_hook_script_init(void* f,void* ori);
extern "C" void asm_hook_script_entry();

#endif //ALPHA_RING_ASM_H