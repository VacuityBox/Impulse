#include "PCH.hpp"
#include "Impulse.hpp"

#include "Utility.hpp"

namespace Impulse {

#pragma region "Creating/Discard Resources"

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

        hr = CreateButtons();
        hr = CreateTimer();
        hr = CreateStaticText();
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
}

auto ImpulseApp::CalculateLayout () -> void
{
    if (mRenderTarget)
    {
    }
}

#pragma endregion

#pragma region "Creating Widgets"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::CreateButtons () -> HRESULT
{
    const auto rt           = mRenderTarget->GetSize();
    const auto buttonWidth  = 32.0f;
    const auto buttonHeight = buttonWidth;
    const auto padding      = 5.0f;

    auto desc     = Button::Desc();
    desc.size     = D2D1::SizeF(buttonWidth, buttonHeight);
    desc.fontSize = 16.0f;

    desc.defaultTextColor     = D2D1::ColorF(D2D1::ColorF::Black);
    desc.defaultOutlineColor  = D2D1::ColorF(D2D1::ColorF::Black);
    desc.hoverTextColor       = D2D1::ColorF(D2D1::ColorF::Blue);
    desc.hoverOutlineColor    = D2D1::ColorF(D2D1::ColorF::Blue);
    desc.activeTextColor      = D2D1::ColorF(D2D1::ColorF::Cyan);
    desc.activeOutlineColor   = D2D1::ColorF(D2D1::ColorF::Cyan);
    desc.disabledTextColor    = D2D1::ColorF(D2D1::ColorF::Gray);
    desc.disabledOutlineColor = D2D1::ColorF(D2D1::ColorF::Gray);

    {
        desc.text     = L"⚙️";
        desc.position = D2D1::Point2F(padding, padding);

        mButtonSettings = Button::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get());
    }

    {
        desc.text     = L"❌";
        desc.position = D2D1::Point2F(rt.width - buttonWidth - padding, padding);

        mButtonClose = Button::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get());
    }

    {
        desc.text     = L"▶️";
        desc.position = D2D1::Point2F(padding, rt.height - buttonHeight - padding);

        mButtonPause = Button::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get());
    }

    {
        desc.text     = L"ℹ️";
        desc.position = D2D1::Point2F(rt.width - buttonWidth - padding, rt.height - buttonHeight - padding);

        mButtonInfo = Button::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get());
    }

    mButtonSettings->OnClick = [&]{ MessageBoxW(mWindowHandle, L"Hello", L"Test", MB_OK); };

    return S_OK;
}

auto ImpulseApp::CreateTimer () -> HRESULT
{
    const auto rt      = mRenderTarget->GetSize();
    const auto padding = 45.0f;
    const auto radius  = (std::min(rt.width, rt.height) / 2) - padding;

    auto desc = Timer::Desc();

    desc.center      = D2D1::Point2F(rt.width/2, rt.height/2);
    desc.outerRadius = radius;
    desc.innerRadius = radius - 25.0f;
    desc.outerStroke = 2.5f;
    desc.innerStroke = 2.5f;
    
    desc.outerCircleColor  = D2D1::ColorF(0.40f, 0.63f, 1.0f);
    desc.outerOutlineColor = D2D1::ColorF(D2D1::ColorF::Black);
    desc.innerCircleColor  = D2D1::ColorF(0.68f, 0.81f, 1.0f);
    desc.innerOutlineColor = D2D1::ColorF(D2D1::ColorF::Black);

    desc.timerTextDesc.fontSize  = 40.0f;
    desc.topTextDesc.fontSize    = 16.0f;
    desc.bottomTextDesc.fontSize = 16.0f;

    mTimerWidget = Timer::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get());

    mTimerWidget->Duration(25 * 60);
    mTimerWidget->Start();

    return S_OK;
}

auto ImpulseApp::CreateStaticText () -> HRESULT
{
    const auto rt = mRenderTarget->GetSize();
    
    auto desc = StaticText::Desc();

    desc.fontWeight = DWRITE_FONT_WEIGHT_BOLD;

    {
        desc.text     = L"<Impulse State>";
        desc.position = D2D1::Point2F(42.0f, 5.0f);
        desc.size     = D2D1::SizeF((rt.width - 42.0f) - desc.position.x, 32.0f - desc.position.y);
        desc.fontSize = 24.0f;

        mStaticImpulseState = StaticText::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get());
    }

    {
        desc.text     = L"<Current Task>";
        desc.position = D2D1::Point2F(42.0f, rt.height - 32.0f);
        desc.size     = D2D1::SizeF((rt.width - 42.0f) - desc.position.x, rt.height - 5.0f - desc.position.y);
        desc.fontSize = 20.0f;
        
        mStaticCurrentTask = StaticText::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get());
    }

    return 0;
}

#pragma endregion

#pragma region "Drawing"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::DrawButtons () -> void
{
    auto rt = mRenderTarget.Get();

    if (mButtonClose)    { mButtonClose->Draw(rt); }
    if (mButtonSettings) { mButtonSettings->Draw(rt); }
    if (mButtonPause)    { mButtonPause->Draw(rt); }
    if (mButtonInfo)     { mButtonInfo->Draw(rt); }
}

auto ImpulseApp::DrawTimer () -> void
{
    if (mTimerWidget)
    {
        mTimerWidget->Draw(mRenderTarget.Get());
    }
}

auto ImpulseApp::DrawStaticTexts () -> void
{
    if (mStaticImpulseState)
    {
        mStaticImpulseState->Draw(mRenderTarget.Get());
    }

    if (mStaticCurrentTask)
    {
        mStaticCurrentTask->Draw(mRenderTarget.Get());
    }
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

#pragma region "HitTest"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::HitTest (D2D_POINT_2F point) -> Widget*
{
    if (mButtonClose->HitTest(point))
    {
        return mButtonClose.get();
    }
    if (mButtonSettings->HitTest(point))
    {
        return mButtonSettings.get();
    }
    if (mButtonPause->HitTest(point))
    {
        return mButtonPause.get();
    }
    if (mButtonInfo->HitTest(point))
    {
        return mButtonInfo.get();
    }
    
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

#pragma region "Window Messages"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::OnCreate () -> LRESULT
{
    // Create D2D1 Factory.
    {
        spdlog::debug("Creating D2D1 Factory");

    #if !defined(_DEBUG)
        const auto options = D2D1_FACTORY_OPTIONS{ };
    #else
        const auto options = D2D1_FACTORY_OPTIONS{ D2D1_DEBUG_LEVEL_INFORMATION };
    #endif

        auto hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            options,
            mD2DFactory.GetAddressOf()
        );
        if (FAILED(hr))
        {
            spdlog::error("D2D1CreateFactory() failed: {}", HResultToString(hr));
            return -1;  // fail CreateWindowEx()
        }

        spdlog::debug("Successfully created D2D Factory");
    }

    // Create DWriteFactory.
    {
        spdlog::debug("Creating DWriteFactory");
        auto hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(mDWriteFactory),
            reinterpret_cast<IUnknown**>(mDWriteFactory.GetAddressOf())
        );
        if (FAILED(hr))
        {
            spdlog::error("DWriteCreateFactory() failed: {}", HResultToString(hr));
            return -1;  // fail CreateWindowEx()
        }
        spdlog::debug("Successfully created DWriteFactory");
    }

    return 0;
}

auto ImpulseApp::OnDestroy () -> LRESULT
{
    DiscardGraphicsResources();
    mD2DFactory.Reset();
    mDWriteFactory.Reset();

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
        mRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        DrawButtons();
        DrawTimer();
        DrawStaticTexts();

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

auto ImpulseApp::OnTimer () -> LRESULT
{
    if (mTimerWidget->Running())
    {
        mTimerWidget->Tick();
        Redraw();
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
    const auto width  = 450;
    const auto height = 330;
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
