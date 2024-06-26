#include "../native.h"
#include "common.h"

// todo: Scheduler

#include "halo3.h"

namespace Halo3::Entry::Engine {
    void Prologue() {
        LOG_INFO("Engine Prologue");
        // open access to main thread resources
        // main thread resources will be copied to the render thread
        NativeHalo3()->NativeInfo()->update((__int64) GetModuleHandleA("halo3.dll"));
    }
    void Epilogue() {
        LOG_INFO("Engine Epilogue");
    }

    Halo3Entry(entry, OFFSET_HALO3_PF_ENGINE, void, detour) {
        Prologue();
        ((detour_t)entry.m_pOriginal)();
        Epilogue();
    }
}