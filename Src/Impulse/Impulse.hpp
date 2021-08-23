#pragma once

#include "Button.hpp"
#include "StaticText.hpp"
#include "Timer.hpp"
#include "Window.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse
{

class ImpulseApp : public Window
{
    ComPtr<ID2D1Factory>          mD2DFactory;
    ComPtr<IDWriteFactory>        mDWriteFactory;
    ComPtr<ID2D1HwndRenderTarget> mRenderTarget;

    std::unique_ptr<Button>       mButtonClose;
    std::unique_ptr<Button>       mButtonSettings;
    std::unique_ptr<Button>       mButtonPause;
    std::unique_ptr<Button>       mButtonInfo;
    std::unique_ptr<Timer>        mTimerWidget;
    std::unique_ptr<StaticText>   mStaticImpulseState;
    std::unique_ptr<StaticText>   mStaticCurrentTask;

    Widget*                       mClickedWidget;
    Widget*                       mHoveredWidget;

    auto CreateGraphicsResources  () -> HRESULT;
    auto DiscardGraphicsResources () -> void;
    auto CalculateLayout          () -> void;
    auto Redraw                   () -> void { InvalidateRect(mWindowHandle, nullptr, false); }

    // Create methods.
    auto CreateButtons    () -> HRESULT;
    auto CreateTimer      () -> HRESULT;
    auto CreateStaticText () -> HRESULT;

    // Draw methods.
    auto DrawButtons     () -> void;
    auto DrawTimer       () -> void;
    auto DrawStaticTexts () -> void;

    // Get widget that is hit by @point.
    auto HitTest (D2D_POINT_2F point) -> Widget*;

    // Inherited via Window
    virtual auto OnCreate  () -> LRESULT override;
    virtual auto OnDestroy () -> LRESULT override;
    virtual auto OnCommand (WPARAM, LPARAM) -> LRESULT override { return 0; }
    virtual auto OnPaint   (WPARAM, LPARAM) -> LRESULT override;
    virtual auto OnResize  (WPARAM, LPARAM) -> LRESULT override;

    virtual auto OnKeyUp   (UINT) -> LRESULT override { return 0; }
    virtual auto OnKeyDown (UINT) -> LRESULT override { return 0; }

    virtual auto OnMouseMove            (int, int, DWORD) -> LRESULT override;
    virtual auto OnLeftMouseButtonUp    (int, int, DWORD) -> LRESULT override;
    virtual auto OnLeftMouseButtonDown  (int, int, DWORD) -> LRESULT override;
    virtual auto OnRightMouseButtonUp   (int, int, DWORD) -> LRESULT override { return 0; }
    virtual auto OnRightMouseButtonDown (int, int, DWORD) -> LRESULT override { return 0; }

    virtual auto OnTimer () -> LRESULT override;

    virtual auto CustomDispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT override;

    virtual auto ClassName () const -> const wchar_t* override { return L"ImpulseApp_WndClass"; }

public:
    ImpulseApp  () = default;
    ~ImpulseApp () = default;

    auto Init    (HINSTANCE hInstance) -> bool;
    auto Release () -> void;
};

} // namespace Impulse
