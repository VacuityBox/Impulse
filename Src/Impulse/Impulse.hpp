#pragma once

#include "Button.hpp"
#include "StaticText.hpp"
#include "Settings.hpp"
#include "Timer.hpp"
#include "Window.hpp"

#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;

    namespace fs = std::filesystem;
}

namespace Impulse
{

class ImpulseApp : public Window
{
    ComPtr<ID2D1Factory>          mD2DFactory;
    ComPtr<IDWriteFactory>        mDWriteFactory;
    ComPtr<ID2D1HwndRenderTarget> mRenderTarget;

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

    auto CreateGraphicsResources  () -> HRESULT;
    auto DiscardGraphicsResources () -> void;
    auto CalculateLayout          () -> void;
    auto Redraw                   () -> void { InvalidateRect(mWindowHandle, nullptr, false); }

    // Create methods.
    auto CreateButtons    () -> HRESULT;
    auto CreateTimer      () -> HRESULT;
    auto CreateStaticText () -> HRESULT;

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

    // Inherited via Window
    virtual auto OnCreate  () -> LRESULT override;
    virtual auto OnDestroy () -> LRESULT override;
    virtual auto OnCommand (WPARAM, LPARAM) -> LRESULT override { return 0; }
    virtual auto OnPaint   (WPARAM, LPARAM) -> LRESULT override;
    virtual auto OnResize  (WPARAM, LPARAM) -> LRESULT override;

    virtual auto OnKeyUp   (UINT) -> LRESULT override { return 0; }
    virtual auto OnKeyDown (UINT) -> LRESULT override { return 0; }

    virtual auto OnMouseMove            (int, int, DWORD) -> LRESULT override;
    virtual auto OnLeftMouseButtonUp    (int, int, DWORD) -> LRESULT override;
    virtual auto OnLeftMouseButtonDown  (int, int, DWORD) -> LRESULT override;
    virtual auto OnRightMouseButtonUp   (int, int, DWORD) -> LRESULT override { return 0; }
    virtual auto OnRightMouseButtonDown (int, int, DWORD) -> LRESULT override { return 0; }

    virtual auto OnTimer () -> LRESULT override;

    virtual auto CustomDispatch (UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT override;

    virtual auto ClassName () const -> const wchar_t* override { return L"ImpulseApp_WndClass"; }

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
