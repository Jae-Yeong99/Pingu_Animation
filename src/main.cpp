#define UNICODE
#define _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <cmath>
#include <cstdlib>
#include <string>

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

namespace {

constexpr int kWindowWidth = 210;
constexpr int kWindowHeight = 180;
constexpr UINT_PTR kAnimationTimer = 1;
constexpr UINT kFrameMs = 16;
constexpr UINT kTrayIconId = 1;
constexpr UINT kTrayCallbackMessage = WM_APP + 1;
constexpr UINT kExitMenuCommand = 1001;

HWND gWindow = nullptr;
int gScreenWidth = 0;
int gScreenHeight = 0;
float gX = 0.0f;
float gY = 0.0f;
float gVelocityX = 2.4f;
float gTime = 0.0f;
bool gPaused = false;
bool gFacingRight = true;
bool gTrayIconAdded = false;
ULONG_PTR gGdiPlusToken = 0;
Gdiplus::Image* gSpriteSheet = nullptr;

NOTIFYICONDATAW CreateTrayData() {
    NOTIFYICONDATAW tray{};
    tray.cbSize = sizeof(tray);
    tray.hWnd = gWindow;
    tray.uID = kTrayIconId;
    tray.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tray.uCallbackMessage = kTrayCallbackMessage;
    tray.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    lstrcpynW(tray.szTip, L"Desktop Pet - right-click to exit",
              static_cast<int>(sizeof(tray.szTip) / sizeof(tray.szTip[0])));
    return tray;
}

void AddTrayIcon() {
    NOTIFYICONDATAW tray = CreateTrayData();
    gTrayIconAdded = Shell_NotifyIconW(NIM_ADD, &tray) != FALSE;
}

void RemoveTrayIcon() {
    if (!gTrayIconAdded) return;
    NOTIFYICONDATAW tray = CreateTrayData();
    Shell_NotifyIconW(NIM_DELETE, &tray);
    gTrayIconAdded = false;
}

void ShowTrayMenu() {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;

    AppendMenuW(menu, MF_STRING, kExitMenuCommand, L"Exit Desktop Pet");

    POINT cursor{};
    GetCursorPos(&cursor);
    SetForegroundWindow(gWindow);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                   cursor.x, cursor.y, 0, gWindow, nullptr);
    PostMessage(gWindow, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

std::wstring SpritePath() {
    wchar_t executablePath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    std::wstring path(executablePath);
    const std::wstring::size_type slash = path.find_last_of(L"\\/");
    if (slash != std::wstring::npos) path.resize(slash);
    return path + L"\\assets\\penguin_walk.png";
}

bool LoadSpriteSheet() {
    delete gSpriteSheet;
    gSpriteSheet = new Gdiplus::Image(SpritePath().c_str());
    if (gSpriteSheet->GetLastStatus() == Gdiplus::Ok) return true;
    delete gSpriteSheet;
    gSpriteSheet = nullptr;
    return false;
}

void RenderPet() {
    if (!gWindow || !gSpriteSheet) return;

    HDC screenDc = GetDC(nullptr);
    HDC memoryDc = CreateCompatibleDC(screenDc);
    if (!screenDc || !memoryDc) {
        if (memoryDc) DeleteDC(memoryDc);
        if (screenDc) ReleaseDC(nullptr, screenDc);
        return;
    }

    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = kWindowWidth;
    bitmapInfo.bmiHeader.biHeight = -kWindowHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* pixels = nullptr;
    HBITMAP bitmap = CreateDIBSection(screenDc, &bitmapInfo, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!bitmap) {
        DeleteDC(memoryDc);
        ReleaseDC(nullptr, screenDc);
        return;
    }
    HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);

    {
        Gdiplus::Bitmap canvas(kWindowWidth, kWindowHeight, kWindowWidth * 4,
                               PixelFormat32bppPARGB, static_cast<BYTE*>(pixels));
        Gdiplus::Graphics graphics(&canvas);
        graphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
        graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        graphics.Clear(Gdiplus::Color(0, 0, 0, 0));

        const int frame = (static_cast<int>(gTime) / 8) % 4;
        const int column = frame % 2;
        const int row = frame / 2;
        const int sourceWidth = static_cast<int>(gSpriteSheet->GetWidth() / 2);
        const int sourceHeight = static_cast<int>(gSpriteSheet->GetHeight() / 2);
        const int bob = static_cast<int>(std::abs(std::sin(gTime * 0.20f)) * 3.0f);

        if (!gFacingRight) {
            graphics.TranslateTransform(static_cast<Gdiplus::REAL>(kWindowWidth), 0.0f);
            graphics.ScaleTransform(-1.0f, 1.0f);
        }

        Gdiplus::Rect destination(15, bob, 180, 180);
        graphics.DrawImage(gSpriteSheet, destination,
                           column * sourceWidth, row * sourceHeight,
                           sourceWidth, sourceHeight, Gdiplus::UnitPixel);
    }

    POINT destination{static_cast<LONG>(gX), static_cast<LONG>(gY)};
    POINT source{0, 0};
    SIZE size{kWindowWidth, kWindowHeight};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(gWindow, screenDc, &destination, &size, memoryDc,
                        &source, 0, &blend, ULW_ALPHA);

    SelectObject(memoryDc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDc);
    ReleaseDC(nullptr, screenDc);
}

void AdvanceAnimation() {
    if (gPaused) return;

    gTime += 1.0f;
    gX += gVelocityX;

    if (gX <= -38.0f) {
        gX = -38.0f;
        gVelocityX = std::abs(gVelocityX);
        gFacingRight = true;
    } else if (gX >= static_cast<float>(gScreenWidth - kWindowWidth + 38)) {
        gX = static_cast<float>(gScreenWidth - kWindowWidth + 38);
        gVelocityX = -std::abs(gVelocityX);
        gFacingRight = false;
    }

    SetWindowPos(gWindow, HWND_TOPMOST, static_cast<int>(gX), static_cast<int>(gY), 0, 0,
                 SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    RenderPet();
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        gWindow = window;
        if (!LoadSpriteSheet()) return -1;
        SetTimer(window, kAnimationTimer, kFrameMs, nullptr);
        AddTrayIcon();
        return 0;
    case WM_TIMER:
        if (wParam == kAnimationTimer) AdvanceAnimation();
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        BeginPaint(window, &paint);
        EndPaint(window, &paint);
        RenderPet();
        return 0;
    }
    case WM_HOTKEY:
        if (wParam == 1) DestroyWindow(window);       // Ctrl+Alt+Q
        if (wParam == 2) gPaused = !gPaused;          // Ctrl+Alt+P
        return 0;
    case kTrayCallbackMessage:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) ShowTrayMenu();
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == kExitMenuCommand) DestroyWindow(window);
        return 0;
    case WM_NCHITTEST:
        // The pet never intercepts mouse input intended for the app underneath it.
        return HTTRANSPARENT;
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        KillTimer(window, kAnimationTimer);
        RemoveTrayIcon();
        UnregisterHotKey(window, 1);
        UnregisterHotKey(window, 2);
        delete gSpriteSheet;
        gSpriteSheet = nullptr;
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(window, message, wParam, lParam);
    }
}

} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    Gdiplus::GdiplusStartupInput gdiPlusInput;
    if (Gdiplus::GdiplusStartup(&gGdiPlusToken, &gdiPlusInput, nullptr) != Gdiplus::Ok) return 1;

    gScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    gScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    gX = 50.0f;
    gY = static_cast<float>(gScreenHeight - kWindowHeight - 30);

    const wchar_t* className = L"WindowAnimationDesktopPet";
    WNDCLASS windowClass{};
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = nullptr;

    if (!RegisterClass(&windowClass)) {
        Gdiplus::GdiplusShutdown(gGdiPlusToken);
        return 1;
    }

    gWindow = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        className, L"Desktop Pet", WS_POPUP,
        static_cast<int>(gX), static_cast<int>(gY), kWindowWidth, kWindowHeight,
        nullptr, nullptr, instance, nullptr);
    if (!gWindow) {
        Gdiplus::GdiplusShutdown(gGdiPlusToken);
        return 1;
    }

    RegisterHotKey(gWindow, 1, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'Q');
    RegisterHotKey(gWindow, 2, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'P');
    ShowWindow(gWindow, SW_SHOWNOACTIVATE);
    RenderPet();

    MSG message{};
    while (GetMessage(&message, nullptr, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    const int result = static_cast<int>(message.wParam);
    Gdiplus::GdiplusShutdown(gGdiPlusToken);
    return result;
}
