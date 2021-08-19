#include "PCH.hpp"
#include "Impulse.hpp"

#include "Utility.hpp"

namespace Impulse {

#pragma region "Creating Resources"

auto ImpulseApp::CreateGraphicsResources () -> HRESULT
{
    auto hr = S_OK;
    if (mRenderTarget == NULL)
    {
        spdlog::debug("Creating Graphics Resources");

        auto rc = RECT{ 0 };
        GetClientRect(mWindowHandle, &rc);
        auto size = D2D1::SizeU(rc.right, rc.bottom);

        spdlog::debug("Creating RenderTarget");
        hr = mD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(mWindowHandle, size),
            &mRenderTarget
        );
        if (FAILED(hr))
        {
            spdlog::error("CreateHwndRenderTarget() failed: {}", HResultToString(hr));
            return hr;
        }
        spdlog::debug("Successfully created RenderTarget");

        hr = CreateBrushes();
        hr = CreateFonts();
        if (FAILED(hr))
        {
            return hr;
        }
        
        CalculateLayout();

        spdlog::debug("Successfully created Graphics Resources");
    }

    return hr;
}

auto ImpulseApp::DiscardGraphicsResources () -> void
{
    spdlog::debug("Discarding Graphics Resources");

    mRenderTarget.Reset();
    mBackgroundBrush.Reset();    
}

auto ImpulseApp::CreateBrushes () -> HRESULT
{
    auto hr = S_OK;

    const auto backgroundColor = D2D1::ColorF(1.0f, 1.0f, 0.0f);
    const auto circleColor     = D2D1::ColorF(1.0f, 1.0f, 0.0f);

    hr = mRenderTarget->CreateSolidColorBrush(backgroundColor, mBackgroundBrush.GetAddressOf());
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", HResultToString(hr));
        return hr;
    }

    return hr;
}

auto ImpulseApp::CreateFonts () -> HRESULT
{
    auto hr = S_OK;

    // Create DWriteFactory.
    {
        spdlog::debug("Creating DWriteFactory");
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(mDWriteFactory), reinterpret_cast<IUnknown**>(mDWriteFactory.GetAddressOf()));
        if (FAILED(hr))
        {
            spdlog::error("DWriteCreateFactory() failed: {}", HResultToString(hr));
            return hr;
        }
        spdlog::debug("Successfully created DWriteFactory");
    }

    // Create a DirectWrite text format object.
    {
        spdlog::debug("Creating Button Font");
        hr = mDWriteFactory->CreateTextFormat(
            L"SegoeUI",
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16,
            L"", //locale
            mTextFormat.GetAddressOf()
        );
        if (FAILED(hr))
        {
            spdlog::error("CreateTextFormat() failed: {}", HResultToString(hr));
            return hr;
        }
        spdlog::debug("Successfully created Button Font");
    }
    
    return S_OK;
}

auto ImpulseApp::CalculateLayout () -> void
{
    if (mRenderTarget)
    {
    }
}

#pragma endregion

#pragma region "Window Messages"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::OnCreate () -> LRESULT
{
    spdlog::debug("Creating D2D1 Factory");

#if !defined(_DEBUG)
    const auto options = D2D1_FACTORY_OPTIONS{ };
#else
    const auto options = D2D1_FACTORY_OPTIONS{ D2D1_DEBUG_LEVEL_INFORMATION };
#endif

    auto hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, mD2DFactory.GetAddressOf());
    if (FAILED(hr))
    {
        spdlog::error("D2D1CreateFactory() failed: {}", HResultToString(hr));
        return -1;  // fail CreateWindowEx()
    }

    spdlog::debug("Successfully created D2D Factory");

    return 0;
}

auto ImpulseApp::OnDestroy () -> LRESULT
{
    DiscardGraphicsResources();
    mD2DFactory.Reset();

    PostQuitMessage(0);
    return 0;
}

auto ImpulseApp::OnPaint (WPARAM wParam, LPARAM lParam) -> LRESULT
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        auto ps = PAINTSTRUCT{};
        BeginPaint(mWindowHandle, &ps);
     
        mRenderTarget->BeginDraw();
        mRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

        hr = mRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }

        EndPaint(mWindowHandle, &ps);
    }

    return 0;
}

auto ImpulseApp::OnResize (WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (mRenderTarget != NULL)
    {
        auto rc = RECT{};
        GetClientRect(mWindowHandle, &rc);

        auto size = D2D1::SizeU(rc.right, rc.bottom);
        mRenderTarget->Resize(size);

        CalculateLayout();
        InvalidateRect(mWindowHandle, nullptr, false);
    }

    return 0;
}

auto ImpulseApp::CustomDispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    return DefWindowProcW(mWindowHandle, message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

#pragma region "Init/Cleanup"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::Init (HINSTANCE hInstance) -> bool
{
    spdlog::info("Initializing Impulse");

    // Calculate start position.
    const auto width  = 400;
    const auto height = 280;
    const auto padding = 20;
    
    auto monitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
    auto info    = MONITORINFO{ 0 };
    info.cbSize = sizeof(MONITORINFO);
    
    if (!GetMonitorInfoW(monitor, &info))
    {
        spdlog::error("GetMonitorInfoW() failed");
        return false;
    }

    const auto monitorWidth  = info.rcWork.right - info.rcWork.left;
    const auto monitorHeight = info.rcWork.bottom - info.rcWork.top;

    const auto x = monitorWidth - width - padding;
    const auto y = monitorHeight - height - padding;

#if !defined(_DEBUG)
    const auto style = WS_POPUP | WS_BORDER;
#else
    const auto style = WS_SYSMENU | WS_OVERLAPPED;
#endif

    const auto r = InternalCreate(L"Impulse", style, 0, x, y, width, height, nullptr, hInstance);
    if (!r)
    {
        spdlog::error("Initialization failed");
        return false;
    }

    ShowWindow(mWindowHandle, SW_SHOW);
    spdlog::info("Initialization finished");
    return true;
}

auto ImpulseApp::Release () -> void
{
    InternalCleanup();
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

} // namespace Impulse
