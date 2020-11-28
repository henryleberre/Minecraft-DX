#include "Window.hpp"

void Window::InitRawInput() noexcept {
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid.usUsage     = HID_USAGE_GENERIC_MOUSE;
    rid.hwndTarget  = NULL;
    rid.dwFlags     = 0u;
    
    if (!RegisterRawInputDevices(&rid, 1u, sizeof(RAWINPUTDEVICE)))
        FATAL_ERROR("Failed to register raw input devices");
}

Window::Window(const char* title, const std::uint16_t width, const std::uint16_t height) noexcept : m_width(width),
                                                                                                   m_height(height) {
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

    this->InitRawInput();
}

void Window::ClipCursor() {
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

void Window::Update() noexcept {
    this->m_mouseXDelta = 0;
    this->m_mouseYDelta = 0;

    MSG message;
    while (PeekMessageA(&message, this->m_handle, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

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
        case WM_INPUT:
            UINT dwSize;

            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)))
                FATAL_ERROR("Failed to get raw input data (wrong return value with pData = NULL)");
            
            std::unique_ptr<BYTE[]> pBuff = std::make_unique<BYTE[]>(dwSize);

            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, pBuff.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
                FATAL_ERROR("Failed to get raw input data (wrong size)");
            
            RAWINPUT* pRawInput = reinterpret_cast<RAWINPUT*>(pBuff.get());

            if (pRawInput->header.dwType == RIM_TYPEMOUSE) {
                pWindow->m_mouseXDelta += pRawInput->data.mouse.lLastX;
                pWindow->m_mouseYDelta += pRawInput->data.mouse.lLastY;
            } 

            return 0;
        };
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
} // WindowProc
