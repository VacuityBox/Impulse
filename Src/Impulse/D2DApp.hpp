#pragma once

#include "Window.hpp"

#include "DX.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse {

class D2DApp : public Window
{
public:
    struct Desc
    {
        Window::Desc windowDesc = Window::Desc();
    };

protected:
    // Direct2D.
    ComPtr<ID2D1Factory1>       mD2DFactory;
    ComPtr<ID2D1Device>         mD2DDevice;
    ComPtr<ID2D1DeviceContext>  mD2DDeviceContext;
    ComPtr<ID2D1Bitmap1>        mD2DTargetBitmap;

    // Direct3D.
    ComPtr<ID3D11Device>        mD3DDevice;
    ComPtr<ID3D11DeviceContext> mD3DDeviceContext;
    D3D_FEATURE_LEVEL           mD3DFeatureLevel   = D3D_FEATURE_LEVEL_9_1;

    // DXGI.
    ComPtr<IDXGISwapChain1>     mDXGISwapChain;

    // DWrite.
    ComPtr<IDWriteFactory>      mDWriteFactory;

    // 
    bool mRedraw = true;
    
protected:

    auto Redraw () { mRedraw = true; }

    auto CreateFactories    () -> bool;
    auto CreateD2D          () -> bool;
    auto CreateRenderTarget () -> bool;

protected:

    auto GetD2DFactory       () const { return mD2DFactory.Get(); }
    auto GetD2DDevice        () const { return mD2DDevice.Get(); }
    auto GetD2DDeviceContext () const { return mD2DDeviceContext.Get(); }
    auto GetD2DTargetBitmap  () const { return mD2DTargetBitmap.Get(); }

    auto GetD3DDevice        () const { return mD3DDevice.Get(); }
    auto GetD3DDeviceContext () const { return mD3DDeviceContext.Get(); }
    auto GetD3DFeatureLevel  () const { return mD3DFeatureLevel; }

    auto GetSwapChain        () const { return mDXGISwapChain.Get(); }

    auto GetDWriteFactory    () const { return mDWriteFactory.Get(); }


protected:
    virtual auto Draw () -> void;

protected:
    virtual auto OnResize (UINT32 width, UINT32 height) -> void
    {
        if (mD2DDeviceContext)
        {
            CreateRenderTarget();
        }
    }

    virtual auto OnDraw () -> void {}

public:
    D2DApp () = default;
    virtual ~D2DApp () = default;

    auto Init (D2DApp::Desc desc) -> bool;

    auto GfxLoop () -> int;
};

} // namespace Impulse
