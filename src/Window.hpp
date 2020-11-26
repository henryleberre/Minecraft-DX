#ifndef __MINECRAFT__WINDOW_HPP
#define __MINECRAFT__WINDOW_HPP

#include "Pch.hpp"
#include "ErrorHandler.hpp"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

class Window {

    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
    HWND m_handle = NULL;

    std::uint16_t m_width, m_height;

    std::array<bool, 0xFE> m_bDownKeys = { false };

public:
    Window(const char* title, const std::uint16_t width, const std::uint16_t height) noexcept
        : m_width(width), m_height(height)
    {
        WNDCLASSA wc = {};

        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpfnWndProc = WindowProc;
        wc.lpszClassName = "Minecraft's Window Class";

        if (!RegisterClassA(&wc))
            FATAL_ERROR("Failed to register a window class");

        RECT windowRect{ 0, 0, width, height };
        if (!AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false))
            FATAL_ERROR("Failed to adjust window rect");

        this->m_handle = CreateWindowExA(0, wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, wc.hInstance, NULL);

        if (!this->m_handle)
            FATAL_ERROR("Failed to create the window");

#ifdef _WIN64
        SetWindowLongPtrA(this->m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#else
        SetWindowLongA(this->m_handle, GWLP_USERDATA, reinterpret_cast<LONG>(this));
#endif

        this->ShowWindow();
    }

    inline void Destroy() noexcept {
        if (!DestroyWindow(this->m_handle))
            FATAL_ERROR("Failed to destroy the window");

        this->m_handle = NULL;
    }

    ~Window() noexcept {
        if (this->m_handle)
            this->Destroy();
    }

    inline bool IsKeyDown(const char c) const noexcept { return  this->m_bDownKeys[std::toupper(c)]; }
    inline bool IsKeyUp(const char c) const noexcept { return !this->m_bDownKeys[std::toupper(c)]; }

    inline std::uint16_t GetWidth()  const noexcept { return this->m_width; }
    inline std::uint16_t GetHeight() const noexcept { return this->m_height; }

    inline bool ClipCursor() {
        RECT clipRect;
        if (!GetClientRect(this->m_handle, &clipRect))
            FATAL_ERROR("Failed get the window's client rect");

        POINT points[2] = {{clipRect.left, clipRect.top}, {clipRect.right, clipRect.bottom}};
        MapWindowPoints(this->m_handle, nullptr, points, 2u);

        clipRect.left   = points[0].x;
        clipRect.top    = points[0].y;
        clipRect.right  = points[1].x;
        clipRect.bottom = points[1].y;

        if (!::ClipCursor(&clipRect))
            FATAL_ERROR("Failed to clip the cursor");
    }

    inline void ShowCursor() const noexcept { ::ShowCursor(1u); }
    inline void HideCursor() const noexcept { ::ShowCursor(0u); }

    inline void HideWindow() const noexcept { ::ShowWindow(this->m_handle, SW_HIDE); }
    inline void ShowWindow() const noexcept { ::ShowWindow(this->m_handle, SW_SHOW); }

    inline HWND GetHandle() const noexcept { return this->m_handle; }
    inline bool IsRunning() const noexcept { return this->m_handle != NULL; }

    void Update() {
        MSG message;
        while (PeekMessageA(&message, this->m_handle, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
}; // class Window

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
#ifdef _WIN64
    Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
#else
    Window* pWindow = reinterpret_cast<Window*>(GetWindowLong(hwnd, GWLP_USERDATA));
#endif

    if (pWindow) {
        switch (msg) {
        case WM_DESTROY:
            pWindow->m_handle = NULL;
            return 0;
        case WM_KEYDOWN:
            pWindow->m_bDownKeys[wParam] = true;
            return 0;
        case WM_KEYUP:
            pWindow->m_bDownKeys[wParam] = false;
            return 0;
        };
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
} // WindowProc

#endif // __MINECRAFT__WINDOW_HPP