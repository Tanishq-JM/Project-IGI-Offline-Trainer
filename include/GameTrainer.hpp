#pragma once
#include "ProcessMemory.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace igi {
enum class LedState { Off, Waiting, Active, Error };

struct FeatureView {
    bool enabled{};
    LedState led{LedState::Off};
    std::uint64_t writes{};
};

struct UiSnapshot {
    bool attached{};
    DWORD pid{};
    std::wstring status{L"Waiting for igi.exe"};
    float healthPercent{};
    FeatureView invincible{}, magazine{}, inventory{}, movement{};
    int movementLevel{1};
    float rootScale{};
    bool fallProtection{};
};

class GameTrainer {
public:
    GameTrainer();
    ~GameTrainer();
    void start();
    void stop();
    void toggleInvincible();
    void toggleMagazine();
    void toggleInventory();
    void cycleMovement();
    void disableAll();
    UiSnapshot snapshot() const;

private:
    bool validPointer(std::uint32_t value) const;
    std::optional<std::uintptr_t> resolvePlayer();
    std::optional<std::uintptr_t> resolveMagazine();
    void updateHotkeys();
    void tick();
    void run();
    void restoreRootScale();

    mutable std::mutex mutex_;
    ProcessMemory memory_;
    UiSnapshot view_;
    std::atomic_bool running_{};
    std::jthread worker_;
    std::uintptr_t lastPlayer_{};
    bool previousGroundedKnown_{};
    bool previousGrounded_{};
    bool modifiedThisJump_{};
    bool airBoostActive_{};
    bool fallProtectionActive_{};
    bool diveActive_{};
    float airDirectionX_{};
    float airDirectionY_{};
    float airTargetSpeed_{};
    std::chrono::steady_clock::time_point airBoostEnd_{};
    std::chrono::steady_clock::time_point fallProtectionStart_{};
    bool originalRootKnown_{};
    float originalRootScale_{};
    int movementLevel_{1};
};
}
