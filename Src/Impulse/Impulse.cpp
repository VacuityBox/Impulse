#include "PCH.hpp"
#include "Impulse.hpp"

#include "Utility.hpp"

#include <chrono>
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
) -> std::tuple<D2D1_POINT_2L, D2D1_SIZE_U>
{
    // Get primary monitor.
    auto monitor = MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY);
    auto info    = MONITORINFO{ 0 };
    info.cbSize  = sizeof(MONITORINFO);
    
    if (!GetMonitorInfoW(monitor, &info))
    {
        spdlog::error("GetMonitorInfoW() failed");
        return std::tuple(
            D2D1::Point2L(originalPadding, originalPadding),
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

    auto includeTaskbar = false;

    auto abd   = APPBARDATA{0};
    abd.cbSize = sizeof(PAPPBARDATA);
    if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd))
    {
        // Detect window position basing on taskbar position.
        if (windowPosition == Impulse::WindowPosition::Auto)
        {
            switch (abd.uEdge)
            {
            case ABE_LEFT:
                windowPosition = Impulse::WindowPosition::LeftBottom;
                break;
            
            case ABE_TOP:
                windowPosition = Impulse::WindowPosition::RightTop;
                break;

            case ABE_RIGHT:
                windowPosition = Impulse::WindowPosition::RightBottom;
                break;
            
            case ABE_BOTTOM:
                windowPosition = Impulse::WindowPosition::RightBottom;
                break;
            }
        }

        includeTaskbar = true;
    }
    else
    {
        spdlog::warn("SHAppBarMessage() failed, ignoring taskbar when positioning window");
    }

    switch (windowPosition)
    {
    case Impulse::WindowPosition::Auto:
        // Fallback to right-bottom.
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

    if (includeTaskbar)
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
    
    return std::tuple(D2D1::Point2L(x, y), D2D1::SizeU(windowWidth, windowHeight));
}

}

namespace Impulse {

#pragma region "Create/Discard Resources"

auto ImpulseApp::CreateGraphicsResources () -> bool
{
    spdlog::debug("Creating Graphics Resources");

    if (!mInitialzied)
    {
        auto r = false;
        r = CreateButtons();
        r = CreateTimer();
        r = CreateStaticText();
        if (r)
        {
            return false;
        }

        CalculateLayout();

        UpdatePauseButton();
        UpdateStateStatic();
        UpdateTaskStatic();
    }

    spdlog::debug("Successfully created Graphics Resources");

    return true;
}

auto ImpulseApp::DiscardGraphicsResources () -> void
{
    spdlog::debug("Discarding Graphics Resources");

    mStaticImpulseState.reset();
    mStaticCurrentTask.reset();

    mTimerWidget.reset();

    mButtonClose.reset();
    mButtonSettings.reset();
    mButtonPause.reset();
    mButtonInfo.reset();

    mHoveredWidget = nullptr;
    mClickedWidget = nullptr;
}

auto ImpulseApp::CalculateLayout () -> void
{
    if (!mD2DDeviceContext || !mInitialzied)
    {
        return;
    }

    auto dpi = GetDpiForWindow(Handle());
    if (dpi == 0)
    {
        spdlog::warn("GetDpiForWindow() failed, using default value (96)");
        dpi = 96;
    }

    const auto scale = dpi / 96.f;

    const auto originalWidth  = 450.f;
    const auto originalHeight = 330.f;
    const auto windowPadding  = 20.f;

    auto [position, size] = CalculateWindowPositionAndSize(
        mSettings->WindowPosition, originalWidth, originalHeight, windowPadding, dpi
    );

#if defined(NDEBUG)
    SetWindowPos(
        Handle(), HWND_TOP, position.x, position.y, size.width, size.height, SWP_NOACTIVATE
    );
#endif

    const auto rt = mD2DDeviceContext->GetSize();

    const auto buttonWidth    = ceil(32.f * scale);
    const auto buttonHeight   = ceil(32.f * scale);
    const auto buttonPaddingX = ceil(5.f * scale);
    const auto buttonPaddingY = ceil(5.f * scale);
    const auto buttonFontSize = ceil(22.f * scale);

    const auto staticTopPadding    = ceil(5.f * scale);
    const auto staticBottomPadding = ceil(5.f * scale);
    const auto staticStateFontSize = ceil(24.f * scale);
    const auto staticTaskFontSize  = ceil(20.f * scale);

    const auto timerPadding          = ceil(55.f * scale);
    const auto timerOuterRadius      = (std::min(rt.width, rt.height) / 2.f) - timerPadding;
    const auto timerInnerRadius      = timerOuterRadius - ceil(25.f * scale); // TODO use some ratio mby
    const auto timerStroke           = ceil(2.f * scale);
    const auto timerFontSize         = ceil(40.f * scale);
    const auto timerTopFontSize      = ceil(16.f * scale);
    const auto timerBottomFontSize   = ceil(16.f * scale);
    const auto timerTopTextHeight    = ceil(32.f * scale);
    const auto timerBottomTextHeight = ceil(32.f * scale);

    // Calculate button position/size.
    {
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
    }

    // Calculate timer position/size.
    {
        const auto cx = rt.width/2;
        const auto cy = rt.height/2;
        mTimerWidget->Position(cx, cy);
        mTimerWidget->OuterRadius(timerOuterRadius);
        mTimerWidget->InnerRadius(timerInnerRadius);
        mTimerWidget->OuterStroke(timerStroke);
        mTimerWidget->InnerStroke(timerStroke);

        const auto timerRect = mTimerWidget->Rect();
        const auto timerText = mTimerWidget->GetTimerStatic();
        timerText->Position(timerRect.left, timerRect.top);
        timerText->Size(2.f * timerOuterRadius, 2.f * timerOuterRadius);
        timerText->FontSize(timerFontSize, mDWriteFactory.Get());

        const auto timerTop = mTimerWidget->GetTopStatic();
        timerTop->Position(timerRect.left, cy - ceil(64.f * scale)); // TODO use ratio
        timerTop->Size(timerRect.right - timerRect.left, timerTopTextHeight);
        timerTop->FontSize(timerTopFontSize, mDWriteFactory.Get());

        const auto timerBottom = mTimerWidget->GetBottomStatic();
        timerBottom->Position(timerRect.left, cy + ceil(32.f * scale)); // TODO use ratio
        timerBottom->Size(timerRect.right - timerRect.left, timerBottomTextHeight);
        timerBottom->FontSize(timerBottomFontSize, mDWriteFactory.Get());
    }

    // Calculate static texts.
    {
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

auto ImpulseApp::CreateButtons () -> bool
{
    const auto rt           = mD2DDeviceContext->GetSize();
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

        mButtonSettings = Button::Create(desc, mD2DDeviceContext.Get(), mDWriteFactory.Get());
    }

    {
        desc.text     = L"❌";
        desc.position = D2D1::Point2F(rt.width - buttonWidth - padding, padding);

        mButtonClose = Button::Create(desc, mD2DDeviceContext.Get(), mDWriteFactory.Get());
    }

    {
        desc.text       = L"▶️";
        desc.position   = D2D1::Point2F(padding, rt.height - buttonHeight - padding);

        mButtonPause = Button::Create(desc, mD2DDeviceContext.Get(), mDWriteFactory.Get());
    }

    {
        desc.text       = L"🛈";
        desc.position   = D2D1::Point2F(rt.width - buttonWidth - padding, rt.height - buttonHeight - padding);

        mButtonInfo = Button::Create(desc, mD2DDeviceContext.Get(), mDWriteFactory.Get());
    }

    mButtonSettings->OnClick = [&]{ ButtonSettings_Click(); };
    mButtonClose->OnClick = [&]{ ButtonClose_Click(); };
    mButtonPause->OnClick = [&]{ ButtonPause_Click(); };
    mButtonInfo->OnClick = [&]{ ButtonInfo_Click(); };

    return true;
}

auto ImpulseApp::CreateTimer () -> bool
{
    const auto rt      = mD2DDeviceContext->GetSize();
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

    mTimerWidget = Timer::Create(desc, mD2DDeviceContext.Get(), mDWriteFactory.Get(), mSettings);

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

    return true;
}

auto ImpulseApp::CreateStaticText () -> bool
{
    const auto rt = mD2DDeviceContext->GetSize();
    
    auto desc = StaticText::Desc();

    desc.fontWeight = DWRITE_FONT_WEIGHT_BOLD;

    {
        desc.text     = L"<Impulse State>";
        desc.position = D2D1::Point2F(42.0f, 5.0f);
        desc.size     = D2D1::SizeF((rt.width - 42.0f) - desc.position.x, 32.0f - desc.position.y);
        desc.fontSize = 24.0f;

        mStaticImpulseState = StaticText::Create(desc, mD2DDeviceContext.Get(), mDWriteFactory.Get());
    }

    {
        desc.text     = L"<Current Task>";
        desc.position = D2D1::Point2F(42.0f, rt.height - 32.0f);
        desc.size     = D2D1::SizeF((rt.width - 42.0f) - desc.position.x, rt.height - 5.0f - desc.position.y);
        desc.fontSize = 20.0f;
        
        mStaticCurrentTask = StaticText::Create(desc, mD2DDeviceContext.Get(), mDWriteFactory.Get());
    }

    return true;
}

#pragma endregion

#pragma region "Drawing"

////////////////////////////////////////////////////////////////////////////////

auto ImpulseApp::DrawButtons () -> void
{
    auto d2dDeviceContext = mD2DDeviceContext.Get();

    if (mButtonClose)    { mButtonClose->Draw(d2dDeviceContext); }
    if (mButtonSettings) { mButtonSettings->Draw(d2dDeviceContext); }
    if (mButtonPause)    { mButtonPause->Draw(d2dDeviceContext); }
    if (mButtonInfo)     { mButtonInfo->Draw(d2dDeviceContext); }
}

auto ImpulseApp::DrawTimer () -> void
{
    if (mTimerWidget)
    {
        mTimerWidget->Draw(mD2DDeviceContext.Get());
    }
}

auto ImpulseApp::DrawStaticTexts () -> void
{
    if (mStaticImpulseState)
    {
        mStaticImpulseState->Draw(mD2DDeviceContext.Get());
    }

    if (mStaticCurrentTask)
    {
        mStaticCurrentTask->Draw(mD2DDeviceContext.Get());
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
    DestroyWindow(Handle());
    Quit();
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
        mButtonPause->Text(L"▶️", mDWriteFactory.Get());
        mSettings->PreviousState = mSettings->CurrentState;
        mSettings->CurrentState = ImpulseState::Paused;
        break;

    case ImpulseState::Inactive:
    case ImpulseState::Paused:
        mTimerWidget->Start();
        mButtonPause->Text(L"⏸", mDWriteFactory.Get());
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
        mButtonPause->Text(L"⏸️", mDWriteFactory.Get());
        break;

    case ImpulseState::Inactive:
    case ImpulseState::Paused:
        mButtonPause->Text(L"▶️", mDWriteFactory.Get());
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

auto ImpulseApp::OnClose () -> void
{
    DestroyWindow(Handle());
    Quit();
}

auto ImpulseApp::OnDpiChange (float dpi) -> void
{
    auto [position, size] = CalculateWindowPositionAndSize(
            mSettings->WindowPosition, 450.f, 330.f, 20.f, dpi
        );

    SetWindowPos(
        Handle(), HWND_TOP, position.x, position.y, size.width, size.height, SWP_NOACTIVATE
    );

    CalculateLayout();
}

auto ImpulseApp::OnMouseDown (MouseButton button, int x, int y) -> void
{
    if (button == MouseButton::Left)
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
    }
}

auto ImpulseApp::OnMouseUp (MouseButton button, int x, int y) -> void
{
    if (button == MouseButton::Left)
    {
        const auto point = D2D1::Point2F(x, y);

        auto widget = HitTest(point);
        if (widget)
        {
            if (widget->Update(Widget::State::Hover))
            {
                Redraw();
            }
        
            if (widget == mClickedWidget)
            {
                widget->OnClick();
            }
        }
        if (mClickedWidget)
        {

        }
    
        mClickedWidget = nullptr;
    }
}

auto ImpulseApp::OnMouseMove (int x, int y) -> void
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
}

auto ImpulseApp::OnDraw () -> void
{
    static float frames = 0.f;

    DrawButtons();
    DrawTimer();
    DrawStaticTexts();

    SetWindowTextW(Handle(), std::to_wstring(frames).c_str());
    frames++;    
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

#pragma region Init/Cleanup/Loop

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

    auto wndDesc           = Window::Desc{0};
    wndDesc.title          = L"Impulse";
    wndDesc.className      = L"Impulse_WndClass";
    wndDesc.exstyle        = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    wndDesc.invisible      = false;
    wndDesc.position       = position;
    wndDesc.size           = size;
    wndDesc.instanceHandle = hInstance;
    wndDesc.parentHandle   = nullptr;

#if defined(NDEBUG)
    wndDesc.style = WS_POPUP | WS_BORDER;
#else
    wndDesc.style = WS_SYSMENU | WS_OVERLAPPED | WS_SIZEBOX;
#endif

    auto d2dDesc = D2DApp::Desc{0};
    d2dDesc.windowDesc = wndDesc;

    const auto r = D2DApp::Init(d2dDesc);
    if (!r)
    {
        spdlog::error("Initialization failed");
        return false;
    }

    CreateGraphicsResources();

    mInitialzied = true;
    
    spdlog::info("Initialization finished");

    return true;
}

auto ImpulseApp::Release () -> void
{
    SaveSettings();
}

////////////////////////////////////////////////////////////////////////////////

#pragma endregion

} // namespace Impulse
