#include "RuntimeDump.h"
#include "And64InlineHook/And64InlineHook.hpp"
#include <jni.h>
#include <android/log.h>
#include "Includes/Macros.h"

#define LOG_TAG "AntigravityDump"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

bool RuntimeDump::g_hasDumped = false;

// Function Pointer for Original OnTriggerEnter
void (*orig_OnTriggerEnter)(void* __this, void* collider);

// Hook Information
void* g_il2cppBase = nullptr; // Needs to be set on load

// Detour Function
void hook_OnTriggerEnter(void* __this, void* collider) {
    if (!RuntimeDump::g_hasDumped && __this != nullptr) {
        LOGI("OnTriggerEnter captured! Starting Dump...");
        RuntimeDump::PerformDump(__this);
    }
    
    // Call Original
    orig_OnTriggerEnter(__this, collider);
}

void RuntimeDump::Initialize() {
#if defined(__aarch64__)
    // Install hook using project's HOOK macro
    HOOK("libil2cpp.so", OFFSET_ATTACKCOMPONENT_ONTRIGGERENTER, hook_OnTriggerEnter, orig_OnTriggerEnter);
    LOGI("RuntimeDump hook installed at 0x%X", OFFSET_ATTACKCOMPONENT_ONTRIGGERENTER);
#endif
    LOGI("RuntimeDump Initialized. Waiting for Game Trigger...");
}

void RuntimeDump::PerformDump(void* attackComponent) {
    if (g_hasDumped) return;

    std::ofstream outFile(DUMP_FILE_PATH);
    if (!outFile.is_open()) {
        LOGI("Failed to open dump file: %s", DUMP_FILE_PATH);
        return;
    }

    LOGI("Dump file opened. Reading memory...");

    // 1. Get List
    void* listPtr = *(void**)((uintptr_t)attackComponent + OFFSET_ATTACKCOMPONENT_ITEMLIST);
    if (!listPtr) {
        LOGI("itemList is null.");
        outFile << "Error: itemList is null.\n";
        outFile.close();
        return;
    }

    Il2CppList* list = (Il2CppList*)listPtr;
    int32_t size = list->size;
    LOGI("List Size: %d", size);
    outFile << "Item List Size: " << size << "\n";
    outFile << "--------------------------------------------------\n";

    if (size <= 0 || !list->items) {
        outFile.close();
        g_hasDumped = true;
        return;
    }

    // 2. Iterate Array
    Il2CppArray* itemsArray = (Il2CppArray*)list->items;
    void** itemVector = (void**)((uintptr_t)itemsArray + OFFSET_ARRAY_DATA);

    for (int i = 0; i < size; i++) {
        void* item = itemVector[i];
        if (!item) continue;

        outFile << "[Item " << i << "]\n";
        outFile << "Address: " << item << "\n";

        // Read Fields
        // Price
        void* pricePtr = *(void**)((uintptr_t)item + OFFSET_ITEM_PRICE);
        if (pricePtr) {
            Il2CppString* str = (Il2CppString*)pricePtr;
            outFile << "Price: " << str->ToString() << "\n";
        }

        // Level
        void* levelPtr = *(void**)((uintptr_t)item + OFFSET_ITEM_LEVEL);
        if (levelPtr) {
            Il2CppString* str = (Il2CppString*)levelPtr;
            outFile << "Level: " << str->ToString() << "\n";
        }
        
        // Tier
        void* tierPtr = *(void**)((uintptr_t)item + OFFSET_ITEM_TIER);
        if (tierPtr) {
            Il2CppString* str = (Il2CppString*)tierPtr;
            outFile << "Tier: " << str->ToString() << "\n";
        }

        outFile << "--------------------------------------------------\n";
    }

    outFile << "End of Dump.\n";
    outFile.close();
    
    LOGI("Dump Complete. Saved to %s", DUMP_FILE_PATH);
    g_hasDumped = true;
}
