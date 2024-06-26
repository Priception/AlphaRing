#include "mcc.h"

#include "common.h"
#include "module/Module.h"
#include "offset_mcc.h"

#include "MinHook.h"

#include "core/String.h"
#include "input/Input.h"
#include "render/Renderer.h"

#include "./setting/setting.h"

static __int64 hModule;

static void (__fastcall *ppOriginal_ModuleLoad)(module_info_t* info, int a2, __int64 a3);
static void __fastcall MODULE_LOAD_DETOUR(module_info_t* info, int a2, __int64 a3) {
    ppOriginal_ModuleLoad(info, a2, a3);
    Modules()->get((eModule)info->title)->load_module(info);
}

static __int64 (__fastcall *ppOriginal_ModuleUnload)(module_info_t* info);
static __int64 __fastcall MODULE_UNLOAD_DETOUR(module_info_t* info) {
    auto result = ppOriginal_ModuleUnload(info);
    Modules()->get((eModule)info->title)->unload_module();
    return result;
}

struct InputDevice {
    struct method_table_t {
        char un[0x38];
        __int64 (__fastcall *set_state)(InputDevice*, float, void*);
        void (__fastcall *check)(InputDevice*);
    } *p_method_table;
    char un0[0x800];
    int input_user;// 0x808
    char un1[0x110];
    XINPUT_STATE state; // 0x91C
};
struct INPUT_t {
    void** pp_method_table;
    InputDevice* p_input_device[5];
};

static bool (__fastcall *ppOriginal_input_get_status)(INPUT_t* self, int player_index, input_data_t* p_data, char a4);
static bool __fastcall input_get_status(INPUT_t* self, int player_index, input_data_t* p_data, char a4) {
    typedef void (__fastcall* get_player0_input)(INPUT_t* self);
    bool result = false;
    InputDevice* p_device;

    if (Renderer()->ShowContext()) return false;

    auto p_setting = InputSetting();

    if (!p_setting->override_input) return ppOriginal_input_get_status(self, player_index, p_data, a4);

//    if (!*(bool*)(hModule + OFFSET_MCC_PV_WINDOWFOCUSED)) return false;

    if (p_setting->enable_km && !player_index) {
        p_device = self->p_input_device[4];
        (get_player0_input(hModule + OFFSET_MCC_PF_GET_PLAYER0_INPUT))(self);
    } else {
        auto choice = p_setting->controller_map[player_index];
        if (choice == 4 || (p_device = self->p_input_device[choice]) == nullptr) return true;
        memset(&p_device->state, 0, sizeof(XINPUT_STATE));
        AlphaRing::Input::GetXInputGetState(p_device->input_user, &p_device->state);
        *(__int64 *)((__int64)p_data + 0x10C) = 0i64;
    }

    result = p_device->p_method_table->set_state(p_device, 0, p_data);
    p_device->p_method_table->check(p_device);

    return result;
}

char (__fastcall* ppOriginal_get_xbox_user_id)(__int64 ,__int64* ,wchar_t *,unsigned int ,unsigned int );

char __fastcall get_xbox_user_id(
        __int64 p_self,
        __int64* p_userId,
        wchar_t *p_gameTag,
        unsigned int size,
        unsigned int player_index) {
    auto p_setting = ProfileSetting();
    
    if (!p_setting->b_override) return ppOriginal_get_xbox_user_id(p_self,p_userId,p_gameTag,size,player_index);

    if (!p_setting->b_override_player0 && !player_index) return ppOriginal_get_xbox_user_id(p_self,p_userId,p_gameTag,size,player_index);

    if (player_index >= p_setting->player_count) return false;

    if (p_userId)
        *p_userId = p_setting->profiles[player_index].id;

    if (p_gameTag)
        String::wstrcpy(p_gameTag, p_setting->profiles[player_index].name, size >> 1);

    return true;
}

bool MCCHook::Initialize() {
    char buffer[1024];
    void* pTarget;

    if ((hModule = (__int64)GetModuleHandleA("MCC-Win64-Shipping.exe")) == 0) return false;

    auto p_mcc = Modules()->get(MCC);

    p_mcc->load_module({MCC,0, hModule});

    p_mcc->version().toString(buffer, sizeof(buffer));

    LOG_INFO("Game Version: {}", buffer);

    if ((pTarget = (LPVOID) (hModule + OFFSET_MCC_PF_MODULELOAD)),
            MH_CreateHook(pTarget, MODULE_LOAD_DETOUR,(void **) &ppOriginal_ModuleLoad) != MH_OK ||
            MH_EnableHook(pTarget) != MH_OK)
        return false;

    if ((pTarget = (LPVOID) (hModule + OFFSET_MCC_PF_MODULEUNLOAD)),
            MH_CreateHook(pTarget,MODULE_UNLOAD_DETOUR,(void **) &ppOriginal_ModuleUnload) != MH_OK ||
            MH_EnableHook(pTarget) != MH_OK)
        return false;

    if ((pTarget = (LPVOID) (hModule + OFFSET_MCC_PF_GAMEINPUT)),
            MH_CreateHook(pTarget,input_get_status,(void **) &ppOriginal_input_get_status) != MH_OK ||
            MH_EnableHook(pTarget) != MH_OK)
        return false;

    if ((pTarget = (LPVOID) (hModule + 0x1E572C)),
            MH_CreateHook(pTarget,get_xbox_user_id,(void **) &ppOriginal_get_xbox_user_id) != MH_OK ||
            MH_EnableHook(pTarget) != MH_OK)
        return false;

    return true;
}
