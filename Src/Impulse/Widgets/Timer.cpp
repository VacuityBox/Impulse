#include "PCH.hpp"
#include "Timer.hpp"

#include "Utility.hpp"

#include <spdlog/spdlog.h>

namespace Impulse::Widgets {

auto Timer::DurationToString() -> std::wstring
{
    const auto original = std::chrono::duration_cast<std::chrono::seconds>(mTimer->Duration());
    const auto negative = original < std::chrono::seconds(0);
    const auto duration = negative ? std::chrono::abs(original) : original;

    const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(original).count();
    const auto seconds = original % 60;
    auto str = std::wstring();

    if (negative)
    {
        str += L"-";
    }

    if (minutes < 10)
    {
        str += L"0";
    }
    str += std::to_wstring(minutes);
    
    str += L":";

    if (seconds < std::chrono::seconds(10))
    {
        str += L"0";
    }
    str += std::to_wstring(seconds.count());
    
    return std::move(str);
}

auto Timer::HitTest (D2D_POINT_2F point) -> bool
{
    const auto dx = mCenter.x - point.x;
    const auto dy = mCenter.y - point.y;
    const auto d  = std::sqrt(dx*dx + dy*dy);

    return d <= mOuterRadius;
}

auto Timer::Draw (ID2D1DeviceContext* d2dDeviceContext) -> void
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
        d2dDeviceContext->DrawEllipse(outer, pOuterOutlineBrush, mOuterStroke);
        d2dDeviceContext->FillEllipse(outer, pOuterBrush);
        d2dDeviceContext->DrawEllipse(inner, pInnerOutlineBrush, mInnerStroke);
        d2dDeviceContext->FillEllipse(inner, pInnerBrush);
    };


    draw(
        mOuterCircleBrush.Get(),
        mOuterOutlineBrush.Get(),
        mInnerCircleBrush.Get(),
        mInnerOutlineBrush.Get()
    );

    // Draw Texts.
    if (mTimer->IsPaused())
    {
        mStaticTop->Draw(d2dDeviceContext);
    }

    const auto bottomText = 
        std::to_wstring(mSettings->WorkShiftCount) +
        L"/" +
        std::to_wstring(mSettings->LongBreakAfter)
        ;
    mStaticBottom->Text(bottomText);
    mStaticBottom->Draw(d2dDeviceContext);

    const auto text = DurationToString();
    mStaticTimer->Text(text);
    mStaticTimer->Draw(d2dDeviceContext);
}

auto Timer::Create (
    const Timer::Desc&                 desc,
    ID2D1DeviceContext*                d2dDeviceContext,
    IDWriteFactory*                    dwriteFactory,
    std::shared_ptr<Impulse::Settings> settingsPtr,
    std::shared_ptr<Impulse::Timer>    timerPtr
) -> std::unique_ptr<Timer>
{
    spdlog::debug("Creating Timer");

    if (!d2dDeviceContext)
    {
        spdlog::error("d2dDeviceContext is null");
        return nullptr;
    }

    if (!dwriteFactory)
    {
        spdlog::error("dwriteFactory is null");
        return nullptr;
    }

    auto hr    = S_OK;
    auto timer = std::make_unique<Timer>();

    timer->mSettings = settingsPtr;
    timer->mTimer    = timerPtr;

    timer->mCenter      = desc.center;
    timer->mOuterRadius = desc.outerRadius;
    timer->mInnerRadius = desc.innerRadius;
    timer->mOuterStroke = desc.outerStroke;
    timer->mInnerStroke = desc.innerStroke;

    // Create Timer Text.
    auto rect   = timer->Rect();
    auto width  = rect.right - rect.left;
    auto height = rect.bottom - rect.top;
    {
        timer->mStaticTimer = StaticText::Create(desc.timerTextDesc, d2dDeviceContext, dwriteFactory);
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
        timer->mStaticTop = StaticText::Create(desc.topTextDesc, d2dDeviceContext, dwriteFactory);
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
        timer->mStaticBottom = StaticText::Create(desc.bottomTextDesc, d2dDeviceContext, dwriteFactory);
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
        return d2dDeviceContext->CreateSolidColorBrush(color, ppBrush);
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

} // namespace Impulse::Widgets
