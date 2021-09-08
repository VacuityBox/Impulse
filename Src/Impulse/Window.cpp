#include "PCH.hpp"
#include "Window.hpp"
#include "Utility.hpp"

#include <spdlog/spdlog.h>
#include <Windows.h>

namespace Impulse
{

Window::~Window ()
{
    UnregisterClassW(mClassName.c_str(), mInstanceHandle);
}

auto Window::Dispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    // Return 0 when handling message, DefWindowProc otherwise.
    switch (message)
    {
    case WM_CLOSE:
        Close();
        return 0;

    case WM_SIZE:
        Resize(static_cast<UINT32>(LOWORD(lParam)), static_cast<UINT32>(HIWORD(lParam)));
        return 0;

    case WM_MOVE:
        Move(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_DPICHANGED:
        DpiChanged();
        return 0;

    case WM_KEYUP:
        KeyUp(static_cast<UINT>(wParam));
        return 0;

    case WM_KEYDOWN:
        KeyDown(static_cast<UINT>(wParam));
        return 0;    
    
    case WM_MOUSEMOVE:
        MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONUP:
        MouseUp(MouseButton::Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONDOWN:
        MouseDown(MouseButton::Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_RBUTTONUP:
        MouseUp(MouseButton::Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_RBUTTONDOWN:
        MouseDown(MouseButton::Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    }
    
    return CustomMessageHandler(message, wParam, lParam);
}

auto Window::Close () -> void
{
    OnClose();
}

auto Window::Resize (UINT32 width, UINT32 height) -> void
{
    mWindowSize = D2D1::SizeU(width, height);
    OnResize(width, height);
}

auto Window::Move (int x, int y) -> void
{
    mWindowPosition = D2D1::Point2L(x, y);
    OnMove(x, y);
}

auto Window::DpiChanged () -> void
{
    const auto dpi = UpdateDpi();
    OnDpiChanged(dpi);
}

auto Window::KeyDown (UINT key) -> void
{
    if (key < mKeys.size())
    {
        mKeys[key] = true;
    }

    OnKeyDown(key);
}

auto Window::KeyUp (UINT key) -> void
{
    if (key < mKeys.size())
    {
        mKeys[key] = false;
    }

    OnKeyUp(key);
}

auto Window::MouseDown (MouseButton button, int x, int y) -> void
{
    mMouseButtons[static_cast<unsigned char>(button)] = true;

    OnMouseDown(button, x, y);
}

auto Window::MouseUp (MouseButton button, int x, int y) -> void
{
    mMouseButtons[static_cast<unsigned char>(button)] = false;

    OnMouseUp(button, x, y);
}

auto Window::MouseMove (int x, int y) -> void
{
    mMousePosition.x = x;
    mMousePosition.y = y;

    OnMouseMove(x, y);
}

auto Window::WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    auto windowPtr = static_cast<Window*>(nullptr);

    if (message == WM_NCCREATE)
    {
        auto lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

        windowPtr = reinterpret_cast<Window*>(lpCreateStruct->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowPtr));
        
        windowPtr->mWindowHandle = hWnd;
    }
    else
    {
        windowPtr = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (windowPtr)
    {
        return windowPtr->Dispatch(message, wParam, lParam);
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

auto Window::UpdateDpi () -> float
{
    auto dpi = GetDpiForWindow(mWindowHandle);
    if (dpi == 0)
    {
        spdlog::warn("GetDpiForWindow() failed, using default dpi value (96)");
        dpi = 96;
    }

    mDpi = dpi;
    return dpi;
}

auto Window::Init (Window::Desc desc) -> bool
{
    auto hInstance = desc.instanceHandle != nullptr ? desc.instanceHandle : GetModuleHandleW(nullptr);
    if (!hInstance)
    {
        spdlog::error("hInstance is null");
        return false;
    }

    mInstanceHandle = desc.instanceHandle;
    mClassName      = desc.className;

    auto wcex          = WNDCLASSEX{ 0 };
    wcex.cbSize        = sizeof(wcex);
    wcex.hInstance     = mInstanceHandle;
    wcex.lpfnWndProc   = WindowProc;
    wcex.lpszClassName = mClassName.c_str();
    wcex.style         = CS_VREDRAW | CS_HREDRAW;

    if (!RegisterClassExW(&wcex))
    {
        spdlog::error("RegisterClassExW() failed: {}", GetLastErrorMessage());
        return false;
    }

    mWindowTitle    = desc.title;
    mWindowPosition = desc.position;
    mWindowSize     = desc.size;

    mWindowHandle = CreateWindowExW(
        desc.exstyle,
        mClassName.c_str(),
        mWindowTitle.c_str(),
        desc.style,
        mWindowPosition.x,
        mWindowPosition.y,
        mWindowSize.width,
        mWindowSize.height,
        desc.parentHandle,
        nullptr,
        mInstanceHandle,
        this
    );

    if (!mWindowHandle)
    {
        spdlog::error("CreateWindowExW() failed: {}", GetLastErrorMessage());
        return false;
    }

    UpdateDpi();

    if (!desc.invisible)
    {
        ShowWindow(mWindowHandle, SW_SHOW);
    }

    return true;
}

auto Window::MainLoop () -> int
{
    // Main message loop.
    auto msg = MSG{ 0 };
    while (auto ret = GetMessageW(&msg, NULL, 0, 0))
    {
        if (ret == -1)
        {
            spdlog::error("GetMessageW() failed: {}", GetLastErrorMessage());
            break;
        }
        else
        {
            if (!IsWindow(mWindowHandle) || !IsDialogMessageW(mWindowHandle, &msg))
            {
                TranslateMessage(&msg); 
                DispatchMessageW(&msg);
            }
        }
    }

    return static_cast<int>(msg.wParam);
}

} // namespace Impulse
