#pragma once
#include <cstdint>

namespace igi::offsets {
inline constexpr wchar_t processName[] = L"igi.exe";
inline constexpr std::uintptr_t rootStatic = 0x16E210;
inline constexpr std::uintptr_t playerStep1 = 0x08;
inline constexpr std::uintptr_t playerStep2 = 0x7CC;
inline constexpr std::uintptr_t playerStep3 = 0x14;
inline constexpr std::uintptr_t accumulatedDamage = 0x254;
inline constexpr std::uintptr_t damageCapacity = 0x258;
inline constexpr std::uintptr_t inventoryTable = 0x340;
inline constexpr std::uintptr_t inventoryFirst = 0x344;
inline constexpr std::uintptr_t inventoryRecordSize = 0x0C;
inline constexpr std::int32_t maxSafeRecords = 64;
inline constexpr std::int32_t unclampedTarget = 100;
inline constexpr std::uintptr_t magazineStatic = 0x671890;
inline constexpr std::uintptr_t magazineStep1 = 0x00;
inline constexpr std::uintptr_t magazineStep2 = 0x4C4;
inline constexpr std::uintptr_t magazineFinal = 0x144;
inline constexpr std::int32_t magazineTarget = 99;
inline constexpr std::int32_t magazineMin = 0;
inline constexpr std::int32_t magazineMax = 500;
inline constexpr auto pollIntervalMs = 25u;
}
