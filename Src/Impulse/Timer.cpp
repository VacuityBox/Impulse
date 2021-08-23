#include "PCH.hpp"
#include "Timer.hpp"

#include "Utility.hpp"

#include <cmath>
#include <spdlog/spdlog.h>

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

auto Timer::Draw (ID2D1RenderTarget* pRenderTarget) -> void
{
    // Draw Ellipse.
    const auto outer = OuterEllipse();
    const auto inner = InnerEllipse();

    auto draw = [&](
        ID2D1SolidColorBrush* pOuterBrush,
        ID2D1SolidColorBrush* pOuterOutlineBrush,
        ID2D1SolidColorBrush* pInnerBrush,
        ID2D1SolidColorBrush* pInnerOutlineBrush
        )
    {
        pRenderTarget->DrawEllipse(outer, pOuterOutlineBrush, mOuterStroke);
        pRenderTarget->FillEllipse(outer, pOuterBrush);
        pRenderTarget->DrawEllipse(inner, pInnerOutlineBrush, mInnerStroke);
        pRenderTarget->FillEllipse(inner, pInnerBrush);
    };


    draw(
        mOuterCircleBrush.Get(),
        mOuterOutlineBrush.Get(),
        mInnerCircleBrush.Get(),
        mInnerOutlineBrush.Get()
    );

    // Draw Texts.
    mStaticTop->Draw(pRenderTarget);
    mStaticBottom->Draw(pRenderTarget);

    const auto text = DurationToString();
    mStaticTimer->Text(text);
    mStaticTimer->Draw(pRenderTarget);
}

auto Timer::Create (
    const Timer::Desc& desc,
    ID2D1RenderTarget* pRenderTarget,
    IDWriteFactory*    pDWriteFactory
) -> std::unique_ptr<Timer>
{
    spdlog::debug("Creating Timer");

    if (!pRenderTarget)
    {
        spdlog::error("pRenderTarget is null");
        return nullptr;
    }

    if (!pDWriteFactory)
    {
        spdlog::error("pDWriteFactory is null");
        return nullptr;
    }

    auto hr    = S_OK;
    auto timer = std::make_unique<Timer>();

    timer->mCenter      = desc.center;
    timer->mOuterRadius = desc.outerRadius;
    timer->mInnerRadius = desc.innerRadius;

    // Create Timer Text.
    auto rect   = timer->Rect();
    auto width  = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    {
        timer->mStaticTimer = StaticText::Create(desc.timerTextDesc, pRenderTarget, pDWriteFactory);
        if (!timer->mStaticTimer)
        {
            spdlog::error("Failed to create timer text");
            return nullptr;
        }
        timer->mStaticTimer->Position(rect.left, rect.top);
        timer->mStaticTimer->Size(width, height);
    }

    // Create Top Text.
    {
        timer->mStaticTop = StaticText::Create(desc.topTextDesc, pRenderTarget, pDWriteFactory);
        if (!timer->mStaticTop)
        {
            spdlog::error("Failed to create top text");
            return nullptr;
        }
        timer->mStaticTop->Position(
            rect.left,
            desc.center.y - 32.0f - 32.0f);
        timer->mStaticTop->Size(width, 32.0f);
        timer->mStaticTop->Text(L"(paused)");
    }

    // Create Bottom Text.
    {
        timer->mStaticBottom = StaticText::Create(desc.bottomTextDesc, pRenderTarget, pDWriteFactory);
        if (!timer->mStaticBottom)
        {
            spdlog::error("Failed to create bottom text");
            return nullptr;
        }
        timer->mStaticBottom->Position(desc.center.x, desc.center.y + 15.0f);
        timer->mStaticBottom->Position(
            rect.left,
            desc.center.y + 32.0f);
        timer->mStaticBottom->Size(width, 32.0f);
        timer->mStaticBottom->Text(L"1/4");
    }

    auto createBrush = [&](const D2D_COLOR_F& color, ID2D1SolidColorBrush** ppBrush)
    {
        return pRenderTarget->CreateSolidColorBrush(color, ppBrush);
    };

    hr = createBrush(desc.outerCircleColor , timer->mOuterCircleBrush .GetAddressOf());
    hr = createBrush(desc.outerOutlineColor, timer->mOuterOutlineBrush.GetAddressOf());
    hr = createBrush(desc.innerCircleColor , timer->mInnerCircleBrush .GetAddressOf());
    hr = createBrush(desc.innerOutlineColor, timer->mInnerOutlineBrush.GetAddressOf());
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", HResultToString(hr));
        return nullptr;
    }

    spdlog::debug("Timer created");

    return timer;
}

} // namespace Impulse
