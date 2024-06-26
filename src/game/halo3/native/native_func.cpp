#include "native_func.h"

#include "native_halo3.h"
#include "core/String.h"

#include <cstring>

class CNativeFunc : public ICNativeFunc {
public:
    bool player_push_message(Index player_index, const wchar_t *msg, int type) override;
    __int64 player_possess(INDEX player_index, INDEX target) override;

    bool players_control_camera(bool custom_control) override;
    bool player_set_camera(Index player_index, eCameraMode mode, float time) override;

    INDEX local_player_add(const wchar_t *name, const wchar_t *id) override;

    INDEX object_create(Datum datum, const Vector3 &position) override;
};

static CNativeFunc native_func;
ICNativeFunc* g_pICHalo3NativeFunc = &native_func;

bool CNativeFunc::player_push_message(Index player_index, const wchar_t *msg, int type) {
    typedef void (__fastcall* func_t)(chud_t::player_chud_t::msg_t* a1, const wchar_t *a2, int a3);

    if (player_index >= MAX_LOCAL_PLAYERS) return false;

    auto p_chud = NativeHalo3()->Render()->getChud();

    if (p_chud == nullptr) return false;

    ((func_t)(NativeHalo3()->NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYER_PUSH_MESSAGE))(&p_chud->player[player_index].msg, msg, type);

    return true;
}

__int64 CNativeFunc::player_possess(INDEX player_index, INDEX target) {
    typedef __int64 (__fastcall* func_t) (INDEX player_index, INDEX target);

    if ((__int16)player_index >= MAX_ONLINE_PLAYERS) return false;

    return ((func_t)(NativeHalo3()->NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYER_POSSESS))(player_index, target);
}

bool CNativeFunc::player_set_camera(Index player_index, eCameraMode mode, float time) {
    typedef bool (__fastcall* func_t) (void* a1, unsigned int a2, float a3, char a4);

    auto p_cameras = NativeHalo3()->Camera()->getCamera();

    if (p_cameras == nullptr) return false;

    auto p_camera = &p_cameras->camera[player_index];

    players_control_camera(true);

    if (mode == CAMERAMODE_FOLLOWING)
        p_camera->target = NativeHalo3()
                ->Players()
                ->getPlayerManager()
                ->get(player_index)
                ->object_INDEX;

    return ((func_t)(NativeHalo3()->NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYER_SET_CAMERA))(p_camera, mode, time, 0);
}

bool CNativeFunc::players_control_camera(bool custom_control) {
    typedef bool (__fastcall* func_t) (bool);
    return ((func_t)(NativeHalo3()->NativeInfo()->getModuleAddress() + OFFSET_HALO3_PF_PLAYERS_CONTROL_CAMERA))(custom_control);
}

INDEX CNativeFunc::local_player_add(const wchar_t *name, const wchar_t *id) {
    typedef __int64 (__fastcall* func_init_t) (INDEX index, void* a2, bool a3);
    player_init_t new_player;

    auto p_player_mng = NativeHalo3()
            ->Players()
            ->getPlayerManager();

    Index index = p_player_mng->m_size;

    if (index < 0 || index > 3) return NONE;

    memset(&new_player, 0, sizeof(new_player));

    new_player.v_true = true;
    new_player.user_input = index;
    new_player.input_map = index;

    auto p_action = NativeHalo3()->NativeInfo()->getEntryAddress(OFFSET_HALO3_V_ENTRY_PLAYERS_ACTION);

    new_player.respawn_flag = *(int*)(p_action + 12);
    new_player.unun = *(__int16*)(p_action + 12 + 4);

    if (name) {
        String::wstrcpy(new_player.name, name);
        String::wstrcpy(new_player.name2, name);
    }

    if (id) {
        String::wstrcpy(new_player.id, id);
    }

    return ((func_init_t)(NativeHalo3()->NativeInfo()->getModuleAddress() + 0xE1DBC))(index, &new_player, false);
}

INDEX CNativeFunc::object_create(Datum datum, const Vector3 &position) {
    struct NewObjectData {
        Datum Datum;//0X0
        __int8 v0[0x8];
        __int32 VariantID;//0xC
        __int8 v1[0x8];
        __int8 flags;//0x18 (0x10)
        Vector3 position;//0x1c
        Vector3 forward;
        Vector3 up;
        Vector3 translational_velocity;
        Vector3 angular_velocity;
        float scale;
        __int8 v3[0x124];
    } newObjectData;//0x180

    INDEX result = NONE;

    typedef void (__fastcall* func_object_init)(NewObjectData *a1, Datum datum, INDEX parent, __int64 a4);
    typedef INDEX (__fastcall* func_object_create)(NewObjectData *a1);
    typedef bool (__fastcall* func_object_network_create)(INDEX object);

    ((func_object_init)(NativeHalo3()->NativeInfo()->getModuleAddress() + 0x341158))(&newObjectData, datum, NONE, 0);

    newObjectData.position = position;

    result = ((func_object_create)(NativeHalo3()->NativeInfo()->getModuleAddress() + 0x341738))(&newObjectData);

    if (result == NONE) return result;

    ((func_object_network_create)(NativeHalo3()->NativeInfo()->getModuleAddress() + 0x1EA04))(result);

    return result;
}
