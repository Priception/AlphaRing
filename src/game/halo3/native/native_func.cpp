#include "native_func.h"

#include "native_info.h"

class CNativeFunc : public ICNativeFunc {
public:
    bool player_push_message(Index player_index, const wchar_t *msg, int type) override;
    __int64 player_possess(INDEX player_index, INDEX target) override;

    bool players_control_camera(bool custom_control) override;
    bool player_set_camera(Index player_index, eCameraMode mode, float time) override;

};

static CNativeFunc native_func;
ICNativeFunc* g_pICNativeFunc = &native_func;

bool CNativeFunc::player_push_message(Index player_index, const wchar_t *msg, int type) {
    typedef void (__fastcall* func_t)(__int64 a1, const wchar_t *a2, int a3);

    if (player_index > 3) return false;
    if (NativeInfo()->getEntryAddress(OFFSET_HALO3_V_ENTRY_MESSAGE) == 0) return false;

    auto p1 = NativeInfo()->getEntryAddress(OFFSET_HALO3_V_ENTRY_MESSAGE) + 0x1F8Ci64 + 0x28C8i64 * player_index;

    ((func_t)(NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYER_PUSH_MESSAGE))(p1, msg, type);

    return true;
}

__int64 CNativeFunc::player_possess(INDEX player_index, INDEX target) {
    typedef __int64 (__fastcall* func_t) (INDEX player_index, INDEX target);

    if (player_index > 15) return false;

    return ((func_t)(NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYER_POSSESS))(player_index, target);
}

bool CNativeFunc::player_set_camera(Index player_index, eCameraMode mode, float time) {
    typedef bool (__fastcall* func_t) (void* a1, unsigned int a2, float a3, char a4);

    auto p_camera = Camera()->getCamera();

    if (p_camera == nullptr) return false;

    players_control_camera(true);

    if (mode == CAMERAMODE_FOLLOWING) {
        auto p_player = Players()->getPlayerManager()->get(player_index);
        if (p_player == nullptr) return false;
        p_camera->camera[player_index].target = p_player->object_INDEX;
    }

    return ((func_t)(NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYER_SET_CAMERA))(p_camera,mode,time,0);
}

bool CNativeFunc::players_control_camera(bool custom_control) {
    typedef bool (__fastcall* func_t) (bool);
    return ((func_t)(NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYERS_CONTROL_CAMERA))(custom_control);
}
