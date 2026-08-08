#pragma once
#include <Windows.h>
#include <cstdint>
#include <optional>
#include <string>

namespace igi {
class ProcessMemory final {
public:
    ProcessMemory() = default;
    ~ProcessMemory();
    ProcessMemory(const ProcessMemory&) = delete;
    ProcessMemory& operator=(const ProcessMemory&) = delete;

    bool attach(const wchar_t* executableName);
    void detach() noexcept;
    [[nodiscard]] bool attached() const noexcept;
    [[nodiscard]] DWORD pid() const noexcept { return pid_; }
    [[nodiscard]] std::uintptr_t moduleBase() const noexcept { return moduleBase_; }

    template<class T> std::optional<T> read(std::uintptr_t address) const {
        T value{}; SIZE_T done{};
        if (!handle_ || !ReadProcessMemory(handle_, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &done) || done != sizeof(T)) return std::nullopt;
        return value;
    }
    template<class T> bool write(std::uintptr_t address, const T& value) const {
        SIZE_T done{};
        return handle_ && WriteProcessMemory(handle_, reinterpret_cast<LPVOID>(address), &value, sizeof(T), &done) && done == sizeof(T);
    }
private:
    HANDLE handle_{nullptr};
    DWORD pid_{0};
    std::uintptr_t moduleBase_{0};
};
}
