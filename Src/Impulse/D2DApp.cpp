#include "PCH.hpp"
#include "D2DApp.hpp"

#include <spdlog/spdlog.h>

namespace Impulse {

auto D2DApp::CreateFactories () -> bool
{
    // Create D2D1 Factory.
    {
        spdlog::debug("Creating D2D1 Factory");

    #if defined(_DEBUG)
        const auto options = D2D1_FACTORY_OPTIONS{ D2D1_DEBUG_LEVEL_INFORMATION };
    #else
        const auto options = D2D1_FACTORY_OPTIONS{ };
    #endif

        DX_CALL(
            D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                __uuidof(mD2DFactory.Get()),
                &options,
                &mD2DFactory
            )
        );

        spdlog::debug("Successfully created D2D1 Factory");
    }

    // Create DWriteFactory.
    {
        spdlog::debug("Creating DWrite Factory");

        DX_CALL(
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(mDWriteFactory.Get()),
                &mDWriteFactory
            )
        );

        spdlog::debug("Successfully created DWriteFactory");
    }

    return true;
}

auto D2DApp::CreateD2D () -> bool
{
    auto dxgiDevice = ComPtr<IDXGIDevice1>();

    // Create D3D11 Device.
    {
        spdlog::debug("Creating D3D11 Device");

        auto flags = UINT{ D3D11_CREATE_DEVICE_BGRA_SUPPORT };

    #if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

        const auto featureLevels = std::array<D3D_FEATURE_LEVEL, 7>{
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };
        
        // Create the DX11 API device object, and get a corresponding context.
        DX_CALL(
            D3D11CreateDevice(
                nullptr,                  // specify nullptr for default adapter
                D3D_DRIVER_TYPE_HARDWARE,
                0,
                flags,
                featureLevels.data(),
                featureLevels.size(),
                D3D11_SDK_VERSION,
                &mD3DDevice,
                &mD3DFeatureLevel,
                &mD3DDeviceContext
            )
        );
        
        spdlog::debug("Successfully created D3D11 Device");
    }

    // Get DXGI device.
    {
        // Obtain the underlying DXGI device that created Direct3D11 device.
        DX_CALL(mD3DDevice.As(&dxgiDevice));
    }

    // Create D2D Device.
    {
        spdlog::debug("Creating D2D1 Device");

        // Create Direct2D device for 2D rendering.
        DX_CALL(mD2DFactory->CreateDevice(dxgiDevice.Get(), &mD2DDevice));

        // Get Direct2D device's corresponding device context object.
        DX_CALL(mD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &mD2DDeviceContext));

        spdlog::debug("Successfully created D2D1 Device");
    }

    return true;
}

auto D2DApp::CreateRenderTarget () -> bool
{
    if (!mD2DDeviceContext)
    {
        return false;
    }

    mD2DDeviceContext->SetTarget(nullptr);
    mD2DTargetBitmap = nullptr;

    if (mDXGISwapChain != nullptr)
    {
        // Resize
        auto hr = mDXGISwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    }
    else
    {
        // Create new swap chain.
        spdlog::debug("Creating SwapChain");

        auto dxgiFactory = ComPtr<IDXGIFactory2>();
        auto dxgiDevice  = ComPtr<IDXGIDevice1>();
        auto dxgiAdapter = ComPtr<IDXGIAdapter>();
                
        // Obtain the underlying DXGI device that created Direct3D11 device.
        DX_CALL(mD3DDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is runs on.
        DX_CALL(dxgiDevice->GetAdapter(&dxgiAdapter));
        
        // Get the factory object that created the DXGI device.
        DX_CALL(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));
        
        // Setup swapchain description.
        auto swapChainDesc               = DXGI_SWAP_CHAIN_DESC1{0};
        swapChainDesc.Width              = 0;                                // use automatic sizing
        swapChainDesc.Height             = 0;
        swapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;       // this is the most common swapchain format
        swapChainDesc.Stereo             = false; 
        swapChainDesc.SampleDesc.Count   = 1;                                // don't use multi-sampling
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount        = 2;                                // use double buffering to enable flip
        swapChainDesc.Scaling            = DXGI_SCALING_NONE;
        swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
        swapChainDesc.Flags              = 0;
    
        // Get the final swap chain for this window from the DXGI factory.
        DX_CALL(
            dxgiFactory->CreateSwapChainForHwnd(
                mD3DDevice.Get(),
                Handle(),
                &swapChainDesc,
                nullptr,
                nullptr,
                &mDXGISwapChain
            )
        );

        // Ensure that DXGI doesn't queue more than one frame at a time.
        DX_CALL(dxgiDevice->SetMaximumFrameLatency(1));

        spdlog::debug("Successfully created SwapChain");
    }

    spdlog::debug("Creating Render Target");

    // Now we set up the Direct2D render target bitmap linked to the swapchain. 
    // Whenever we render to this bitmap, it is directly rendered to the 
    // swap chain associated with the window.
    const auto bitmapProperties = BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        GetDpi(),
        GetDpi()
    );

    // Direct2D needs the DXGI version of the backbuffer surface pointer.
    auto dxgiSurface = ComPtr<IDXGISurface>();
    DX_CALL(mDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface)));
    
    // Get a D2D surface from the DXGI back buffer to use as the D2D render target.
    DX_CALL(
        mD2DDeviceContext->CreateBitmapFromDxgiSurface(
            dxgiSurface.Get(),
            bitmapProperties,
            &mD2DTargetBitmap
        )
    );

    // Now we can set the Direct2D render target.
    mD2DDeviceContext->SetTarget(mD2DTargetBitmap.Get());

    spdlog::debug("Successfully created Render Target");
 
    return true;
}

auto D2DApp::Draw () -> void
{
    if (mRedraw && mD2DDeviceContext)
    {
        mD2DDeviceContext->BeginDraw();
        mD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::White));

        OnDraw();

        mD2DDeviceContext->EndDraw();

        auto parameters = DXGI_PRESENT_PARAMETERS{0};
        auto hr = mDXGISwapChain->Present1(1, 0, &parameters);
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
        
        }

        mRedraw = false;
    }
}

auto D2DApp::Init (D2DApp::Desc desc) -> bool
{
    if (!Window::Init(desc.windowDesc))
    {
        return false;
    }

    if (!CreateFactories())
    {
        return false;
    }

    if (!CreateD2D())
    {
        return false;
    }

    if (!CreateRenderTarget())
    {
        return false;
    }

    return true;
}

auto D2DApp::GfxLoop () -> int
{
    auto msg = MSG{0};
    while (true)
    {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return 0;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

        Draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}

} // namespace Impulse
