#pragma once

#include "Button.hpp"
#include "Window.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse
{

class ImpulseApp : public Window
{
    ComPtr<ID2D1Factory>          mD2DFactory;
    ComPtr<ID2D1HwndRenderTarget> mRenderTarget;
    ComPtr<IDWriteFactory>        mDWriteFactory;
    
    ComPtr<IDWriteTextFormat>     mButtonTextFormat;

    ComPtr<ID2D1SolidColorBrush>  mBackgroundBrush;
    ComPtr<ID2D1SolidColorBrush>  mDefaultTextBrush;
    ComPtr<ID2D1SolidColorBrush>  mDefaultOutlineBrush;
    ComPtr<ID2D1SolidColorBrush>  mHoverTextBrush;
    ComPtr<ID2D1SolidColorBrush>  mHoverOutlineBrush;
    ComPtr<ID2D1SolidColorBrush>  mActiveTextBrush;
    ComPtr<ID2D1SolidColorBrush>  mActiveOutlineBrush;
    ComPtr<ID2D1SolidColorBrush>  mFocusTextBrush;
    ComPtr<ID2D1SolidColorBrush>  mFocusOutlineBrush;
    ComPtr<ID2D1SolidColorBrush>  mDisabledTextBrush;
    ComPtr<ID2D1SolidColorBrush>  mDisabledOutlineBrush;

    std::unique_ptr<Button>       mCloseButton;
    std::unique_ptr<Button>       mSettingsButton;
    std::unique_ptr<Button>       mPauseButton;

    Widget*                       mClickedWidget;
    Widget*                       mHoveredWidget;

    // Create methods.
    auto CreateGraphicsResources  () -> HRESULT;
    auto DiscardGraphicsResources () -> void;

    auto CreateBrushes  () -> HRESULT;
    auto CreateFonts    () -> HRESULT;
    auto CreateButtons  () -> HRESULT;

    auto CalculateLayout () -> void;

    // Draw methods.
    auto DrawButtons () -> void;

    auto Redraw () -> void { InvalidateRect(mWindowHandle, nullptr, false); }

    // Get widget hit by point.
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

    virtual auto CustomDispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT override;

    virtual auto ClassName () const -> const wchar_t* override { return L"ImpulseApp_WndClass"; }

public:
    ImpulseApp  () = default;
    ~ImpulseApp () = default;

    auto Init    (HINSTANCE hInstance) -> bool;
    auto Release () -> void;
};

} // namespace Impulse
