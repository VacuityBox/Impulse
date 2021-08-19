#pragma once

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
    
    ComPtr<IDWriteTextFormat>     mTextFormat;
    ComPtr<ID2D1SolidColorBrush>  mBackgroundBrush;


    auto CreateGraphicsResources  () -> HRESULT;
    auto DiscardGraphicsResources () -> void;

    auto CreateBrushes  () -> HRESULT;
    auto CreateFonts    () -> HRESULT;

    auto CalculateLayout () -> void;

    // Inherited via Window
    virtual auto OnCreate  () -> LRESULT override;
    virtual auto OnDestroy () -> LRESULT override;
    virtual auto OnCommand (WPARAM, LPARAM) -> LRESULT override { return 0; }
    virtual auto OnPaint   (WPARAM, LPARAM) -> LRESULT override;
    virtual auto OnResize  (WPARAM, LPARAM) -> LRESULT override;

    virtual auto OnKeyUp   (UINT) -> LRESULT override { return 0; }
    virtual auto OnKeyDown (UINT) -> LRESULT override { return 0; }

    virtual auto OnMouseMove            (int, int, DWORD) -> LRESULT override { return 0; }
    virtual auto OnLeftMouseButtonUp    (int, int, DWORD) -> LRESULT override { return 0; }
    virtual auto OnLeftMouseButtonDown  (int, int, DWORD) -> LRESULT override { return 0; }
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
