#pragma once
#include <cstddef>
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

inline constexpr std::uintptr_t stateFlags = 0x2F4;
inline constexpr std::uintptr_t speedX = 0x664;
inline constexpr std::uintptr_t speedY = 0x668;
inline constexpr std::uintptr_t speedZ = 0x66C;
inline constexpr std::uint32_t groundFlag = 0x04;
inline constexpr std::uintptr_t rootMotionScale = 0x16E1E8;

inline constexpr float normalJumpImpulse = 1024.0f;
inline constexpr float minTakeoffSpeed = 700.0f;
inline constexpr float maxTakeoffSpeed = 1100.0f;
inline constexpr float minHorizontalSpeed = 5.0f;
inline constexpr float minAirBoostSpeed = 900.0f;
inline constexpr float maxHorizontalSpeed = 6000.0f;
inline constexpr int airBoostMs = 300;

inline constexpr float fallTrigger = -400.0f;
inline constexpr float safeDownwardSpeed = -450.0f;
inline constexpr float softLandingTargetSpeed = -80.0f;
inline constexpr int softLandingRampMs = 600;
inline constexpr float diveDownSpeed = -1600.0f;

inline constexpr int pollIntervalMs = 1;
}
