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
        hr = CreateButtons();
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

    const auto buttonDefaultTextColor     = D2D1::ColorF(0.5f, 0.0f, 0.0f);
    const auto buttonDefaultOutlineColor  = D2D1::ColorF(0.5f, 0.0f, 0.0f);
    const auto buttonHoverTextColor       = D2D1::ColorF(0.7f, 0.7f, 0.0f);
    const auto buttonHoverOutlineColor    = D2D1::ColorF(0.7f, 0.7f, 0.0f);
    const auto buttonActiveTextColor      = D2D1::ColorF(0.9f, 0.9f, 0.9f);
    const auto buttonActiveOutlineColor   = D2D1::ColorF(0.9f, 0.9f, 0.9f);
    const auto buttonFocusTextColor       = D2D1::ColorF(0.5f, 0.0f, 0.0f);
    const auto buttonFocusOutlineColor    = D2D1::ColorF(0.5f, 0.0f, 0.0f);
    const auto buttonDisabledTextColor    = D2D1::ColorF(0.2f, 0.2f, 0.2f);
    const auto buttonDisabledOutlineColor = D2D1::ColorF(0.2f, 0.2f, 0.2f);
    
    hr = mRenderTarget->CreateSolidColorBrush(backgroundColor, mBackgroundBrush.GetAddressOf());

    hr = mRenderTarget->CreateSolidColorBrush(buttonDefaultTextColor    , mDefaultTextBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonDefaultOutlineColor , mDefaultOutlineBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonHoverTextColor      , mHoverTextBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonHoverOutlineColor   , mHoverOutlineBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonActiveTextColor     , mActiveTextBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonActiveOutlineColor  , mActiveOutlineBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonFocusTextColor      , mFocusTextBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonFocusOutlineColor   , mFocusOutlineBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonDisabledTextColor   , mDisabledTextBrush.GetAddressOf());
    hr = mRenderTarget->CreateSolidColorBrush(buttonDisabledOutlineColor, mDisabledOutlineBrush.GetAddressOf());

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
            mButtonTextFormat.GetAddressOf()
        );
        if (FAILED(hr))
        {
            spdlog::error("CreateTextFormat() failed: {}", HResultToString(hr));
            return hr;
        }

        mButtonTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        mButtonTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

        spdlog::debug("Successfully created Button Font");
    }
    
    return S_OK;
}

auto ImpulseApp::CreateButtons () -> HRESULT
{
    const auto rt           = mRenderTarget->GetSize();
    const auto buttonWidth  = 32.0f;
    const auto buttonHeight = buttonWidth;
    const auto padding      = 5.0f;
    const auto buttonSize   = D2D1::SizeF(buttonWidth, buttonHeight);

    auto create = [&](const wchar_t* text, D2D1_POINT_2F position, D2D1_SIZE_F size)
    {
        return Button::Create(
            text,
            position,
            size,
            mButtonTextFormat,
            mDefaultTextBrush,
            mDefaultOutlineBrush,
            mHoverTextBrush,
            mHoverOutlineBrush,
            mActiveTextBrush,
            mActiveOutlineBrush,
            mFocusTextBrush,
            mFocusOutlineBrush,
            mDisabledTextBrush,
            mDisabledOutlineBrush
        );
    };

    mSettingsButton = create(L"O", D2D1::Point2F(padding, padding), buttonSize);
    mCloseButton = create(L"X", D2D1::Point2F(rt.width - buttonWidth - padding, padding), buttonSize);
    mPauseButton = create(L"||", D2D1::Point2F(padding, rt.height - buttonHeight - padding), buttonSize);

    mSettingsButton->OnClick = [&]{ MessageBoxW(mWindowHandle, L"Hello", L"Test", MB_OK); };

    return S_OK;
}

auto ImpulseApp::CalculateLayout () -> void
{
    if (mRenderTarget)
    {
    }
}

#pragma endregion

#pragma region "Drawing"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::DrawButtons () -> void
{
    mCloseButton->Draw(mRenderTarget);
    mSettingsButton->Draw(mRenderTarget);
    mPauseButton->Draw(mRenderTarget);
}

auto ImpulseApp::HitTest (D2D_POINT_2F point) -> Widget*
{
    if (mCloseButton->HitTest(point))
    {
        return mCloseButton.get();
    }
    if (mSettingsButton->HitTest(point))
    {
        return mSettingsButton.get();
    }
    if (mPauseButton->HitTest(point))
    {
        return mPauseButton.get();
    }
    
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

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

        DrawButtons();

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

auto ImpulseApp::OnMouseMove (int x, int y, DWORD flags) -> LRESULT
{
    const auto point = D2D1::Point2F(x, y);

    auto widget = HitTest(point);
    if (widget)
    {
        if (widget == mClickedWidget)
        {
            if (widget->Update(Widget::State::Active))
            {
                Redraw();
            }
        }
        else
        {
            if (widget->Update(Widget::State::Hover))
            {
                Redraw();
            }
        }
        mHoveredWidget = widget;
    }
    else
    {
        if (mHoveredWidget)
        {
            if (mHoveredWidget->Update(Widget::State::Default))
            {
                Redraw();
            }
            mHoveredWidget = nullptr;
        }
    }

    return 0;
}

auto ImpulseApp::OnLeftMouseButtonUp (int x, int y, DWORD flags) -> LRESULT 
{
    const auto point = D2D1::Point2F(x, y);

    auto widget = HitTest(point);
    if (widget)
    {
        if (widget == mClickedWidget)
        {
            widget->OnClick();
        }

        if (widget->Update(Widget::State::Hover))
        {
            Redraw();
        }

    }
    
    mClickedWidget = nullptr;
    
    return 0;
}

auto ImpulseApp::OnLeftMouseButtonDown (int x, int y, DWORD flags) -> LRESULT 
{
    auto widget = HitTest(D2D1::Point2F(x, y));
    if (widget)
    {
        mClickedWidget = widget;
        if (widget->Update(Widget::State::Active))
        {
            Redraw();
        }
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
