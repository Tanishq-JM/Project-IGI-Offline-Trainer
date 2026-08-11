#include <App.hpp>
#include <algorithm>
#include <dwmapi.h>
#include <windowsx.h>
#include <cwchar>
#include <string>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dwmapi.lib")

namespace igi {
namespace {
constexpr wchar_t kWindowClass[] = L"TJM.IGITrainer.RegalCommand";
constexpr int kClientWidth = 620;
constexpr int kClientHeight = 470;

D2D1_COLOR_F rgb(UINT32 value, float alpha = 1.0f) {
    return D2D1::ColorF(value, alpha);
}

bool inside(float x, float y, float left, float top, float right, float bottom) {
    return x >= left && x <= right && y >= top && y <= bottom;
}
}

int App::run(HINSTANCE instance, int show) {
    instance_ = instance;
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory_.ReleaseAndGetAddressOf()))) return 1;
    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(writeFactory_.ReleaseAndGetAddressOf())))) return 1;

    HICON icon = static_cast<HICON>(LoadImageW(instance_, MAKEINTRESOURCEW(101), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
    if (!icon) icon = LoadIconW(nullptr, IDI_APPLICATION);

    WNDCLASSEXW wc{sizeof(wc)};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance_;
    wc.hIcon = icon;
    wc.hIconSm = icon;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return 1;

    const int x = (GetSystemMetrics(SM_CXSCREEN) - kClientWidth) / 2;
    const int y = (GetSystemMetrics(SM_CYSCREEN) - kClientHeight) / 2;
    hwnd_ = CreateWindowExW(
        WS_EX_APPWINDOW,
        kWindowClass,
        L"IGI Offline Trainer",
        WS_POPUP | WS_MINIMIZEBOX,
        x, y, kClientWidth, kClientHeight,
        nullptr, nullptr, instance_, this);
    if (!hwnd_) return 1;

    SendMessageW(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
    SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
    BOOL dark = FALSE;
    DwmSetWindowAttribute(hwnd_, 20, &dark, sizeof(dark));
    const DWM_WINDOW_CORNER_PREFERENCE corners = DWMWCP_ROUND;
    DwmSetWindowAttribute(hwnd_, DWMWA_WINDOW_CORNER_PREFERENCE, &corners, sizeof(corners));

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

LRESULT CALLBACK App::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    App* app = nullptr;
    if (message == WM_NCCREATE) {
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        app = static_cast<App*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        app = reinterpret_cast<App*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }
    return app ? app->handle(window, message, wParam, lParam) : DefWindowProcW(window, message, wParam, lParam);
}

LRESULT App::handle(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
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
        if (target_) target_->Resize(D2D1::SizeU(static_cast<UINT32>(LOWORD(lParam)), static_cast<UINT32>(HIWORD(lParam))));
        return 0;
    case WM_TIMER:
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_NCHITTEST: {
        const LRESULT hit = DefWindowProcW(window, message, wParam, lParam);
        if (hit != HTCLIENT) return hit;
        POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        ScreenToClient(window, &point);
        if (point.y < 72 && point.x < 520) return HTCAPTION;
        return HTCLIENT;
    }
    case WM_SETCURSOR: {
        if (LOWORD(lParam) == HTCLIENT) {
            POINT point{};
            GetCursorPos(&point);
            ScreenToClient(window, &point);
            const float x = static_cast<float>(point.x);
            const float y = static_cast<float>(point.y);
            const bool clickable =
                inside(x, y, 24, 157, 596, 221) ||
                inside(x, y, 24, 229, 596, 293) ||
                inside(x, y, 24, 301, 596, 365) ||
                inside(x, y, 24, 86, 596, 143) ||
                inside(x, y, 24, 406, 166, 446) ||
                inside(x, y, 530, 18, 566, 54) ||
                inside(x, y, 570, 18, 606, 54);
            SetCursor(LoadCursorW(nullptr, clickable ? IDC_HAND : IDC_ARROW));
            return TRUE;
        }
        break;
    }
    case WM_LBUTTONUP: {
        const float x = static_cast<float>(GET_X_LPARAM(lParam));
        const float y = static_cast<float>(GET_Y_LPARAM(lParam));
        if (inside(x, y, 24, 157, 596, 221)) trainer_.toggleInvincible();
        else if (inside(x, y, 24, 229, 596, 293)) trainer_.toggleMagazine();
        else if (inside(x, y, 24, 301, 596, 365)) trainer_.toggleInventory();
        else if (inside(x, y, 24, 86, 596, 143)) trainer_.cycleMovement();
        else if (inside(x, y, 24, 406, 166, 446)) trainer_.disableAll();
        else if (inside(x, y, 530, 18, 566, 54)) ShowWindow(window, SW_MINIMIZE);
        else if (inside(x, y, 570, 18, 606, 54)) DestroyWindow(window);
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
    if (FAILED(factory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)),
        D2D1::HwndRenderTargetProperties(hwnd_, D2D1::SizeU(static_cast<UINT32>(rect.right), static_cast<UINT32>(rect.bottom)), D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        target_.ReleaseAndGetAddressOf()))) return false;

    auto makeBrush = [this](D2D1_COLOR_F color, Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>& destination) {
        return target_->CreateSolidColorBrush(color, destination.ReleaseAndGetAddressOf());
    };

    const bool ok =
        SUCCEEDED(makeBrush(rgb(0xF4EFE5), bg_)) &&
        SUCCEEDED(makeBrush(rgb(0xFFFCF5), panel_)) &&
        SUCCEEDED(makeBrush(rgb(0xD8C9AB), stroke_)) &&
        SUCCEEDED(makeBrush(rgb(0x17263C), white_)) &&
        SUCCEEDED(makeBrush(rgb(0x615B52), muted_)) &&
        SUCCEEDED(makeBrush(rgb(0x8C2340), accent_)) &&
        SUCCEEDED(makeBrush(rgb(0x16745A), green_)) &&
        SUCCEEDED(makeBrush(rgb(0xA26B18), amber_)) &&
        SUCCEEDED(makeBrush(rgb(0xA33B49), red_)) &&
        SUCCEEDED(makeBrush(rgb(0xB9AA8A), off_));

    if (!ok) {
        discardResources();
        return false;
    }
    return true;
}

void App::discardResources() {
    target_.Reset();
    bg_.Reset(); panel_.Reset(); stroke_.Reset(); white_.Reset(); muted_.Reset();
    accent_.Reset(); green_.Reset(); amber_.Reset(); red_.Reset(); off_.Reset();
}

void App::rounded(float x, float y, float width, float height, float radius, ID2D1Brush* brush) {
    target_->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(x, y, x + width, y + height), radius, radius), brush);
}

void App::text(const wchar_t* value, float x, float y, float width, float height, float size, ID2D1Brush* brush, DWRITE_TEXT_ALIGNMENT alignment) {
    if (!value || !brush || !target_ || !writeFactory_) return;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
    const auto weight = size >= 17.0f ? DWRITE_FONT_WEIGHT_BOLD : (size >= 11.0f ? DWRITE_FONT_WEIGHT_SEMI_BOLD : DWRITE_FONT_WEIGHT_NORMAL);
    if (FAILED(writeFactory_->CreateTextFormat(size >= 17.0f ? L"Palatino Linotype" : L"Segoe UI", nullptr, weight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        size, L"en-us", format.ReleaseAndGetAddressOf()))) return;
    format->SetTextAlignment(alignment);
    format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    target_->DrawTextW(value, static_cast<UINT32>(std::wcslen(value)), format.Get(), D2D1::RectF(x, y, x + width, y + height), brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void App::led(float x, float y, LedState state) {
    ID2D1Brush* brush = off_.Get();
    if (state == LedState::Active) brush = green_.Get();
    else if (state == LedState::Waiting) brush = amber_.Get();
    else if (state == LedState::Error) brush = red_.Get();
    target_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 5.0f, 5.0f), brush);
}

void App::card(float y, const wchar_t* name, const wchar_t* subtitle, const wchar_t* key, const FeatureView& feature) {
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> fill;
    target_->CreateSolidColorBrush(feature.enabled ? rgb(0xF8EDEF) : rgb(0xFFFCF5), fill.ReleaseAndGetAddressOf());
    rounded(24.0f, y, 572.0f, 64.0f, 12.0f, fill.Get());
    target_->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(24.0f, y, 596.0f, y + 64.0f), 12.0f, 12.0f),
        feature.enabled ? accent_.Get() : stroke_.Get(), feature.enabled ? 1.5f : 1.0f);

    led(46.0f, y + 32.0f, feature.led);
    text(name, 66.0f, y + 7.0f, 240.0f, 25.0f, 13.0f, white_.Get());
    text(subtitle, 66.0f, y + 32.0f, 315.0f, 20.0f, 10.0f, muted_.Get());

    wchar_t writes[48]{};
    swprintf_s(writes, L"%llu corrections", static_cast<unsigned long long>(feature.writes));
    text(writes, 354.0f, y + 18.0f, 125.0f, 28.0f, 9.5f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    rounded(502.0f, y + 14.0f, 70.0f, 36.0f, 18.0f, feature.enabled ? accent_.Get() : stroke_.Get());
    text(feature.enabled ? L"ON" : key, 502.0f, y + 14.0f, 70.0f, 36.0f, 10.0f,
        feature.enabled ? panel_.Get() : muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
}

void App::paint() {
    if (!createResources()) return;
    const UiSnapshot view = trainer_.snapshot();

    target_->BeginDraw();
    target_->Clear(rgb(0xF4EFE5));
    target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

    // Crisp light header with confident contrast.
    rounded(24.0f, 18.0f, 36.0f, 36.0f, 10.0f, amber_.Get());
    text(L"T", 24.0f, 18.0f, 36.0f, 36.0f, 18.0f, panel_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    text(L"IGI COMMAND", 72.0f, 11.0f, 290.0f, 31.0f, 18.0f, white_.Get());
    text(L"OFFLINE OPERATIONS CONSOLE", 72.0f, 38.0f, 290.0f, 17.0f, 9.0f, muted_.Get());

    rounded(530.0f, 18.0f, 36.0f, 36.0f, 10.0f, panel_.Get());
    target_->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(530.0f, 18.0f, 566.0f, 54.0f), 10.0f, 10.0f), stroke_.Get(), 1.0f);
    text(L"-", 530.0f, 16.0f, 36.0f, 36.0f, 15.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    rounded(570.0f, 18.0f, 36.0f, 36.0f, 10.0f, panel_.Get());
    target_->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(570.0f, 18.0f, 606.0f, 54.0f), 10.0f, 10.0f), stroke_.Get(), 1.0f);
    text(L"x", 570.0f, 18.0f, 36.0f, 36.0f, 10.0f, red_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

    target_->DrawLine(D2D1::Point2F(24.0f, 66.0f), D2D1::Point2F(596.0f, 66.0f), amber_.Get(), 1.5f);

    // Hero movement command card, unique but restrained.
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> movementFill;
    target_->CreateSolidColorBrush(view.movement.enabled ? rgb(0xF6E9EC) : rgb(0xFFFCF5), movementFill.ReleaseAndGetAddressOf());
    rounded(24.0f, 86.0f, 572.0f, 57.0f, 12.0f, movementFill.Get());
    target_->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(24.0f, 86.0f, 596.0f, 143.0f), 12.0f, 12.0f),
        view.movement.enabled ? accent_.Get() : stroke_.Get(), view.movement.enabled ? 1.5f : 1.0f);

    led(46.0f, 114.5f, view.movement.led);
    text(L"MOBILITY COMMAND", 66.0f, 90.0f, 180.0f, 24.0f, 12.5f, white_.Get());
    text(view.fallProtection ? L"Travel, jump and landing protection engaged" : L"Standard mobility profile",
        66.0f, 114.0f, 285.0f, 18.0f, 9.5f, view.fallProtection ? green_.Get() : muted_.Get());

    wchar_t level[24]{};
    swprintf_s(level, L"%d x", view.movementLevel);
    text(level, 354.0f, 93.0f, 48.0f, 34.0f, 17.0f, accent_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

    const float segmentStart = 418.0f;
    for (int index = 0; index < 5; ++index) {
        const bool active = index < std::clamp(view.movementLevel, 1, 5);
        rounded(segmentStart + index * 27.0f, 108.0f, 19.0f, 9.0f, 4.5f, active ? accent_.Get() : off_.Get());
    }
    text(L"F4", 557.0f, 92.0f, 23.0f, 24.0f, 9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    // Highly legible feature controls.
    card(157.0f, L"DAMAGE PROTECTION", L"Maintains full operational health", L"F1", view.invincible);
    card(229.0f, L"MAGAZINE RESERVE", L"Maintains the active weapon magazine", L"F2", view.magazine);
    card(301.0f, L"INVENTORY RESERVE", L"Maintains all valid item quantities", L"F3", view.inventory);

    // Live status and telemetry.
    rounded(24.0f, 373.0f, 572.0f, 25.0f, 8.0f, panel_.Get());
    target_->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(24.0f, 373.0f, 596.0f, 398.0f), 8.0f, 8.0f), stroke_.Get(), 1.0f);
    led(41.0f, 385.5f, view.attached ? LedState::Active : LedState::Waiting);
    text(view.status.c_str(), 55.0f, 373.0f, 280.0f, 25.0f, 9.0f, view.attached ? green_.Get() : amber_.Get());

    wchar_t runtime[96]{};
    if (view.attached) {
        swprintf_s(runtime, L"PID %lu    HP %.0f%%    ROOT %.2f", view.pid,
            static_cast<double>(view.healthPercent), static_cast<double>(view.rootScale));
    } else {
        wcscpy_s(runtime, L"Waiting for igi.exe");
    }
    text(runtime, 335.0f, 373.0f, 240.0f, 25.0f, 9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    // Footer.
    rounded(24.0f, 406.0f, 142.0f, 40.0f, 10.0f, panel_.Get());
    target_->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(24.0f, 406.0f, 166.0f, 446.0f), 10.0f, 10.0f), stroke_.Get(), 1.0f);
    text(L"F12  RESET ALL", 24.0f, 406.0f, 142.0f, 40.0f, 9.5f, amber_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    text(L"F1-F4 HOTKEYS OR CLICK A CONTROL", 190.0f, 406.0f, 285.0f, 40.0f, 9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    text(L"ESC  EXIT", 500.0f, 406.0f, 96.0f, 40.0f, 9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    const HRESULT result = target_->EndDraw();
    if (result == D2DERR_RECREATE_TARGET) discardResources();
}

void App::showAbout() {
    MessageBoxW(hwnd_,
        L"IGI Command\n\nF1 Damage Protection\nF2 Magazine Reserve\nF3 Inventory Reserve\nF4 Mobility Command 1x-5x\nF12 Reset All\n\nIntegrated landing protection.\nOffline single-player use only.\n\nDesigned and developed by TJM.",
        L"IGI Command", MB_OK | MB_ICONINFORMATION);
}
}
