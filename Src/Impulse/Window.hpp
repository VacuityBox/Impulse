#pragma once

#include <memory>
#include <string_view>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Impulse
{

class Window
{
protected:
    HWND      mWindowHandle;
    HINSTANCE mInstanceHandle;

private:
    virtual auto ClassName () const -> const wchar_t* = 0;

    // Return 0 when message is handled.
    virtual auto OnCreate  () -> LRESULT = 0;
    virtual auto OnDestroy () -> LRESULT = 0;

    virtual auto OnCommand (WPARAM, LPARAM) -> LRESULT = 0;
    virtual auto OnPaint   (WPARAM, LPARAM) -> LRESULT = 0;
    virtual auto OnResize  (WPARAM, LPARAM) -> LRESULT = 0;

    virtual auto OnKeyUp   (UINT) -> LRESULT = 0;
    virtual auto OnKeyDown (UINT) -> LRESULT = 0;

    virtual auto OnMouseMove            (int, int, DWORD) -> LRESULT = 0;
    virtual auto OnLeftMouseButtonUp    (int, int, DWORD) -> LRESULT = 0;
    virtual auto OnLeftMouseButtonDown  (int, int, DWORD) -> LRESULT = 0;
    virtual auto OnRightMouseButtonUp   (int, int, DWORD) -> LRESULT = 0;
    virtual auto OnRightMouseButtonDown (int, int, DWORD) -> LRESULT = 0;

    // Override to handle custom messages.
    virtual auto CustomDispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT = 0;
    
    // Window message dispatcher.
    auto Dispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;

    // WndProc callback.
    static auto CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;

protected:
    auto InternalCreate (
        std::wstring_view title,
        DWORD             style,
        DWORD             styleEx,
        int               x,
        int               y,
        int               width,
        int               height,
        HWND              parent,
        HINSTANCE         hInstance
    ) -> bool;

    auto InternalCleanup () -> void;

public:
    virtual ~Window () = default;

    auto MainLoop () -> int;

    auto Handle () { return mWindowHandle; }
};

} // namespace Impulse
