#include <list>
#include <vector>
#include <cstring>
#include <thread>
#include <jni.h>
#include "RuntimeDump.h"
#include <unistd.h>
#include <android/log.h>

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Includes/Macros.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Mod_Menu", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Mod_Menu", __VA_ARGS__)

// =====================================================
// Global State
// =====================================================
static std::string itemNameToSpawn = "Level Up";
static volatile bool triggerSpawn = false;

// AttackComponent الحقيقي (مضمون)
static void* g_AttackComponent = nullptr;

// =====================================================
// Game Functions (Offsets من dump/script)
// =====================================================
static void* (*GetItemByName)(void* attackComp, void* nameStr) = nullptr;
static bool  (*ApplyItem)(void* item, void* attackComp) = nullptr;

// =====================================================
// Menu
// =====================================================
jobjectArray GetFeatureList(JNIEnv* env, jobject) {
    const char* features[] = {
        OBFUSCATE("Category_Item Spawner"),
        OBFUSCATE("InputText_Item Name"),
        OBFUSCATE("Button_Spawn / Apply Item")
    };

    int n = (int)(sizeof(features) / sizeof(features[0]));
    jobjectArray ret = env->NewObjectArray(
        n,
        env->FindClass("java/lang/String"),
        env->NewStringUTF("")
    );

    for (int i = 0; i < n; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return ret;
}

void Changes(JNIEnv* env, jclass, jobject, jint featNum,
             jstring, jint, jlong, jboolean, jstring text) {

    LOGI("Changes called! featNum=%d", featNum);

    // InputText
    if (text) {
        const char* s = env->GetStringUTFChars(text, 0);
        itemNameToSpawn = s ? s : "";
        env->ReleaseStringUTFChars(text, s);
        LOGI("Item name set to: %s", itemNameToSpawn.c_str());
        return;
    }

    // Button
    if (featNum >= 0) {
        triggerSpawn = true;
        LOGI("Spawn triggered");
    }
}

// =====================================================
// Hook: AttackComponent::OnTriggerEnter(Collider)
// Offset: 0x41C3738
// =====================================================
static void (*old_OnTriggerEnter)(void* attackComp, void* collider) = nullptr;

static void OnTriggerEnter_Hook(void* attackComp, void* collider) {
    if (attackComp && g_AttackComponent != attackComp) {
        g_AttackComponent = attackComp;
        LOGI("AttackComponent captured via OnTriggerEnter: %p", g_AttackComponent);
    }
    old_OnTriggerEnter(attackComp, collider);
}

// =====================================================
// Worker: ينفّذ Spawn/Apply بدون Update
// =====================================================
static void worker_thread() {
    while (true) {
        if (triggerSpawn && g_AttackComponent && !itemNameToSpawn.empty()) {

            LOGI("=== APPLY ITEM START ===");
            LOGI("AttackComponent: %p", g_AttackComponent);
            LOGI("Item Name: %s", itemNameToSpawn.c_str());

            void* item = nullptr;
            if (GetItemByName) {
                item = GetItemByName(
                    g_AttackComponent,
                    (void*)itemNameToSpawn.c_str()
                );
            }

            LOGI("GetItemByName returned: %p", item);

            if (item && ApplyItem) {
                bool ok = ApplyItem(item, g_AttackComponent);
                LOGI("ApplyItem result: %d", ok);
            } else {
                LOGE("Item not found or ApplyItem null");
            }

            triggerSpawn = false;
            LOGI("=== APPLY ITEM END ===");
        }

        usleep(50 * 1000); // 50ms
    }
}

// =====================================================
// Init / Hooks
// =====================================================
#define targetLibName OBFUSCATE("libil2cpp.so")

static void hack_thread() {
    LOGI("Mod started, waiting for libil2cpp.so...");

    while (!isLibraryLoaded(targetLibName))
        sleep(1);

    LOGI("libil2cpp.so loaded!");

#if defined(__aarch64__)
    // Offsets (حسب اللي عندك شغال)
    GetItemByName = (void* (*)(void*, void*))
        getAbsoluteAddress(targetLibName, 0x3E1B044);

    ApplyItem = (bool (*)(void*, void*))
        getAbsoluteAddress(targetLibName, 0x41C2CB0);

    // Hook OnTriggerEnter لالتقاط AttackComponent
    HOOK(targetLibName, 0x41C3738, OnTriggerEnter_Hook, old_OnTriggerEnter);

    // Initialize RuntimeDump hook (after libil2cpp.so is loaded)
    RuntimeDump::Initialize();

    LOGI("Hooks installed!");
    LOGI("GetItemByName: %p", GetItemByName);
    LOGI("ApplyItem: %p", ApplyItem);

    std::thread(worker_thread).detach();
#endif
}

__attribute__((constructor))
void lib_main() {
    std::thread(hack_thread).detach();
}
