
#pragma once

// Constants.h
// Offsets derived from dump.cs and script.json for Version: [Pending Verification]
// Target Architecture: x86_64 (Nox Emulator)

// Hook Target
// AttackComponent$$OnTriggerEnter
// Address: 68959944 (Decimal) -> 0x41C2AC8 (Hex)
#define OFFSET_ATTACKCOMPONENT_ONTRIGGERENTER 0x41C2AC8

// AttackComponent Offsets
// public List<BadNerdItem> itemList; // 0x138
#define OFFSET_ATTACKCOMPONENT_ITEMLIST 0x138

// Standard Il2Cpp List<T> Offsets (64-bit Unity)
#define OFFSET_LIST_ITEMS 0x10   // Pointer to Array
#define OFFSET_LIST_SIZE  0x18   // int32_t size

// Standard Il2Cpp Array Offsets
#define OFFSET_ARRAY_DATA 0x20   // Start of data[]

// BadNerdItem Offsets (dump.cs)
/*
    public string _price; // 0x50
    public string _level; // 0x70
    public string _tier; // 0x78
    public string description; // 0x90
    public float carryingWeight; // 0xA8
    public string folderName; // 0x20 in InventoryItemData wrapper?
    // Note: Checking BadNerdItem directly first.
*/
#define OFFSET_ITEM_PRICE       0x50
#define OFFSET_ITEM_LEVEL       0x70
#define OFFSET_ITEM_TIER        0x78
#define OFFSET_ITEM_DESC        0x90
#define OFFSET_ITEM_WEIGHT      0xA8

// System.String Offsets
#define OFFSET_STRING_LENGTH 0x10
#define OFFSET_STRING_CHARS  0x14

// Output Path
#define DUMP_FILE_PATH "/sdcard/items_runtime_dump.txt"
