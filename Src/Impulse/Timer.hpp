#pragma once

#include "StaticText.hpp"
#include "Widget.hpp"

#include <memory>
#include <string>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse {

class Timer : public Widget
{
public:
    struct Desc
    {
        D2D1_POINT_2F    center             = D2D1::Point2F();
        float            outerRadius        = 0.0f;
        float            innerRadius        = 0.0f;
        uint64_t         duration           = 0;                // in seconds
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
    D2D1_POINT_2F mCenter      = D2D1::Point2F();
    float         mOuterRadius = 0.0f;
    float         mInnerRadius = 0.0f;
    float         mOuterStroke = 1.0f;
    float         mInnerStroke = 1.0f;
    uint64_t      mDuration    = 0;                // in seconds
    bool          mPaused      = true;

    ComPtr<IDWriteTextFormat>    mTimerTextFormat;
    ComPtr<IDWriteTextFormat>    mTopTextFormat;
    ComPtr<IDWriteTextFormat>    mBottomTextFormat;

    ComPtr<ID2D1SolidColorBrush> mOuterCircleBrush;
    ComPtr<ID2D1SolidColorBrush> mOuterOutlineBrush;
    ComPtr<ID2D1SolidColorBrush> mInnerCircleBrush;
    ComPtr<ID2D1SolidColorBrush> mInnerOutlineBrush;

    std::unique_ptr<StaticText>  mStaticTimer;
    std::unique_ptr<StaticText>  mStaticTop;
    std::unique_ptr<StaticText>  mStaticBottom;

public:
    std::function<void ()> OnTimeout = []{};

private:
    auto DurationToString() -> std::wstring;

public:
    Timer  () = default;
    ~Timer () = default;

    auto Position    (float x, float y)  { mCenter.x = x; mCenter.y = y; }
    auto OuterRadius (float r)           { mOuterRadius = r; }
    auto InnerRadius (float r)           { mInnerRadius = r; }

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

    virtual auto HitTest (D2D_POINT_2F point)               -> bool override;    
    virtual auto Draw    (ID2D1RenderTarget* pRenderTarget) -> void override;

    auto Tick ()
    {
        if (!mPaused)
        {
            mDuration -= 1;
        }
    }

    auto Duration (uint64_t duration) { mDuration = duration; }
    auto Duration ()                  { return mDuration; }

    auto Start   () { mPaused = false; }
    auto Pause   () { mPaused = true; }
    auto Running () { return !mPaused; }
    auto Paused  () { return mPaused; }

    static auto Create (
        const Timer::Desc& desc,
        ID2D1RenderTarget* pRenderTarget,
        IDWriteFactory*    pDWriteFactory
    ) -> std::unique_ptr<Timer>;
};

} // namespace Impulse
