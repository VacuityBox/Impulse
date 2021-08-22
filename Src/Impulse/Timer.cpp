#include "PCH.hpp"
#include "Timer.hpp"

#include <cmath>

namespace Impulse {

auto Timer::DurationToString() -> std::wstring
{
    const auto minutes = mDuration / 60;
    const auto seconds = mDuration % 60;
    auto str = std::wstring();

    if (minutes < 10)
    {
        str += L"0";
    }
    str += std::to_wstring(minutes);
    
    str += L":";

    if (seconds < 10)
    {
        str += L"0";
    }
    str += std::to_wstring(seconds);

    return std::move(str);
}

auto Timer::HitTest (D2D_POINT_2F point) -> bool
{
    const auto dx = mCenter.x - point.x;
    const auto dy = mCenter.y - point.y;
    const auto d  = std::sqrt(dx*dx + dy*dy);

    return d <= mOuterRadius;
}

auto Timer::Update (Widget::State state) -> bool
{
    if (mState == state)
    {
        return false;
    }

    mState = state;

    return true;
}

auto Timer::Draw (ComPtr<ID2D1RenderTarget> pRenderTarget) -> void
{
    // Draw Ellipse.
    const auto outer = OuterEllipse();
    const auto inner = InnerEllipse();

    auto draw = [&](
        ComPtr<ID2D1SolidColorBrush> pOuterBrush,
        ComPtr<ID2D1SolidColorBrush> pOuterOutlineBrush,
        ComPtr<ID2D1SolidColorBrush> pInnerBrush,
        ComPtr<ID2D1SolidColorBrush> pInnerOutlineBrush
        )
    {
        pRenderTarget->DrawEllipse(outer, pOuterOutlineBrush.Get(), 2.5f);
        pRenderTarget->FillEllipse(outer, pOuterBrush.Get());
        pRenderTarget->DrawEllipse(inner, pInnerOutlineBrush.Get(), 2.5f);
        pRenderTarget->FillEllipse(inner, pInnerBrush.Get());
    };


    draw(mOuterBrush, mOuterOutlineBrush, mInnerBrush, mInnerOutlineBrush);

    // Draw Timer.
    const auto text = DurationToString();
    pRenderTarget->DrawTextW(
        text.c_str(), text.length(), mTextFormat.Get(), Rect(), mTextBrush.Get()
    );
}

auto Timer::Create (
        D2D_POINT_2F                 center,
        float                        outerRadius,
        float                        innerRadius,
        ComPtr<IDWriteTextFormat>    pDWriteTextFormat,
        ComPtr<ID2D1SolidColorBrush> pTextBrush,
        ComPtr<ID2D1SolidColorBrush> pOuterBrush,
        ComPtr<ID2D1SolidColorBrush> pOuterOutlineBrush,
        ComPtr<ID2D1SolidColorBrush> pInnerBrush,
        ComPtr<ID2D1SolidColorBrush> pInnerOutlineBrush
) -> std::unique_ptr<Timer>
{
    auto timer = Timer();

    timer.mCenter      = center;
    timer.mOuterRadius = outerRadius;
    timer.mInnerRadius = innerRadius;

    timer.mDuration = 0;
    timer.mPaused   = true;

    timer.mTextFormat = pDWriteTextFormat;
    timer.mTextBrush  = pTextBrush;

    timer.mOuterBrush        = pOuterBrush;
    timer.mOuterOutlineBrush = pOuterOutlineBrush;
    timer.mInnerBrush        = pInnerBrush;
    timer.mInnerOutlineBrush = pInnerOutlineBrush;

    return std::make_unique<Timer>(std::move(timer));
}

} // namespace Impulse
