#include <GameTrainer.hpp>
#include <Offsets.hpp>
#include <Windows.h>
#include <algorithm>
#include <cmath>

namespace igi {
GameTrainer::GameTrainer() = default;
GameTrainer::~GameTrainer() { stop(); }

void GameTrainer::start() {
    if (running_.exchange(true)) return;
    worker_ = std::jthread([this] { run(); });
}

void GameTrainer::stop() {
    running_ = false;
    if (worker_.joinable()) worker_.join();
    restoreRootScale();
}

void GameTrainer::toggleInvincible() {
    std::scoped_lock lock(mutex_);
    view_.invincible.enabled = !view_.invincible.enabled;
    view_.invincible.led = view_.invincible.enabled ? LedState::Waiting : LedState::Off;
}

void GameTrainer::toggleMagazine() {
    std::scoped_lock lock(mutex_);
    view_.magazine.enabled = !view_.magazine.enabled;
    view_.magazine.led = view_.magazine.enabled ? LedState::Waiting : LedState::Off;
}

void GameTrainer::toggleInventory() {
    std::scoped_lock lock(mutex_);
    view_.inventory.enabled = !view_.inventory.enabled;
    view_.inventory.led = view_.inventory.enabled ? LedState::Waiting : LedState::Off;
}

void GameTrainer::cycleMovement() {
    std::scoped_lock lock(mutex_);
    movementLevel_ = movementLevel_ % 5 + 1;
    view_.movementLevel = movementLevel_;
    view_.movement.enabled = movementLevel_ > 1;
    view_.movement.led = view_.movement.enabled ? LedState::Waiting : LedState::Off;
}

void GameTrainer::disableAll() {
    std::scoped_lock lock(mutex_);
    view_.invincible.enabled = false;
    view_.magazine.enabled = false;
    view_.inventory.enabled = false;
    view_.invincible.led = LedState::Off;
    view_.magazine.led = LedState::Off;
    view_.inventory.led = LedState::Off;
    movementLevel_ = 1;
    view_.movementLevel = 1;
    view_.movement.enabled = false;
    view_.movement.led = LedState::Off;
    view_.fallProtection = false;
    airBoostActive_ = false;
    fallProtectionActive_ = false;
    diveActive_ = false;
    modifiedThisJump_ = false;
}

UiSnapshot GameTrainer::snapshot() const {
    std::scoped_lock lock(mutex_);
    return view_;
}

bool GameTrainer::validPointer(std::uint32_t value) const {
    return value >= 0x10000u && value < 0x80000000u;
}

std::optional<std::uintptr_t> GameTrainer::resolvePlayer() {
    auto p0 = memory_.read<std::uint32_t>(memory_.moduleBase() + offsets::rootStatic);
    if (!p0 || !validPointer(*p0)) return {};
    auto p1 = memory_.read<std::uint32_t>(*p0 + offsets::playerStep1);
    if (!p1 || !validPointer(*p1)) return {};
    auto p2 = memory_.read<std::uint32_t>(*p1 + offsets::playerStep2);
    if (!p2 || !validPointer(*p2)) return {};
    auto p3 = memory_.read<std::uint32_t>(*p2 + offsets::playerStep3);
    if (!p3 || !validPointer(*p3)) return {};
    return static_cast<std::uintptr_t>(*p3);
}

std::optional<std::uintptr_t> GameTrainer::resolveMagazine() {
    auto m0 = memory_.read<std::uint32_t>(memory_.moduleBase() + offsets::magazineStatic);
    if (!m0 || !validPointer(*m0)) return {};
    auto m1 = memory_.read<std::uint32_t>(*m0 + offsets::magazineStep1);
    if (!m1 || !validPointer(*m1)) return {};
    auto m2 = memory_.read<std::uint32_t>(*m1 + offsets::magazineStep2);
    if (!m2 || !validPointer(*m2)) return {};
    return static_cast<std::uintptr_t>(*m2) + offsets::magazineFinal;
}

void GameTrainer::restoreRootScale() {
    if (memory_.attached() && originalRootKnown_) {
        memory_.write<float>(memory_.moduleBase() + offsets::rootMotionScale, originalRootScale_);
    }
}

void GameTrainer::updateHotkeys() {
    if (GetAsyncKeyState(VK_F1) & 1) toggleInvincible();
    if (GetAsyncKeyState(VK_F2) & 1) toggleMagazine();
    if (GetAsyncKeyState(VK_F3) & 1) toggleInventory();
    if (GetAsyncKeyState(VK_F4) & 1) cycleMovement();
    if (GetAsyncKeyState(VK_F12) & 1) disableAll();
}

void GameTrainer::tick() {
    updateHotkeys();

    if (!memory_.attached()) {
        memory_.detach();
        originalRootKnown_ = false;
        lastPlayer_ = 0;
        if (!memory_.attach(offsets::processName)) {
            std::scoped_lock lock(mutex_);
            view_.attached = false;
            view_.pid = 0;
            view_.status = L"Waiting for igi.exe";
            return;
        }
    }

    const auto rootAddress = memory_.moduleBase() + offsets::rootMotionScale;
    if (!originalRootKnown_) {
        auto value = memory_.read<float>(rootAddress);
        if (value && std::isfinite(*value) && *value > 0.0f && *value < 100.0f) {
            originalRootScale_ = *value;
            originalRootKnown_ = true;
        }
    }

    int level;
    bool invincible;
    bool magazine;
    bool inventory;
    {
        std::scoped_lock lock(mutex_);
        level = movementLevel_;
        invincible = view_.invincible.enabled;
        magazine = view_.magazine.enabled;
        inventory = view_.inventory.enabled;
    }

    if (originalRootKnown_) {
        const float target = originalRootScale_ * static_cast<float>(level);
        memory_.write<float>(rootAddress, target);
        std::scoped_lock lock(mutex_);
        view_.rootScale = target;
    }

    auto player = resolvePlayer();
    if (!player) {
        lastPlayer_ = 0;
        previousGroundedKnown_ = false;
        airBoostActive_ = false;
        fallProtectionActive_ = false;
        diveActive_ = false;
        modifiedThisJump_ = false;
        std::scoped_lock lock(mutex_);
        view_.attached = true;
        view_.pid = memory_.pid();
        view_.status = L"Game detected - start or resume mission";
        view_.movement.led = view_.movement.enabled ? LedState::Waiting : LedState::Off;
        return;
    }

    if (*player != lastPlayer_) {
        lastPlayer_ = *player;
        previousGroundedKnown_ = false;
        airBoostActive_ = false;
        fallProtectionActive_ = false;
        diveActive_ = false;
        modifiedThisJump_ = false;
    }

    {
        std::scoped_lock lock(mutex_);
        view_.attached = true;
        view_.pid = memory_.pid();
        view_.status = L"Offline mission active";
    }

    auto damage = memory_.read<float>(*player + offsets::accumulatedDamage);
    auto capacity = memory_.read<float>(*player + offsets::damageCapacity);
    const bool healthValid = damage && capacity && std::isfinite(*damage) && std::isfinite(*capacity) && *capacity > 0.0f;

    if (healthValid) {
        std::scoped_lock lock(mutex_);
        view_.healthPercent = std::clamp((*capacity - *damage) / *capacity * 100.0f, 0.0f, 100.0f);
    }

    if (invincible) {
        bool wrote = false;
        if (healthValid && *damage != 0.0f) {
            wrote = memory_.write<float>(*player + offsets::accumulatedDamage, 0.0f);
        }
        std::scoped_lock lock(mutex_);
        view_.invincible.led = healthValid ? LedState::Active : LedState::Waiting;
        if (wrote) ++view_.invincible.writes;
    }

    if (magazine) {
        auto address = resolveMagazine();
        bool valid = false;
        bool wrote = false;
        if (address) {
            auto current = memory_.read<std::int32_t>(*address);
            valid = current && *current >= offsets::magazineMin && *current <= offsets::magazineMax;
            if (valid && *current != offsets::magazineTarget) {
                wrote = memory_.write<std::int32_t>(*address, offsets::magazineTarget);
            }
        }
        std::scoped_lock lock(mutex_);
        view_.magazine.led = valid ? LedState::Active : LedState::Waiting;
        if (wrote) ++view_.magazine.writes;
    }

    if (inventory) {
        auto count = memory_.read<std::int32_t>(*player + offsets::inventoryTable);
        const bool valid = count && *count >= 0 && *count <= offsets::maxSafeRecords;
        std::uint64_t writes = 0;
        if (valid) {
            for (std::int32_t index = 0; index < *count; ++index) {
                const auto record = *player + offsets::inventoryFirst + static_cast<std::uintptr_t>(index) * offsets::inventoryRecordSize;
                auto current = memory_.read<std::int32_t>(record + 4);
                auto maximum = memory_.read<std::int32_t>(record + 8);
                if (!current || !maximum || *current < 0) continue;
                const auto target = *maximum > 0 ? *maximum : offsets::unclampedTarget;
                if (target >= 0 && target <= 100000 && *current != target && memory_.write<std::int32_t>(record + 4, target)) {
                    ++writes;
                }
            }
        }
        std::scoped_lock lock(mutex_);
        view_.inventory.led = valid ? LedState::Active : LedState::Waiting;
        view_.inventory.writes += writes;
    }

    auto flags = memory_.read<std::uint32_t>(*player + offsets::stateFlags);
    auto speedX = memory_.read<float>(*player + offsets::speedX);
    auto speedY = memory_.read<float>(*player + offsets::speedY);
    auto speedZ = memory_.read<float>(*player + offsets::speedZ);
    if (!flags || !speedX || !speedY || !speedZ || !std::isfinite(*speedX) || !std::isfinite(*speedY) || !std::isfinite(*speedZ)) {
        previousGroundedKnown_ = false;
        return;
    }

    const bool grounded = (*flags & offsets::groundFlag) != 0;
    const bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    const auto now = std::chrono::steady_clock::now();

    if (grounded) {
        if (previousGroundedKnown_ && !previousGrounded_ && level > 1) {
            memory_.write<float>(*player + offsets::speedZ, 0.0f);
            if (invincible || fallProtectionActive_ || diveActive_) {
                memory_.write<float>(*player + offsets::accumulatedDamage, 0.0f);
            }
        }
        modifiedThisJump_ = false;
        airBoostActive_ = false;
        fallProtectionActive_ = false;
        diveActive_ = false;
    }

    const bool takeoff = previousGroundedKnown_ && previousGrounded_ && !grounded;
    if (takeoff && !modifiedThisJump_ && *speedZ >= offsets::minTakeoffSpeed && *speedZ <= offsets::maxTakeoffSpeed) {
        memory_.write<float>(*player + offsets::speedZ, offsets::normalJumpImpulse * static_cast<float>(level));
        const float horizontalSpeed = std::hypot(*speedX, *speedY);
        if (level > 1 && horizontalSpeed >= offsets::minHorizontalSpeed) {
            const float boostedSpeed = std::max(horizontalSpeed * static_cast<float>(level), offsets::minAirBoostSpeed);
            airDirectionX_ = *speedX / horizontalSpeed;
            airDirectionY_ = *speedY / horizontalSpeed;
            airTargetSpeed_ = std::min(boostedSpeed, offsets::maxHorizontalSpeed);
            airBoostActive_ = true;
            airBoostEnd_ = now + std::chrono::milliseconds(offsets::airBoostMs);
        }
        modifiedThisJump_ = true;
        std::scoped_lock lock(mutex_);
        ++view_.movement.writes;
    }

    if (airBoostActive_ && !grounded) {
        if (now < airBoostEnd_) {
            memory_.write<float>(*player + offsets::speedX, airDirectionX_ * airTargetSpeed_);
            memory_.write<float>(*player + offsets::speedY, airDirectionY_ * airTargetSpeed_);
        } else {
            airBoostActive_ = false;
        }
    }

    bool divingThisCycle = false;
    if (level > 1 && !grounded && shiftPressed) {
        memory_.write<float>(*player + offsets::speedZ, offsets::diveDownSpeed);
        if (!fallProtectionActive_) fallProtectionStart_ = now;
        fallProtectionActive_ = true;
        diveActive_ = true;
        divingThisCycle = true;
    }

    if (level > 1 && !grounded && !divingThisCycle && *speedZ < offsets::fallTrigger) {
        if (!fallProtectionActive_) {
            fallProtectionActive_ = true;
            fallProtectionStart_ = now;
        }
        const float elapsedMs = std::chrono::duration<float, std::milli>(now - fallProtectionStart_).count();
        const float interpolation = std::clamp(elapsedMs / static_cast<float>(offsets::softLandingRampMs), 0.0f, 1.0f);
        const float target = offsets::safeDownwardSpeed + (offsets::softLandingTargetSpeed - offsets::safeDownwardSpeed) * interpolation;
        memory_.write<float>(*player + offsets::speedZ, target);
        if (invincible) {
            memory_.write<float>(*player + offsets::accumulatedDamage, 0.0f);
        }
    }

    {
        std::scoped_lock lock(mutex_);
        view_.fallProtection = level > 1 && (fallProtectionActive_ || diveActive_);
        view_.movement.led = level > 1 ? LedState::Active : LedState::Off;
    }

    previousGroundedKnown_ = true;
    previousGrounded_ = grounded;
}

void GameTrainer::run() {
    while (running_) {
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(offsets::pollIntervalMs));
    }
    restoreRootScale();
    memory_.detach();
}
}
