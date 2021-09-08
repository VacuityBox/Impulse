#pragma once

#include "Timer.hpp"
#include "Settings.hpp"
#include "StaticText.hpp"
#include "Widget.hpp"

#include <memory>
#include <string>

#include <d2d1_1.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse::Widgets {

class Clock : public Widget
{
public:
    struct Desc
    {
        D2D1_POINT_2F    center             = D2D1::Point2F();
        float            outerRadius        = 0.0f;
        float            innerRadius        = 0.0f;
        float            outerStroke        = 1.0f;
        float            innerStroke        = 1.0f;
        
        D2D1_COLOR_F     outerCircleColor   = D2D1::ColorF(D2D1::ColorF::Black);
        D2D1_COLOR_F     outerOutlineColor  = D2D1::ColorF(D2D1::ColorF::Black);
        D2D1_COLOR_F     innerCircleColor   = D2D1::ColorF(D2D1::ColorF::Black);
        D2D1_COLOR_F     innerOutlineColor  = D2D1::ColorF(D2D1::ColorF::Black);
        
        StaticText::Desc timerTextDesc      = StaticText::Desc();
        StaticText::Desc topTextDesc        = StaticText::Desc();
        StaticText::Desc bottomTextDesc     = StaticText::Desc();
    };

private:
    D2D1_POINT_2F                mCenter             = D2D1::Point2F();
    float                        mOuterRadius        = 0.0f;
    float                        mInnerRadius        = 0.0f;
    float                        mOuterStroke        = 1.0f;
    float                        mInnerStroke        = 1.0f;

    D2D1_COLOR_F                 mOuterCircleColor   = {0};
    D2D1_COLOR_F                 mOuterOutlineColor  = {0};
    D2D1_COLOR_F                 mInnerCircleColor   = {0};
    D2D1_COLOR_F                 mInnerOutlineColor  = {0};

    ComPtr<ID2D1SolidColorBrush> mOuterCircleBrush;
    ComPtr<ID2D1SolidColorBrush> mOuterOutlineBrush;
    ComPtr<ID2D1SolidColorBrush> mInnerCircleBrush;
    ComPtr<ID2D1SolidColorBrush> mInnerOutlineBrush;

    ID2D1DeviceContext*          mD2DDeviceContext   = nullptr;
    IDWriteFactory*              mDWriteFactory      = nullptr;

    std::unique_ptr<StaticText>  mStaticTimer;
    std::unique_ptr<StaticText>  mStaticTop;
    std::unique_ptr<StaticText>  mStaticBottom;

    std::shared_ptr<Settings>    mSettings;
    std::shared_ptr<Timer>       mTimer;

private:
    auto DurationToString() -> std::wstring;

    auto CreateBrushes () -> bool;
    auto CreateBrush   (D2D_COLOR_F color) -> ComPtr<ID2D1SolidColorBrush>;

public:
    Clock  () = default;
    ~Clock () = default;

    auto Position    (float x, float y)  { mCenter.x = x; mCenter.y = y; }
    auto OuterRadius (float r)           { mOuterRadius = r; }
    auto InnerRadius (float r)           { mInnerRadius = r; }
    auto OuterStroke (float s)           { mOuterStroke = s; }
    auto InnerStroke (float s)           { mInnerStroke = s; }

    auto GetTopStatic    () { return mStaticTop.get(); }
    auto GetTimerStatic  () { return mStaticTimer.get(); }
    auto GetBottomStatic () { return mStaticBottom.get(); }

    const auto Rect  () const
    {
        return D2D1::RectF(
            mCenter.x - mOuterRadius,
            mCenter.y - mOuterRadius,
            mCenter.x + mOuterRadius,
            mCenter.y + mOuterRadius
        );
    }

    const auto OuterEllipse () const { return D2D1::Ellipse(mCenter, mOuterRadius, mOuterRadius); }
    const auto InnerEllipse () const { return D2D1::Ellipse(mCenter, mInnerRadius, mInnerRadius); }

    virtual auto HitTest (D2D_POINT_2F point)                   -> bool override;    
    virtual auto Draw    (ID2D1DeviceContext* d2dDeviceContext) -> void override;

    static auto Create (
        const Clock::Desc&        desc,
        ID2D1DeviceContext*       d2dDeviceContext,
        IDWriteFactory*           dwriteFactory,
        std::shared_ptr<Settings> settingsPtr,
        std::shared_ptr<Timer>    timerPtr
    ) -> std::unique_ptr<Clock>;
};

} // namespace Impulse::Widgets
