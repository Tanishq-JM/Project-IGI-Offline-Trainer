#pragma once
#include "ProcessMemory.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

namespace igi {
enum class LedState { Off, Active, Waiting, Error };
struct FeatureView { bool enabled{}; LedState led{LedState::Off}; std::uint64_t writes{}; };
struct UiSnapshot {
    bool attached{}; DWORD pid{}; float healthPercent{};
    FeatureView invincible, magazine, inventory;
    std::wstring status{L"Waiting for igi.exe"};
};

class GameTrainer final {
public:
    GameTrainer(); ~GameTrainer();
    GameTrainer(const GameTrainer&) = delete;
    GameTrainer& operator=(const GameTrainer&) = delete;
    void start(); void stop();
    void toggleInvincible(); void toggleMagazine(); void toggleInventory();
    [[nodiscard]] UiSnapshot snapshot() const;
private:
    void run();
    bool validPointer(std::uint32_t p) const;
    std::optional<std::uintptr_t> resolvePlayer();
    std::optional<std::uintptr_t> resolveMagazine();
    void tick(); void updateHotkeys();
    mutable std::mutex mutex_;
    ProcessMemory memory_;
    UiSnapshot view_;
    std::atomic_bool running_{false};
    std::jthread worker_;
};
}
