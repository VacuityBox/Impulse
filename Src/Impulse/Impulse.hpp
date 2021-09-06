#pragma once

#include "D2DApp.hpp"
#include "Settings.hpp"
#include "Timer.hpp"

#include "Widgets/Button.hpp"
#include "Widgets/StaticText.hpp"
#include "Widgets/Timer.hpp"

#include <filesystem>

namespace {
    using namespace Impulse::Widgets;
    namespace fs = std::filesystem;
}

namespace Impulse
{

class ImpulseApp : public D2DApp
{
    static constexpr auto WM_IMPULSE_REDRAW            = WM_USER + 1;
    static constexpr auto WM_IMPULSE_START_WORK_SHIFT  = WM_USER + 2;
    static constexpr auto WM_IMPULSE_START_LONG_BREAK  = WM_USER + 3;
    static constexpr auto WM_IMPULSE_START_SHORT_BREAK = WM_USER + 4;

    bool                                  mInitialzied = false;

    std::unique_ptr<Widgets::Button>      mButtonClose;
    std::unique_ptr<Widgets::Button>      mButtonSettings;
    std::unique_ptr<Widgets::Button>      mButtonPause;
    std::unique_ptr<Widgets::Button>      mButtonInfo;
    std::unique_ptr<Widgets::Timer>       mTimerWidget;
    std::unique_ptr<Widgets::StaticText>  mStaticImpulseState;
    std::unique_ptr<Widgets::StaticText>  mStaticCurrentTask;

    Widget*                               mClickedWidget = nullptr;
    Widget*                               mHoveredWidget = nullptr;
                                          
    fs::path                              mSettingsFilePath;
    std::shared_ptr<Impulse::Settings>    mSettings;      
    std::shared_ptr<Impulse::Timer>       mTimerClock;

    auto CreateGraphicsResources  () -> bool;    
    auto DiscardGraphicsResources () -> void;

    auto CalculateLayout          () -> void;
    
    // Create methods.
    auto CreateButtons    () -> bool;
    auto CreateTimer      () -> bool;
    auto CreateStaticText () -> bool;

    // Draw methods.
    auto DrawButtons     () -> void;
    auto DrawTimer       () -> void;
    auto DrawStaticTexts () -> void;

    // Get widget that is hit by @point.
    auto HitTest (D2D_POINT_2F point) -> Widget*;

    // Events (NOTE: Timer events are called from other thread).
    auto Timer_Tick           () -> void;
    auto Timer_Timeout        () -> void;
    auto ButtonClose_Click    () -> void;
    auto ButtonSettings_Click () -> void;
    auto ButtonPause_Click    () -> void;
    auto ButtonInfo_Click     () -> void;

    // Update.
    auto UpdatePauseButton () -> void;
    auto UpdateTaskStatic  () -> void;
    auto UpdateStateStatic () -> void;

    // Manipulate state.
    auto ImpulseStartWork       () -> void;
    auto ImpulseStartShortBreak () -> void;
    auto ImpulseStartLongBreak  () -> void;
    auto ImpulsePauseClock      () -> void;
    auto ImpulseInactive        () -> void;

    // Settings.
    auto LoadSettings () -> bool;
    auto SaveSettings () -> bool;

    // Window callbacks.
    virtual auto OnClose      () -> void;
    virtual auto OnDpiChanged (float dpi) -> void;

    virtual auto OnMouseDown  (MouseButton button, int x, int y) -> void;
    virtual auto OnMouseUp    (MouseButton button, int x, int y) -> void;
    virtual auto OnMouseMove  (int x, int y)                     -> void;

    virtual auto OnDraw       () -> void;
    virtual auto OnResize     (UINT32 width, UINT32 height) -> void;

    virtual auto CustomMessageHandler (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT;

public:
    ImpulseApp ()
        : mSettings   (std::make_shared<Impulse::Settings>())
        , mTimerClock (std::make_shared<Impulse::Timer>())
    {
        auto appData = GetAppDataPath() / L"Impulse";
        fs::create_directory(appData);
    
        mSettingsFilePath = appData / "Impulse.json";
    }
    ~ImpulseApp () = default;

    auto Init    (HINSTANCE hInstance) -> bool;
    auto Release () -> void;
};

} // namespace Impulse
