#pragma once
#include <Windows.h>
#include <cstdint>
#include <optional>
#include <type_traits>
namespace igi {
class ProcessMemory {
public:
    ~ProcessMemory();
    bool attach(const wchar_t* name);
    void detach() noexcept;
    bool attached() const noexcept;
    DWORD pid() const noexcept { return pid_; }
    std::uintptr_t moduleBase() const noexcept { return moduleBase_; }
    template<class T> std::optional<T> read(std::uintptr_t address) const {
        static_assert(std::is_trivially_copyable_v<T>);
        if (!handle_ || !address) return std::nullopt;
        T value{}; SIZE_T done{};
        if (!ReadProcessMemory(handle_, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &done) || done != sizeof(T)) return std::nullopt;
        return value;
    }
    template<class T> bool write(std::uintptr_t address, const T& value) const {
        static_assert(std::is_trivially_copyable_v<T>);
        if (!handle_ || !address) return false;
        SIZE_T done{};
        return WriteProcessMemory(handle_, reinterpret_cast<LPVOID>(address), &value, sizeof(T), &done) && done == sizeof(T);
    }
private:
    HANDLE handle_{};
    DWORD pid_{};
    std::uintptr_t moduleBase_{};
};
}
