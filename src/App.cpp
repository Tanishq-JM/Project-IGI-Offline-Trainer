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
constexpr wchar_t kWindowClass[] = L"TJM.ProjectIGI.NightOps.Reference";
constexpr int kClientWidth = 688;
constexpr int kClientHeight = 508;

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
        L"Project I.G.I // Night Ops",
        WS_POPUP | WS_MINIMIZEBOX,
        x, y, kClientWidth, kClientHeight,
        nullptr, nullptr, instance_, this);
    if (!hwnd_) return 1;

    SendMessageW(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
    SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd_, 20, &dark, sizeof(dark));
    const DWM_WINDOW_CORNER_PREFERENCE corners = DWMWCP_DONOTROUND;
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
        if (point.y < 88 && point.x < 574) return HTCAPTION;
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
                inside(x, y, 37, 119, 651, 181) ||
                inside(x, y, 37, 181, 651, 241) ||
                inside(x, y, 37, 241, 651, 301) ||
                inside(x, y, 37, 301, 651, 361) ||
                inside(x, y, 37, 425, 159, 453) ||
                inside(x, y, 569, 39, 595, 65) ||
                inside(x, y, 601, 39, 627, 65);
            SetCursor(LoadCursorW(nullptr, clickable ? IDC_HAND : IDC_ARROW));
            return TRUE;
        }
        break;
    }
    case WM_LBUTTONUP: {
        const float x = static_cast<float>(GET_X_LPARAM(lParam));
        const float y = static_cast<float>(GET_Y_LPARAM(lParam));
        if (inside(x, y, 37, 119, 651, 181)) trainer_.cycleMovement();
        else if (inside(x, y, 37, 181, 651, 241)) trainer_.toggleInvincible();
        else if (inside(x, y, 37, 241, 651, 301)) trainer_.toggleMagazine();
        else if (inside(x, y, 37, 301, 651, 361)) trainer_.toggleInventory();
        else if (inside(x, y, 37, 425, 159, 453)) trainer_.disableAll();
        else if (inside(x, y, 569, 39, 595, 65)) ShowWindow(window, SW_MINIMIZE);
        else if (inside(x, y, 601, 39, 627, 65)) DestroyWindow(window);
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
        SUCCEEDED(makeBrush(rgb(0x05070A), bg_)) &&
        SUCCEEDED(makeBrush(rgb(0x0D1013), panel_)) &&
        SUCCEEDED(makeBrush(rgb(0x2A3238), stroke_)) &&
        SUCCEEDED(makeBrush(rgb(0xE8ECEE), white_)) &&
        SUCCEEDED(makeBrush(rgb(0x5C6970), muted_)) &&
        SUCCEEDED(makeBrush(rgb(0x4DD8C4), accent_)) &&
        SUCCEEDED(makeBrush(rgb(0x4DD8C4), green_)) &&
        SUCCEEDED(makeBrush(rgb(0xC99A3E), amber_)) &&
        SUCCEEDED(makeBrush(rgb(0xD8574D), red_)) &&
        SUCCEEDED(makeBrush(rgb(0x1A2226), off_));

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
    const auto weight = size >= 12.0f ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
    if (FAILED(writeFactory_->CreateTextFormat(
        L"Consolas", nullptr, weight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        size, L"en-us", format.ReleaseAndGetAddressOf()))) return;
    format->SetTextAlignment(alignment);
    format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    target_->DrawTextW(value, static_cast<UINT32>(std::wcslen(value)), format.Get(),
        D2D1::RectF(x, y, x + width, y + height), brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void App::led(float x, float y, LedState state) {
    ID2D1Brush* brush = stroke_.Get();
    if (state == LedState::Active) brush = accent_.Get();
    else if (state == LedState::Waiting) brush = amber_.Get();
    else if (state == LedState::Error) brush = red_.Get();
    target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 3.0f, 3.0f), brush, 1.0f);
    if (state == LedState::Active || state == LedState::Waiting || state == LedState::Error) {
        target_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), 1.2f, 1.2f), brush);
    }
}

void App::card(float y, const wchar_t* name, const wchar_t* subtitle, const wchar_t* key, const FeatureView& feature) {
    target_->DrawLine(D2D1::Point2F(37.0f, y + 60.0f), D2D1::Point2F(627.0f, y + 60.0f), off_.Get(), 1.0f);
    led(40.0f, y + 30.0f, feature.led);
    text(name, 57.0f, y + 12.0f, 290.0f, 20.0f, 12.0f, white_.Get());
    text(subtitle, 57.0f, y + 32.0f, 330.0f, 17.0f, 10.0f, muted_.Get());

    wchar_t corrections[64]{};
    swprintf_s(corrections, L"%llu corrections", static_cast<unsigned long long>(feature.writes));
    text(corrections, 443.0f, y + 18.0f, 136.0f, 24.0f, 10.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    target_->DrawRectangle(D2D1::RectF(593.0f, y + 17.0f, 627.0f, y + 43.0f), stroke_.Get(), 1.0f);
    text(feature.enabled ? L"ON" : key, 593.0f, y + 17.0f, 34.0f, 26.0f, 9.5f,
        feature.enabled ? accent_.Get() : muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
}

void App::paint() {
    if (!createResources()) return;
    const UiSnapshot view = trainer_.snapshot();

    target_->BeginDraw();
    target_->Clear(rgb(0x05070A));
    target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    target_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

    // Outer panel and reference-matched header.
    target_->DrawRectangle(D2D1::RectF(12.0f, 8.0f, 652.0f, 476.0f), stroke_.Get(), 1.0f);

    Microsoft::WRL::ComPtr<ID2D1PathGeometry> badgeGeometry;
    factory_->CreatePathGeometry(badgeGeometry.ReleaseAndGetAddressOf());
    if (badgeGeometry) {
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        if (SUCCEEDED(badgeGeometry->Open(sink.ReleaseAndGetAddressOf()))) {
            sink->BeginFigure(D2D1::Point2F(43.0f, 33.0f), D2D1_FIGURE_BEGIN_FILLED);
            sink->AddLine(D2D1::Point2F(74.0f, 33.0f));
            sink->AddLine(D2D1::Point2F(69.0f, 71.0f));
            sink->AddLine(D2D1::Point2F(38.0f, 71.0f));
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();
            target_->FillGeometry(badgeGeometry.Get(), off_.Get());
        }
    }
    text(L"IGI", 39.0f, 35.0f, 34.0f, 34.0f, 10.0f, accent_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

    text(L"PROJECT I.G.I", 89.0f, 31.0f, 142.0f, 22.0f, 14.0f, white_.Get());
    text(L"// NIGHT OPS", 230.0f, 31.0f, 124.0f, 22.0f, 14.0f, accent_.Get());
    text(L"TACTICAL CONTROL SYSTEM", 89.0f, 53.0f, 220.0f, 15.0f, 9.0f, muted_.Get());

    target_->DrawRectangle(D2D1::RectF(569.0f, 39.0f, 595.0f, 65.0f), stroke_.Get(), 1.0f);
    text(L"-", 569.0f, 39.0f, 26.0f, 26.0f, 10.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    target_->DrawRectangle(D2D1::RectF(601.0f, 39.0f, 627.0f, 65.0f), red_.Get(), 1.0f);
    text(L"x", 601.0f, 39.0f, 26.0f, 26.0f, 10.0f, red_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    target_->DrawLine(D2D1::Point2F(37.0f, 87.0f), D2D1::Point2F(627.0f, 87.0f), stroke_.Get(), 1.0f);

    // Progress strip.
    target_->FillRectangle(D2D1::RectF(37.0f, 102.0f, 184.0f, 104.0f), accent_.Get());
    target_->FillRectangle(D2D1::RectF(186.0f, 102.0f, 332.0f, 104.0f), amber_.Get());
    target_->FillRectangle(D2D1::RectF(334.0f, 102.0f, 627.0f, 104.0f), off_.Get());

    // Traversal row.
    const float traversalY = 119.0f;
    target_->DrawLine(D2D1::Point2F(37.0f, traversalY + 62.0f), D2D1::Point2F(627.0f, traversalY + 62.0f), off_.Get(), 1.0f);
    led(40.0f, traversalY + 31.0f, view.movement.led);
    text(L"TRAVERSAL SYSTEM", 57.0f, traversalY + 12.0f, 250.0f, 20.0f, 12.0f, white_.Get());
    text(view.fallProtection ? L"Boosted traversal with landing shield" : L"Standard traversal profile",
        57.0f, traversalY + 32.0f, 330.0f, 17.0f, 10.0f, muted_.Get());

    wchar_t moveTag[16]{};
    swprintf_s(moveTag, L"%dX", view.movementLevel);
    text(moveTag, 452.0f, traversalY + 16.0f, 28.0f, 24.0f, 11.0f, accent_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    for (int index = 0; index < 5; ++index) {
        const bool active = index < std::clamp(view.movementLevel, 1, 5);
        target_->FillRectangle(
            D2D1::RectF(488.0f + index * 19.0f, traversalY + 28.0f, 504.0f + index * 19.0f, traversalY + 32.0f),
            active ? accent_.Get() : stroke_.Get());
    }

    target_->DrawRectangle(D2D1::RectF(593.0f, traversalY + 17.0f, 627.0f, traversalY + 43.0f), stroke_.Get(), 1.0f);
    text(L"F4", 593.0f, traversalY + 17.0f, 34.0f, 26.0f, 9.5f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);

    card(181.0f, L"DAMAGE CONTROL", L"Suppresses accumulated player damage", L"F1", view.invincible);
    card(241.0f, L"WEAPON RESUPPLY", L"Maintains the active weapon magazine", L"F2", view.magazine);
    card(301.0f, L"FIELD INVENTORY", L"Maintains all valid item quantities", L"F3", view.inventory);

    // Status bar.
    target_->FillRectangle(D2D1::RectF(37.0f, 376.0f, 627.0f, 409.0f), off_.Get());
    target_->FillRectangle(D2D1::RectF(37.0f, 376.0f, 39.0f, 409.0f), amber_.Get());
    led(56.0f, 392.5f, view.attached ? LedState::Active : LedState::Waiting);
    text(view.attached ? L"MISSION LINK ACTIVE" : L"WAITING FOR IGI.EXE", 69.0f, 376.0f, 260.0f, 33.0f, 10.0f,
        view.attached ? accent_.Get() : amber_.Get());

    wchar_t status[96]{};
    if (view.attached) {
        swprintf_s(status, L"PID %lu   HP %.0f%%   ROOT %.2f", view.pid,
            static_cast<double>(view.healthPercent), static_cast<double>(view.rootScale));
    } else {
        wcscpy_s(status, L"STANDBY");
    }
    text(status, 420.0f, 376.0f, 188.0f, 33.0f, 9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    // Footer and authorship.
    target_->DrawRectangle(D2D1::RectF(37.0f, 425.0f, 159.0f, 453.0f), red_.Get(), 1.0f);
    text(L"F12 RESET ALL", 37.0f, 425.0f, 122.0f, 28.0f, 9.5f, red_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    text(L"F1-F4 HOTKEYS OR CLICK A CONTROL", 209.0f, 425.0f, 268.0f, 28.0f, 9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_CENTER);
    text(L"ESC EXIT", 548.0f, 425.0f, 79.0f, 28.0f, 9.0f, muted_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);
    text(L"DESIGNED BY TJM", 457.0f, 456.0f, 170.0f, 14.0f, 8.0f, accent_.Get(), DWRITE_TEXT_ALIGNMENT_TRAILING);

    const HRESULT result = target_->EndDraw();
    if (result == D2DERR_RECREATE_TARGET) discardResources();
}

void App::showAbout() {
    MessageBoxW(
        hwnd_,
        L"PROJECT I.G.I // NIGHT OPS\n\nF1 Damage Control\nF2 Weapon Resupply\nF3 Field Inventory\nF4 Traversal System 1x-5x\nF12 Reset All\n\nIntegrated landing protection.\nOffline single-player use only.\n\nDesigned by TJM.",
        L"PROJECT I.G.I // NIGHT OPS",
        MB_OK | MB_ICONINFORMATION);
}
}
