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

    std::int16_t m_mouseXDelta = 0;
    std::int16_t m_mouseYDelta = 0;

private:
    void InitRawInput() noexcept;

public:
    Window(const char* title, const std::uint16_t width, const std::uint16_t height) noexcept;

    inline void Destroy() noexcept {
        if (!DestroyWindow(this->m_handle))
            FATAL_ERROR("Failed to destroy the window");

        this->m_handle = NULL;
    }

    inline ~Window() noexcept {
        if (this->m_handle)
            this->Destroy();
    }

    inline bool IsKeyDown(const char c) const noexcept { return  this->m_bDownKeys[std::toupper(c)]; }
    inline bool IsKeyUp  (const char c) const noexcept { return !this->m_bDownKeys[std::toupper(c)]; }

    inline std::uint16_t GetWidth()  const noexcept { return this->m_width; }
    inline std::uint16_t GetHeight() const noexcept { return this->m_height; }

    void ClipCursor();

    inline void ShowCursor() const noexcept { ::ShowCursor(1u); }
    inline void HideCursor() const noexcept { ::ShowCursor(0u); }

    inline void HideWindow() const noexcept { ::ShowWindow(this->m_handle, SW_HIDE); }
    inline void ShowWindow() const noexcept { ::ShowWindow(this->m_handle, SW_SHOW); }

    inline HWND GetHandle() const noexcept { return this->m_handle; }
    inline bool IsRunning() const noexcept { return this->m_handle != NULL; }

    inline std::int16_t GetMouseXDelta() const noexcept { return this->m_mouseXDelta; }
    inline std::int16_t GetMouseYDelta() const noexcept { return this->m_mouseYDelta; }

    void Update() noexcept;
}; // class Window

#endif // __MINECRAFT__WINDOW_HPP