#pragma once

#include <array>
#include <memory>
#include <string>

#include <Windows.h>
#include <d2d1.h>

namespace {
    using namespace D2D1;
}

namespace Impulse
{

enum class MouseButton : unsigned char
{
    Left,
    Middle,
    Right,
    Count
};

class Window
{
public:
    struct Desc
    {
        const wchar_t*  title          = L"Window";
        const wchar_t*  className      = L"WndClass";
        DWORD           style          = WS_OVERLAPPEDWINDOW;
        DWORD           exstyle        = 0;
        D2D1_POINT_2L   position       = D2D1::Point2L(CW_USEDEFAULT, CW_USEDEFAULT);
        D2D1_SIZE_U     size           = D2D1::SizeU(CW_USEDEFAULT, CW_USEDEFAULT);
        HWND            parentHandle   = nullptr;
        HINSTANCE       instanceHandle = nullptr;
        bool            invisible      = false;
    };

private:
    using MouseButtons = std::array<bool, static_cast<size_t>(MouseButton::Count)>;
    using Keys         = std::array<bool, 256>;

    HWND          mWindowHandle    = nullptr;
    HINSTANCE     mInstanceHandle  = nullptr;

    std::wstring  mWindowTitle     = L"";
    std::wstring  mClassName       = L"";

    D2D1_POINT_2L mWindowPosition  = D2D1::Point2L(0, 0);
    D2D1_SIZE_U   mWindowSize      = D2D1::SizeU(0, 0);

    bool          mClosed          = false;
    bool          mInvisible       = false;
    float         mDpi             = 96.f;

    D2D1_POINT_2L mMousePosition   = D2D1::Point2L();
    MouseButtons  mMouseButtons    = MouseButtons();
    Keys          mKeys            = Keys();

protected:
    // Callbacks.
    virtual auto OnClose      () -> void {}

    virtual auto OnResize     (UINT32 width, UINT32 height) -> void {}
    virtual auto OnMove       (int x, int y)                -> void {}
    virtual auto OnDpiChange  (float dpi)                   -> void {}

    virtual auto OnKeyDown    (UINT key) -> void {}
    virtual auto OnKeyUp      (UINT key) -> void {}

    virtual auto OnMouseDown  (MouseButton button, int x, int y) -> void {}
    virtual auto OnMouseUp    (MouseButton button, int x, int y) -> void {}
    virtual auto OnMouseMove  (int x, int y)                     -> void {}

    // Override to handle custom messages.
    virtual auto CustomMessageHandler (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        return DefWindowProcW(mWindowHandle, message, wParam, lParam);
    }
    
private:
    // Window message dispatcher.
    auto Dispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;

    // Message handlers.
    auto Close     ()                                 -> void;
    auto Resize    (UINT32 width, UINT32 height)      -> void;
    auto Move      (int x, int y)                     -> void;
    auto DpiChange ()                                 -> void;
    auto KeyDown   (UINT key)                         -> void;
    auto KeyUp     (UINT key)                         -> void;
    auto MouseDown (MouseButton button, int x, int y) -> void;
    auto MouseUp   (MouseButton button, int x, int y) -> void;
    auto MouseMove (int x, int y)                     -> void;

    // WndProc callback.
    static auto CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;

    Window            (const Window&) = delete;
    Window& operator= (const Window&) = delete;

public:
    Window () = default;
    virtual ~Window ();
    
    auto Init (Window::Desc desc) -> bool;

    auto MainLoop () -> int;
    auto Quit     () -> void { PostQuitMessage(0); }

    auto Handle   () const { return mWindowHandle; }
    auto Instance () const { return mInstanceHandle; }
    auto GetDpi   () const { return mDpi; }

    auto Position () const { return mWindowPosition; }
    auto Left     () const { return mWindowPosition.x; }
    auto Top      () const { return mWindowPosition.y; }

    auto Size     () const { return mWindowSize; }
    auto Width    () const { return mWindowSize.width; }
    auto Height   () const { return mWindowSize.height; }

    auto Title    () const { return mWindowTitle; }

    auto Show     () { ShowWindow(mWindowHandle, SW_SHOW); }
    auto Hide     () { ShowWindow(mWindowHandle, SW_HIDE); }

    auto Position (int x, int y)
    {
        mWindowPosition = D2D1::Point2L(x, y);
        SetWindowPos(mWindowHandle, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    auto Size (UINT32 width, UINT32 height)
    {
        mWindowSize = D2D1::SizeU(width, height);
        SetWindowPos(mWindowHandle, nullptr, 0, 0, width, height, SWP_NOREPOSITION | SWP_NOZORDER);
    }

    auto Title (const wchar_t* title)
    {
        mWindowTitle = title;
        SetWindowTextW(mWindowHandle, title);
    }

    auto IsMouseButtonDown (MouseButton button) const -> bool
    {
        return mMouseButtons[static_cast<unsigned char>(button)];
    }

    auto IsMouseButtonUp (MouseButton button) const -> bool
    {
        return !IsMouseButtonDown(button);
    }

    auto GetMousePosition  () const -> D2D1_POINT_2L { return mMousePosition; }
    auto GetMousePositionX () const -> long          { return mMousePosition.x; }
    auto GetMousePositionY () const -> long          { return mMousePosition.y; }

    auto IsKeyDown (UINT key) const -> bool          { return key < mKeys.size() ? mKeys[key] : false; }
    auto IsKeyUp   (UINT key) const -> bool          { return !IsKeyDown(key); }
};

} // namespace Impulse
