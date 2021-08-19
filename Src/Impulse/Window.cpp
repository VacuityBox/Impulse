#include "PCH.hpp"
#include "Window.hpp"

#include "Utility.hpp"

#include <spdlog/spdlog.h>

#include <windowsx.h>

namespace Impulse
{

auto Window::Dispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    // Return 0 when handling message, DefWindowProc otherwise.
    switch (message)
    {
    case WM_CREATE:  return OnCreate();
    case WM_DESTROY: return OnDestroy();
    case WM_COMMAND: return OnCommand(wParam, lParam);
    case WM_PAINT:   return OnPaint(wParam, lParam);
    case WM_SIZE:    return OnResize(wParam, lParam);
    case WM_KEYUP:   return OnKeyUp(static_cast<UINT>(wParam));
    case WM_KEYDOWN: return OnKeyDown(static_cast<UINT>(wParam));
    
    case WM_MOUSEMOVE:
        return OnMouseMove(
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam),
            static_cast<DWORD>(wParam)
        );

    case WM_LBUTTONUP:
        return OnLeftMouseButtonUp(
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam),
            static_cast<DWORD>(wParam)
        );

    case WM_LBUTTONDOWN:
        return OnLeftMouseButtonDown(
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam),
            static_cast<DWORD>(wParam)
        );

    case WM_RBUTTONUP:
        return OnLeftMouseButtonUp(
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam),
            static_cast<DWORD>(wParam)
        );

    case WM_RBUTTONDOWN:
        return OnLeftMouseButtonDown(
            GET_X_LPARAM(lParam),
            GET_Y_LPARAM(lParam),
            static_cast<DWORD>(wParam)
        );
    }
    
    return CustomDispatch(message, wParam, lParam);
}

auto Window::CustomDispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    return DefWindowProcW(mWindowHandle, message, wParam, lParam);
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

auto Window::InternalCreate (
    std::wstring_view title,
    DWORD             style,
    DWORD             styleEx,
    int               x,
    int               y,
    int               width,
    int               height,
    HWND              parent,
    HINSTANCE         hInstance
) -> bool
{
    mInstanceHandle = hInstance;

    auto wcex          = WNDCLASSEX{ 0 };
    wcex.cbSize        = sizeof(wcex);
    wcex.hInstance     = mInstanceHandle;
    wcex.lpfnWndProc   = WindowProc;
    wcex.lpszClassName = ClassName();
    wcex.style         = CS_VREDRAW | CS_HREDRAW;

    if (!RegisterClassExW(&wcex))
    {
        spdlog::error("RegisterClassExW() failed: {}", GetLastErrorMessage());
        return false;
    }

    mWindowHandle = CreateWindowExW(
        styleEx,
        wcex.lpszClassName,
        title.data(),
        style,
        x,
        y,
        width,
        height,
        parent,
        nullptr,
        mInstanceHandle,
        this
    );

    if (!mWindowHandle)
    {
        spdlog::error("CreateWindowExW() failed: {}", GetLastErrorMessage());
        return false;
    }

    return true;
}

auto Window::InternalCleanup () -> void
{
    UnregisterClassW(ClassName(), mInstanceHandle);
}

} // namespace Impulse
