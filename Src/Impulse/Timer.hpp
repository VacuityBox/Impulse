#pragma once

#include "Utility.hpp"
#include "Widget.hpp"

#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
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
    D2D1_POINT_2F mCenter      = D2D1_POINT_2F{0};
    float         mOuterRadius = float{0};
    float         mInnerRadius = float{0};
    uint64_t      mDuration    = 0;               // in seconds
    bool          mPaused      = true;

    ComPtr<IDWriteTextFormat>    mTextFormat;

    ComPtr<ID2D1SolidColorBrush> mTextBrush;
    ComPtr<ID2D1SolidColorBrush> mOuterBrush;
    ComPtr<ID2D1SolidColorBrush> mOuterOutlineBrush;
    ComPtr<ID2D1SolidColorBrush> mInnerBrush;
    ComPtr<ID2D1SolidColorBrush> mInnerOutlineBrush;

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

    const auto State () const { return mState; }
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

    virtual auto HitTest (D2D_POINT_2F point)  -> bool override;
    virtual auto Update  (Widget::State state) -> bool override;
    virtual auto Draw    (ComPtr<ID2D1RenderTarget> pRenderTarget) -> void override;

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
        D2D_POINT_2F                 center,
        float                        outerRadius,
        float                        innerRadius,
        ComPtr<IDWriteTextFormat>    pDWriteTextFormat,
        ComPtr<ID2D1SolidColorBrush> mTextBrush,
        ComPtr<ID2D1SolidColorBrush> mOuterBrush,
        ComPtr<ID2D1SolidColorBrush> mOuterOutlineBrush,
        ComPtr<ID2D1SolidColorBrush> mInnerBrush,
        ComPtr<ID2D1SolidColorBrush> mInnerOutlineBrush
    ) -> std::unique_ptr<Timer>;
};

} // namespace Impulse
