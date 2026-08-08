#include "App.hpp"

#include <dwmapi.h>
#include <windowsx.h>

#include <cwchar>
#include <string>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dwmapi.lib")

namespace igi {
namespace {
constexpr wchar_t kWindowClass[] = L"TJM.IGITrainer.Compact.Window";
constexpr int kClientWidth = 470;
constexpr int kClientHeight = 520;
constexpr UINT kIconId = 101;

D2D1_COLOR_F rgb(UINT32 value, float alpha = 1.0f) {
    return D2D1::ColorF(value, alpha);
}

bool missionActive(const UiSnapshot& view) {
    return view.attached &&
           view.status.find(L"mission active") != std::wstring::npos;
}
}

int App::run(HINSTANCE instance, int show) {
    instance_ = instance;
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;

    if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            factory_.ReleaseAndGetAddressOf()))) {
        CoUninitialize();
        return 1;
    }

    if (FAILED(DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(writeFactory_.ReleaseAndGetAddressOf())))) {
        CoUninitialize();
        return 1;
    }

    HICON iconLarge = static_cast<HICON>(LoadImageW(
        instance_, MAKEINTRESOURCEW(kIconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
    HICON iconSmall = static_cast<HICON>(LoadImageW(
        instance_, MAKEINTRESOURCEW(kIconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));

    if (!iconLarge) iconLarge = LoadIconW(nullptr, IDI_APPLICATION);
    if (!iconSmall) iconSmall = LoadIconW(nullptr, IDI_APPLICATION);

    WNDCLASSEXW wc{sizeof(wc)};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance_;
    wc.hIcon = iconLarge;
    wc.hIconSm = iconSmall;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kWindowClass;

    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        CoUninitialize();
        return 1;
    }

    RECT rect{0, 0, kClientWidth, kClientHeight};
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&rect, style, FALSE);
    const int windowWidth = rect.right - rect.left;
    const int windowHeight = rect.bottom - rect.top;

    hwnd_ = CreateWindowExW(
        0,
        kWindowClass,
        L"IGI Offline Trainer",
        style,
        (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        instance_,
        this);

    if (!hwnd_) {
        CoUninitialize();
        return 1;
    }

    SendMessageW(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(iconLarge));
    SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(iconSmall));

    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd_, 20, &dark, sizeof(dark));
    const DWM_WINDOW_CORNER_PREFERENCE corners = DWMWCP_ROUND;
    DwmSetWindowAttribute(
        hwnd_, DWMWA_WINDOW_CORNER_PREFERENCE, &corners, sizeof(corners));

    ShowWindow(hwnd_, show);
    UpdateWindow(hwnd_);
    trainer_.start();
    SetTimer(hwnd_, 1, 33, nullptr);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    trainer_.stop();
    discardResources();
    CoUninitialize();
    return static_cast<int>(message.wParam);
}

LRESULT CALLBACK App::WindowProc(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    App* app = nullptr;
    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        app = static_cast<App*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        app = reinterpret_cast<App*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }
    return app ? app->handle(window, message, wParam, lParam)
               : DefWindowProcW(window, message, wParam, lParam);
}

LRESULT App::handle(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        BeginPaint(window, &ps);
        paint();
        EndPaint(window, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_SIZE:
        if (target_) {
            target_->Resize(D2D1::SizeU(
                static_cast<UINT32>(LOWORD(lParam)),
                static_cast<UINT32>(HIWORD(lParam))));
        }
        return 0;
    case WM_TIMER:
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
            POINT p{};
            GetCursorPos(&p);
            ScreenToClient(window, &p);
            const bool clickable =
                (p.x >= 18 && p.x <= 452) &&
                ((p.y >= 132 && p.y <= 222) ||
                 (p.y >= 232 && p.y <= 322) ||
                 (p.y >= 332 && p.y <= 422) ||
                 (p.y >= 466 && p.y <= 504));
            SetCursor(LoadCursorW(nullptr, clickable ? IDC_HAND : IDC_ARROW));
            return TRUE;
        }
        break;
    case WM_LBUTTONUP: {
        const float x = static_cast<float>(GET_X_LPARAM(lParam));
        const float y = static_cast<float>(GET_Y_LPARAM(lParam));
        if (x >= 18.0f && x <= 452.0f) {
            if (y >= 132.0f && y <= 222.0f) trainer_.toggleInvincible();
            else if (y >= 232.0f && y <= 322.0f) trainer_.toggleMagazine();
            else if (y >= 332.0f && y <= 422.0f) trainer_.toggleInventory();
            else if (y >= 466.0f && y <= 504.0f && x <= 116.0f) showAbout();
            else if (y >= 466.0f && y <= 504.0f && x >= 354.0f) DestroyWindow(window);
        }
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        KillTimer(window, 1);
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

bool App::createResources() {
    if (target_) return true;

    RECT rect{};
    GetClientRect(hwnd_, &rect);
    const auto size = D2D1::SizeU(
        static_cast<UINT32>(rect.right),
        static_cast<UINT32>(rect.bottom));

    if (FAILED(factory_->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_HARDWARE,
                D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)),
            D2D1::HwndRenderTargetProperties(
                hwnd_, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
            target_.ReleaseAndGetAddressOf()))) {
        return false;
    }

    auto brush = [this](
        D2D1_COLOR_F value,
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>& destination) {
        return target_->CreateSolidColorBrush(
            value, destination.ReleaseAndGetAddressOf());
    };

    if (FAILED(brush(rgb(0x090C12), bg_)) ||
        FAILED(brush(rgb(0x121821), panel_)) ||
        FAILED(brush(rgb(0x273245), stroke_)) ||
        FAILED(brush(rgb(0xF5F7FB), white_)) ||
        FAILED(brush(rgb(0x8C98AA), muted_)) ||
        FAILED(brush(rgb(0x777EFF), accent_)) ||
        FAILED(brush(rgb(0x39D98A), green_)) ||
        FAILED(brush(rgb(0xF6C85F), amber_)) ||
        FAILED(brush(rgb(0xFF667C), red_)) ||
        FAILED(brush(rgb(0x48566D), off_))) {
        discardResources();
        return false;
    }

    writeFactory_->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        12.0f, L"en-us", normal_.ReleaseAndGetAddressOf());
    writeFactory_->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        13.0f, L"en-us", semibold_.ReleaseAndGetAddressOf());
    writeFactory_->CreateTextFormat(
        L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_BOLD,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        20.0f, L"en-us", title_.ReleaseAndGetAddressOf());

    return normal_ && semibold_ && title_;
}

void App::discardResources() {
    target_.Reset();
    bg_.Reset(); panel_.Reset(); stroke_.Reset(); white_.Reset(); muted_.Reset();
    accent_.Reset(); green_.Reset(); amber_.Reset(); red_.Reset(); off_.Reset();
}

void App::rounded(
    float x, float y, float width, float height, float radius, ID2D1Brush* brush) {
    target_->FillRoundedRectangle(
        D2D1::RoundedRect(D2D1::RectF(x, y, x + width, y + height), radius, radius),
        brush);
}

void App::text(
    const wchar_t* value,
    float x, float y, float width, float height,
    float size, ID2D1Brush* brush,
    DWRITE_TEXT_ALIGNMENT alignment) {
    if (!value || !target_ || !writeFactory_ || !brush) return;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
    const auto weight = size >= 18.0f
        ? DWRITE_FONT_WEIGHT_BOLD
        : (size >= 12.5f ? DWRITE_FONT_WEIGHT_SEMI_BOLD
                         : DWRITE_FONT_WEIGHT_NORMAL);

    if (FAILED(writeFactory_->CreateTextFormat(
            L"Segoe UI", nullptr, weight,
            DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            size, L"en-us", format.ReleaseAndGetAddressOf()))) {
        return;
    }

    format->SetTextAlignment(alignment);
    format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

    target_->DrawTextW(
        value,
        static_cast<UINT32>(std::wcslen(value)),
        format.Get(),
        D2D1::RectF(x, y, x + width, y + height),
        brush,
        D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void App::led(float x, float y, LedState state) {
    ID2D1Brush* brush = off_.Get();
    if (state == LedState::Active) brush = green_.Get();
    else if (state == LedState::Waiting) brush = amber_.Get();
    else if (state == LedState::Error) brush = red_.Get();

    target_->FillEllipse(
        D2D1::Ellipse(D2D1::Point2F(x, y), 5.0f, 5.0f), brush);
    target_->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(x, y), 8.0f, 8.0f), brush, 1.25f);
}

void App::card(
    float y,
    const wchar_t* name,
    const wchar_t* subtitle,
    const wchar_t* key,
    const FeatureView& feature,
    int) {
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> fill;
    target_->CreateSolidColorBrush(
        feature.enabled ? rgb(0x172033) : rgb(0x111720),
        fill.ReleaseAndGetAddressOf());

    rounded(18.0f, y, 434.0f, 90.0f, 14.0f, fill.Get());
    target_->DrawRoundedRectangle(
        D2D1::RoundedRect(
            D2D1::RectF(18.0f, y, 452.0f, y + 90.0f), 14.0f, 14.0f),
        feature.enabled ? accent_.Get() : stroke_.Get(),
        feature.enabled ? 1.5f : 1.0f);

    led(40.0f, y + 31.0f, feature.led);
    text(name, 58.0f, y + 12.0f, 228.0f, 26.0f, 13.5f, white_.Get());
    text(subtitle, 58.0f, y + 38.0f, 260.0f, 24.0f, 9.5f, muted_.Get());

    rounded(322.0f, y + 15.0f, 38.0f, 23.0f, 7.0f, stroke_.Get());
    text(key, 322.0f, y + 15.0f, 38.0f, 23.0f, 9.0f, muted_.Get(),
         DWRITE_TEXT_ALIGNMENT_CENTER);

    text(feature.enabled ? L"ON" : L"OFF",
         374.0f, y + 14.0f, 54.0f, 24.0f, 10.0f,
         feature.enabled ? green_.Get() : muted_.Get(),
         DWRITE_TEXT_ALIGNMENT_TRAILING);

    const std::wstring count = std::to_wstring(feature.writes) + L" writes";
    text(count.c_str(), 302.0f, y + 51.0f, 126.0f, 22.0f, 8.5f, muted_.Get(),
         DWRITE_TEXT_ALIGNMENT_TRAILING);
}

void App::paint() {
    if (!createResources()) return;

    const UiSnapshot view = trainer_.snapshot();
    const bool activeMission = missionActive(view);
    const LedState state = activeMission
        ? LedState::Active
        : (view.attached ? LedState::Waiting : LedState::Off);

    const wchar_t* statusTitle = activeMission
        ? L"MISSION ACTIVE"
        : (view.attached ? L"GAME DETECTED" : L"GAME OFFLINE");
    const wchar_t* statusDetail = activeMission
        ? L"Connected to offline mission"
        : (view.attached ? L"Start or resume a mission" : L"Waiting for igi.exe");

    target_->BeginDraw();
    target_->Clear(rgb(0x090C12));
    target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

    HICON icon = static_cast<HICON>(LoadImageW(
        instance_, MAKEINTRESOURCEW(kIconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
    if (icon) {
        HDC dc = GetDC(hwnd_);
        DrawIconEx(dc, 19, 18, icon, 32, 32, 0, nullptr, DI_NORMAL);
        ReleaseDC(hwnd_, dc);
    }

    text(L"IGI OFFLINE TRAINER", 62.0f, 12.0f, 280.0f, 32.0f,
         18.0f, white_.Get());
    text(L"Compact control panel", 62.0f, 39.0f, 220.0f, 20.0f,
         9.5f, muted_.Get());
    text(L"v1.0.0", 380.0f, 18.0f, 68.0f, 24.0f,
         9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> statusFill;
    target_->CreateSolidColorBrush(
        activeMission ? rgb(0x10251C)
        : (view.attached ? rgb(0x29230F) : rgb(0x121821)),
        statusFill.ReleaseAndGetAddressOf());

    rounded(18.0f, 72.0f, 434.0f, 46.0f, 12.0f, statusFill.Get());
    target_->DrawRoundedRectangle(
        D2D1::RoundedRect(D2D1::RectF(18.0f, 72.0f, 452.0f, 118.0f), 12.0f, 12.0f),
        activeMission ? green_.Get() : (view.attached ? amber_.Get() : stroke_.Get()),
        1.0f);
    led(39.0f, 95.0f, state);
    text(statusTitle, 57.0f, 77.0f, 190.0f, 20.0f, 11.0f,
         activeMission ? green_.Get() : (view.attached ? amber_.Get() : muted_.Get()));
    text(statusDetail, 57.0f, 96.0f, 230.0f, 17.0f, 8.5f, muted_.Get());

    std::wstring right = view.attached
        ? (L"PID " + std::to_wstring(view.pid))
        : L"NOT RUNNING";
    text(right.c_str(), 303.0f, 79.0f, 125.0f, 18.0f, 8.5f,
         muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    wchar_t health[32]{};
    swprintf_s(health, L"HP %.1f%%", static_cast<double>(view.healthPercent));
    text(health, 303.0f, 97.0f, 125.0f, 17.0f, 8.5f,
         activeMission ? green_.Get() : muted_.Get(),
         DWRITE_TEXT_ALIGNMENT_TRAILING);

    card(132.0f, L"INVINCIBLE", L"Keeps accumulated damage at zero",
         L"F1", view.invincible, 1);
    card(232.0f, L"MAGAZINE AUTO-FILL", L"Maintains the active magazine",
         L"F2", view.magazine, 2);
    card(332.0f, L"INVENTORY AUTO-MAX", L"Maintains valid item quantities",
         L"F3", view.inventory, 3);

    target_->DrawLine(
        D2D1::Point2F(18.0f, 448.0f), D2D1::Point2F(452.0f, 448.0f),
        stroke_.Get(), 1.0f);

    rounded(18.0f, 466.0f, 98.0f, 38.0f, 10.0f, panel_.Get());
    target_->DrawRoundedRectangle(
        D2D1::RoundedRect(D2D1::RectF(18.0f, 466.0f, 116.0f, 504.0f), 10.0f, 10.0f),
        stroke_.Get(), 1.0f);
    text(L"ABOUT", 18.0f, 466.0f, 98.0f, 38.0f, 9.5f,
         white_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

    text(L"F12  DISABLE ALL", 145.0f, 466.0f, 180.0f, 38.0f,
         8.5f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

    rounded(354.0f, 466.0f, 98.0f, 38.0f, 10.0f, panel_.Get());
    target_->DrawRoundedRectangle(
        D2D1::RoundedRect(D2D1::RectF(354.0f, 466.0f, 452.0f, 504.0f), 10.0f, 10.0f),
        stroke_.Get(), 1.0f);
    text(L"EXIT", 354.0f, 466.0f, 98.0f, 38.0f, 9.5f,
         white_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

    const HRESULT result = target_->EndDraw();
    if (result == D2DERR_RECREATE_TARGET) discardResources();
}

void App::showAbout() {
    MessageBoxW(
        hwnd_,
        L"IGI Offline Trainer v1.0.0\n\n"
        L"Compact Direct2D control panel for authorized offline "
        L"single-player experimentation.\n\n"
        L"Green: offline mission active\n"
        L"Yellow: game running, mission unavailable\n"
        L"Gray: game not running\n\n"
        L"F1 Invincible\nF2 Magazine Auto-Fill\n"
        L"F3 Inventory Auto-Max\nF12 Disable All\n\n"
        L"Unofficial fan-made utility. No game files included.\n"
        L"Designed and developed by TJM.",
        L"About IGI Offline Trainer",
        MB_OK | MB_ICONINFORMATION);
}

} // namespace igi
