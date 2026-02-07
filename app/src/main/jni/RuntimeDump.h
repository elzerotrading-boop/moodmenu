#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "Constants.h"

// RuntimeDump.h
struct Il2CppString {
    void* klass;
    void* monitor;
    int32_t length;
    char16_t chars[1];
    
    std::string ToString() {
        if (length <= 0) return "";
        std::string s;
        for (int i = 0; i < length; i++) {
            s += (char)chars[i]; // Basic truncation for ASCII
        }
        return s;
    }
};

struct Il2CppList {
    void* klass;
    void* monitor;
    void* items; // Array pointer
    int32_t size;
};

struct Il2CppArray {
    void* klass;
    void* monitor;
    void* bounds;
    void* max_length;
    void* vector[1]; // Start of data
};

// Forward declaration
class RuntimeDump {
public:
    static void Initialize();
    static void PerformDump(void* attackComponent);
    
    // ✅ نقلناها من private لـ public
    static bool g_hasDumped;
    
private:
    // أي متغيرات خاصة تانية هنا
};
