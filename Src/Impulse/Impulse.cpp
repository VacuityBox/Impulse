#include "PCH.hpp"
#include "Impulse.hpp"

#include "Utility.hpp"

#include <fstream>
#include <tuple>

#include <shellapi.h>

namespace {

auto CalculateWindowPositionAndSize (
    Impulse::WindowPosition windowPosition,
    float originalWidth,
    float originalHeight,
    float originalPadding,
    float dpi = 96.f
) -> std::tuple<D2D1_POINT_2U, D2D1_SIZE_U>
{
    // Get primary monitor.
    auto monitor = MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
    auto info    = MONITORINFO{ 0 };
    info.cbSize  = sizeof(MONITORINFO);
    
    if (!GetMonitorInfoW(monitor, &info))
    {
        spdlog::error("GetMonitorInfoW() failed");
        return std::tuple(
            D2D1::Point2U(originalPadding, originalPadding),
            D2D1::SizeU(originalWidth, originalHeight)
        );
    }

    // Re-calculate with dpi in mind.
    const auto windowWidth   = ceil(originalWidth * dpi / 96.f);
    const auto windowHeight  = ceil(originalHeight * dpi / 96.f);
    const auto windowPadding = ceil(originalPadding * dpi / 96.f);
    
    const auto monitorWidth  = info.rcMonitor.right - info.rcMonitor.left;
    const auto monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;

    auto x = monitorWidth - windowWidth - windowPadding;
    auto y = monitorHeight - windowHeight - windowPadding;

    switch (windowPosition)
    {
    case Impulse::WindowPosition::Auto:
        // TODO: bottom right for now
        x = monitorWidth - windowWidth - windowPadding;
        y = monitorHeight - windowHeight - windowPadding;
        break;
    case Impulse::WindowPosition::LeftTop:
        x = windowPadding;
        y = windowPadding;
        break;
    case Impulse::WindowPosition::LeftEdge:
        x = windowPadding;
        y = (monitorHeight - windowHeight) / 2;
        break;
    case Impulse::WindowPosition::LeftBottom:
        x = windowPadding;
        y = monitorHeight - windowHeight - windowPadding;
        break;
    case Impulse::WindowPosition::CenterTop:
        x = (monitorWidth - windowWidth) / 2;
        y = windowPadding;
        break;
    case Impulse::WindowPosition::Center:
        x = (monitorWidth - windowWidth) / 2;
        y = (monitorHeight - windowHeight) / 2;
        break;
    case Impulse::WindowPosition::CenterBottom:
        x = (monitorWidth - windowWidth) / 2;
        y = monitorHeight - windowHeight - windowPadding;
        break;
    case Impulse::WindowPosition::RightTop:
        x = monitorWidth - windowWidth - windowPadding;
        y = windowPadding;
        break;
    case Impulse::WindowPosition::RightEdge:
        x = monitorWidth - windowWidth - windowPadding;
        y = (monitorHeight - windowHeight) / 2;
        break;
    case Impulse::WindowPosition::RightBottom:
        x = monitorWidth - windowWidth - windowPadding;
        y = monitorHeight - windowHeight - windowPadding;
        break;
    }

    auto abd = APPBARDATA{0};
    abd.cbSize = sizeof(PAPPBARDATA);

    if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd))
    {
        const auto dx = abd.rc.right - abd.rc.left;
        const auto dy = abd.rc.bottom - abd.rc.top;

        switch (abd.uEdge)
        {
        case ABE_LEFT:
            if (windowPosition == Impulse::WindowPosition::LeftTop
            ||  windowPosition == Impulse::WindowPosition::LeftEdge
            ||  windowPosition == Impulse::WindowPosition::LeftBottom
               )
            {
                x += dx;
            }
            break;
            
        case ABE_TOP:
            if (windowPosition == Impulse::WindowPosition::LeftTop
            ||  windowPosition == Impulse::WindowPosition::CenterTop
            ||  windowPosition == Impulse::WindowPosition::RightTop
               )
            {
                y += dy;
            }
            break;

        case ABE_RIGHT:
            if (windowPosition == Impulse::WindowPosition::RightTop
            ||  windowPosition == Impulse::WindowPosition::RightEdge
            ||  windowPosition == Impulse::WindowPosition::RightBottom
               )
            {
                x -= dx;
            }
            break;
            
        case ABE_BOTTOM:
            if (windowPosition == Impulse::WindowPosition::LeftBottom
            ||  windowPosition == Impulse::WindowPosition::CenterBottom
            ||  windowPosition == Impulse::WindowPosition::RightBottom
               )
            {
                y -= dy;
            }
            break;
        }
    }
    else
    {
        spdlog::warn("SHAppBarMessage() failed, ignoring taskbar when positioning window");
    }
    
    return std::tuple(D2D1::Point2U(x, y), D2D1::SizeU(windowWidth, windowHeight));
}

}

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

        UpdatePauseButton();
        UpdateStateStatic();
        UpdateTaskStatic();

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
    auto dpi = GetDpiForWindow(mWindowHandle);
    if (dpi == 0)
    {
        spdlog::warn("GetDpiForWindow() failed, using default value (96)");
        dpi = 96;
    }

    auto [position, size] = CalculateWindowPositionAndSize(
        mSettings->WindowPosition, 450.f, 330.f, 20.f, dpi
    );

#if defined(NDEBUG)
    SetWindowPos(
        mWindowHandle, HWND_TOP, position.x, position.y, size.width, size.height, SWP_NOACTIVATE
    );
#endif

    const auto rt = mRenderTarget->GetSize();

    const auto buttonWidth    = ceil(32.f * dpi / 96.f);
    const auto buttonHeight   = ceil(32.f * dpi / 96.f);
    const auto buttonPaddingX = ceil(5.f * dpi / 96.f);
    const auto buttonPaddingY = ceil(5.f * dpi / 96.f);
    const auto buttonFontSize = ceil(22.f * dpi / 96.f);

    const auto staticTopPadding    = ceil(5.f * dpi / 96.f);
    const auto staticBottomPadding = ceil(5.f * dpi / 96.f);
    const auto staticStateFontSize = ceil(24.f * dpi / 96.f);
    const auto staticTaskFontSize  = ceil(20.f * dpi / 96.f);

    const auto timerPadding          = ceil(55.f * dpi / 96.f);
    const auto timerOuterRadius      = (std::min(rt.width, rt.height) / 2.f) - timerPadding;
    const auto timerInnerRadius      = timerOuterRadius - ceil(25.f * dpi / 96.f); // TODO use some ratio mby
    const auto timerStroke           = ceil(2.f * dpi / 96.f);
    const auto timerFontSize         = ceil(40.f * dpi / 96.f);
    const auto timerTopFontSize      = ceil(16.f * dpi / 96.f);
    const auto timerBottomFontSize   = ceil(16.f * dpi / 96.f);
    const auto timerTopTextHeight    = ceil(32.f * dpi / 96.f);
    const auto timerBottomTextHeight = ceil(32.f * dpi / 96.f);

    if (mRenderTarget)
    {
        // Calculate button position/size.
        const auto brx = rt.width - buttonWidth - buttonPaddingX;
        const auto bry = rt.height - buttonHeight - buttonPaddingY;

        mButtonSettings->Position(buttonPaddingX, buttonPaddingY);
        mButtonSettings->Size(buttonWidth, buttonHeight);
        mButtonSettings->FontSize(buttonFontSize, mDWriteFactory.Get());

        mButtonClose->Position(brx, buttonPaddingY);
        mButtonClose->Size(buttonWidth, buttonHeight);
        mButtonClose->FontSize(buttonFontSize, mDWriteFactory.Get());

        mButtonPause->Position(buttonPaddingX, bry);
        mButtonPause->Size(buttonWidth, buttonHeight);
        mButtonPause->FontSize(buttonFontSize, mDWriteFactory.Get());

        mButtonInfo->Position(brx, bry);
        mButtonInfo->Size(buttonWidth, buttonHeight);
        mButtonInfo->FontSize(buttonFontSize, mDWriteFactory.Get());

        // Calculate timer position/size.
        const auto cx = rt.width/2;
        const auto cy = rt.height/2;
        mTimerWidget->Position(cx, cy);
        mTimerWidget->OuterRadius(timerOuterRadius);
        mTimerWidget->InnerRadius(timerInnerRadius);
        mTimerWidget->OuterStroke(timerStroke);
        mTimerWidget->InnerStroke(timerStroke);

        auto timerRect = mTimerWidget->Rect();
        auto timerText = mTimerWidget->GetTimerStatic();
        timerText->Position(timerRect.left, timerRect.top);
        timerText->Size(2.f * timerOuterRadius, 2.f * timerOuterRadius);
        timerText->FontSize(timerFontSize, mDWriteFactory.Get());

        auto timerTop = mTimerWidget->GetTopStatic();
        timerTop->Position(timerRect.left, cy - ceil(64.f * dpi / 96.f)); // TODO use ratio
        timerTop->Size(timerRect.right - timerRect.left, timerTopTextHeight);
        timerTop->FontSize(timerTopFontSize, mDWriteFactory.Get());

        auto timerBottom = mTimerWidget->GetBottomStatic();
        timerBottom->Position(timerRect.left, cy + ceil(32.f * dpi / 96.f)); // TODO use ratio
        timerBottom->Size(timerRect.right - timerRect.left, timerBottomTextHeight);
        timerBottom->FontSize(timerBottomFontSize, mDWriteFactory.Get());

        // Calculate static texts.
        const auto slx = buttonWidth + 2 * buttonPaddingX;
        const auto slw = (rt.width - (2 * (buttonWidth + 2 * buttonPaddingX)));

        mStaticImpulseState->Position(slx, staticTopPadding);
        mStaticImpulseState->Size(slw, buttonHeight);
        mStaticImpulseState->FontSize(staticStateFontSize, mDWriteFactory.Get());

        mStaticCurrentTask->Position(slx, rt.height - staticBottomPadding - buttonHeight);
        mStaticCurrentTask->Size(slw, buttonHeight);
        mStaticCurrentTask->FontSize(staticTaskFontSize, mDWriteFactory.Get());
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
    desc.fontSize = 22.0f;

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

    mButtonSettings->OnClick = [&]{ ButtonSettings_Click(); };
    mButtonClose->OnClick = [&]{ ButtonClose_Click(); };
    mButtonPause->OnClick = [&]{ ButtonPause_Click(); };
    mButtonInfo->OnClick = [&]{ ButtonInfo_Click(); };

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

    mTimerWidget = Timer::Create(desc, mRenderTarget.Get(), mDWriteFactory.Get(), mSettings);

    mTimerWidget->OnTimeout = [&]{ Timer_Timeout(); };
    mTimerWidget->Duration(mSettings->WorkDuration);
    
    if (mSettings->AutoStartTimer)
    {
        mTimerWidget->Start();
        mSettings->CurrentState = ImpulseState::WorkShift;
    }
    else
    {
        mTimerWidget->Pause();
        mSettings->CurrentState = ImpulseState::Inactive; // on start timer is inactive
    }

    if (!InternalStartTimer())
    {
        spdlog::error("Failed to start internal timer");
        return E_ABORT;
    }

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

#pragma region Events

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::Timer_Timeout () -> void
{
    // This function gets executed when timer hit 0.
    switch (mSettings->CurrentState)
    {
    // Time for break.
    case ImpulseState::WorkShift:
        // Check if we need to start long break.
        if (mSettings->WorkShiftCount == mSettings->LongBreakAfter)
        {
            ImpulseLongBreak();
        }
        else
        {
            ImpulseShortBreak();
        }
        break;

    // Time for work.
    case ImpulseState::ShortBreak:
    case ImpulseState::LongBreak:
        ImpulseWork();

        if (mSettings->WorkShiftCount < mSettings->LongBreakAfter)
        {
            mSettings->WorkShiftCount += 1;
        }
        else
        {
            mSettings->WorkShiftCount = 1;
        }     

        break;
    }
}

auto ImpulseApp::ButtonClose_Click () -> void
{
    DestroyWindow(mWindowHandle);
}

auto ImpulseApp::ButtonSettings_Click () -> void
{

}

auto ImpulseApp::ButtonPause_Click () -> void
{
    ImpulsePause();
}

auto ImpulseApp::ButtonInfo_Click () -> void
{
}

auto ImpulseApp::ImpulseWork () -> void
{
    mTimerWidget->Duration(mSettings->WorkDuration);
    mSettings->CurrentState = ImpulseState::WorkShift;
    UpdateStateStatic();
}

auto ImpulseApp::ImpulseShortBreak () -> void
{
    mTimerWidget->Duration(mSettings->ShortBreakDuration);
    mSettings->CurrentState = ImpulseState::ShortBreak;
    UpdateStateStatic();
}

auto ImpulseApp::ImpulseLongBreak () -> void
{
    mTimerWidget->Duration(mSettings->LongBreakDuration);
    mSettings->CurrentState = ImpulseState::LongBreak;
    UpdateStateStatic();
}

auto ImpulseApp::ImpulsePause () -> void
{
    switch (mSettings->CurrentState)
    {
    case ImpulseState::WorkShift:
    case ImpulseState::LongBreak:
    case ImpulseState::ShortBreak:
        mTimerWidget->Pause();
        mButtonPause->Text(L"▶️");
        mSettings->PreviousState = mSettings->CurrentState;
        mSettings->CurrentState = ImpulseState::Paused;
        break;

    case ImpulseState::Inactive:
    case ImpulseState::Paused:
        mTimerWidget->Start();
        mButtonPause->Text(L"⏸️");
        mSettings->PreviousState = mSettings->CurrentState;
        mSettings->CurrentState = ImpulseState::WorkShift;
        break;
    }

    UpdateStateStatic();
}

auto ImpulseApp::ImpulseInactive () -> void
{
    mSettings->CurrentState = ImpulseState::Inactive;
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

#pragma region Update

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::UpdatePauseButton () -> void
{
    switch (mSettings->CurrentState)
    {
    case ImpulseState::WorkShift:
    case ImpulseState::LongBreak:
    case ImpulseState::ShortBreak:
        mButtonPause->Text(L"⏸️");
        break;

    case ImpulseState::Inactive:
    case ImpulseState::Paused:
        mButtonPause->Text(L"▶️");
        break;
    }
}

auto ImpulseApp::UpdateTaskStatic () -> void
{
    mStaticCurrentTask->Text(mSettings->TaskName);
}

auto ImpulseApp::UpdateStateStatic () -> void
{
    switch (mSettings->CurrentState)
    {
    case ImpulseState::Inactive:
        mStaticImpulseState->Text(L"");
        break;

    case ImpulseState::WorkShift:
        mStaticImpulseState->Text(L"Work Time");
        break;

    case ImpulseState::LongBreak:
        mStaticImpulseState->Text(L"Break Time");
        break;

    case ImpulseState::ShortBreak:
        mStaticImpulseState->Text(L"Break Time");
        break;

    case ImpulseState::Paused: 
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

#pragma region Settings Load/Save

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::LoadSettings () -> bool
{
    // NOTE: Settings should be in UTF-8
    // Read Settings file.
    auto file = std::ifstream(mSettingsFilePath);
    if (!file)
    {
        spdlog::error("Can't open Settings file '{}' for reading", mSettingsFilePath.string());
        return false;
    }

    // Deserialize.
    auto json = nlohmann::json::parse(file, nullptr, false, true);
    if (json.is_discarded())
    {
        spdlog::error("Failed to deserialize json");
        return false;
    }
    
    try
    {
        *mSettings = json.get<Settings>();
    }
    catch (nlohmann::json::exception&)
    {
        spdlog::error("Failed to deserialize settings");
        spdlog::info("Using default values");
        *mSettings = Settings();
        return true;
    }

    //Log() << json.dump(4).c_str() << std::endl;
    spdlog::info("Loaded Settings '{}'", mSettingsFilePath.string());

    return true;
}

auto ImpulseApp::SaveSettings () -> bool
{
    // Open Settings file.
    auto file = std::ofstream(mSettingsFilePath);
    if (!file)
    {
        spdlog::error("Can't open Settings file '{}' for writing", mSettingsFilePath.string());
        return false;
    }

    // Serialize.
    auto json = nlohmann::json(*mSettings);
    file << std::setw(4) << json;

    spdlog::info("Saved Settings '{}'", mSettingsFilePath.string());
    return true;
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
    if (!mTimerWidget)
    {
        return 0;
    }

    if (mTimerWidget->Running())
    {
        mTimerWidget->Tick();
        Redraw();
    }

    return 0;
}

auto ImpulseApp::CustomDispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (message == WM_DPICHANGED)
    {
        if (mRenderTarget != NULL)
        {
            auto dpi = GetDpiForWindow(mWindowHandle);
            if (dpi == 0)
            {
                spdlog::warn("GetDpiForWindow() failed, using default value (96)");
                dpi = 96;
            }

            spdlog::info("Dpi changed, new dpi ({})", dpi);

            auto [position, size] = CalculateWindowPositionAndSize(
                mSettings->WindowPosition, 450.f, 330.f, 20.f, dpi
            );

        //#if defined(NDEBUG)
            SetWindowPos(
                mWindowHandle, HWND_TOP, position.x, position.y, size.width, size.height, SWP_NOACTIVATE
            );
        //#endif
        }
    }

    return DefWindowProcW(mWindowHandle, message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

#pragma region "Init/Cleanup"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::Init (HINSTANCE hInstance) -> bool
{
    spdlog::info("Initializing Impulse");

    // Load Settings.
    {
        // Create default settings file if not exists.
        if (!fs::exists(mSettingsFilePath))
        {
            spdlog::info("Settings file not found, creating default one");
            SaveSettings();
        }
        else
        {
            if (!LoadSettings())
            {
                return false;
            }
        }
    }

    // Create window.
    auto [position, size] = CalculateWindowPositionAndSize(
        WindowPosition::Auto, 450.f, 330.f, 20.f
    );

    const auto ex = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
#if defined(NDEBUG)
    const auto style = WS_POPUP | WS_BORDER;
#else
    const auto style = WS_SYSMENU | WS_OVERLAPPED | WS_SIZEBOX;
#endif

    const auto r = InternalCreate(
        L"Impulse",
        style, ex,
        position.x, position.y,
        size.width, size.height,
        nullptr,
        hInstance
    );
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
    SaveSettings();
    InternalCleanup();
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

} // namespace Impulse
