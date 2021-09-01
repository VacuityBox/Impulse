#pragma once

#include "D2DApp.hpp"
#include "Button.hpp"
#include "StaticText.hpp"
#include "Settings.hpp"
#include "Timer.hpp"

#include <filesystem>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
    namespace fs = std::filesystem;
}

namespace Impulse
{

class ImpulseApp : public D2DApp
{
    bool                          mInitialzied = false;

    std::unique_ptr<Button>       mButtonClose;
    std::unique_ptr<Button>       mButtonSettings;
    std::unique_ptr<Button>       mButtonPause;
    std::unique_ptr<Button>       mButtonInfo;
    std::unique_ptr<Timer>        mTimerWidget;
    std::unique_ptr<StaticText>   mStaticImpulseState;
    std::unique_ptr<StaticText>   mStaticCurrentTask;

    Widget*                       mClickedWidget = nullptr;
    Widget*                       mHoveredWidget = nullptr;

    fs::path                      mSettingsFilePath;
    std::shared_ptr<Settings>     mSettings;      

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

    // Events.
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
    auto ImpulseWork       () -> void;
    auto ImpulseShortBreak () -> void;
    auto ImpulseLongBreak  () -> void;
    auto ImpulsePause      () -> void;
    auto ImpulseInactive   () -> void;

    // Settings.
    auto LoadSettings () -> bool;
    auto SaveSettings () -> bool;

    // Window callbacks.
    virtual auto OnClose     () -> void;
    virtual auto OnDpiChange (float dpi) -> void;

    virtual auto OnMouseDown  (MouseButton button, int x, int y) -> void;
    virtual auto OnMouseUp    (MouseButton button, int x, int y) -> void;
    virtual auto OnMouseMove  (int x, int y)                     -> void;

    virtual auto OnDraw       () -> void;

    virtual auto OnResize (UINT32 width, UINT32 height) -> void
    {
        D2DApp::OnResize(width, height);

        CalculateLayout();
        Redraw();

        Draw();
    }

public:
    ImpulseApp ()
        : mSettings (std::make_shared<Settings>())
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
