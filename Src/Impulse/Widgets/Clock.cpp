#include "PCH.hpp"
#include "Clock.hpp"

#include "DX.hpp"

#include <spdlog/spdlog.h>

namespace Impulse::Widgets {

auto Clock::DurationToString() -> std::wstring
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

auto Clock::CreateBrushes () -> bool
{
    auto hr = S_OK;
    
    hr = mD2DDeviceContext->CreateSolidColorBrush(mOuterCircleColor , &mOuterCircleBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(mOuterOutlineColor, &mOuterOutlineBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(mInnerCircleColor , &mInnerCircleBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(mInnerOutlineColor, &mInnerOutlineBrush);
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", DX::GetErrorMessage(hr));
        return false;
    }

    return true;
}

auto Clock::CreateBrush (D2D_COLOR_F color) -> ComPtr<ID2D1SolidColorBrush>
{
    auto brush = ComPtr<ID2D1SolidColorBrush>();

    auto hr = mD2DDeviceContext->CreateSolidColorBrush(color, &brush);
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", DX::GetErrorMessage(hr));
        return nullptr;
    }

    return brush;
}

auto Clock::HitTest (D2D_POINT_2F point) -> bool
{
    const auto dx = mCenter.x - point.x;
    const auto dy = mCenter.y - point.y;
    const auto d  = std::sqrt(dx*dx + dy*dy);

    return d <= mOuterRadius;
}

auto Clock::Draw (ID2D1DeviceContext* d2dDeviceContext) -> void
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
    if (mSettings->CurrentState == ImpulseState::Paused)
    {
        mStaticTop->Draw(d2dDeviceContext);
    }

    const auto bottomText = std::wstring()
        + std::to_wstring(mSettings->WorkShiftCount)
        + L"/"
        + std::to_wstring(mSettings->LongBreakAfter)
        ;
    mStaticBottom->Text(bottomText);
    mStaticBottom->Draw(d2dDeviceContext);

    const auto text = DurationToString();
    mStaticTimer->Text(text);
    mStaticTimer->Draw(d2dDeviceContext);
}

auto Clock::Create (
    const Clock::Desc&        desc,
    ID2D1DeviceContext*       d2dDeviceContext,
    IDWriteFactory*           dwriteFactory,
    std::shared_ptr<Settings> settingsPtr,
    std::shared_ptr<Timer>    timerPtr
) -> std::unique_ptr<Clock>
{
    spdlog::debug("Creating Clock");

    if (!d2dDeviceContext)
    {
        spdlog::error("D2DDeviceContext is null");
        return nullptr;
    }

    if (!dwriteFactory)
    {
        spdlog::error("DWriteFactory is null");
        return nullptr;
    }

    auto clock = std::make_unique<Clock>();

    clock->mD2DDeviceContext = d2dDeviceContext;
    clock->mDWriteFactory    = dwriteFactory;

    clock->mSettings = settingsPtr;
    clock->mTimer    = timerPtr;

    clock->mCenter      = desc.center;
    clock->mOuterRadius = desc.outerRadius;
    clock->mInnerRadius = desc.innerRadius;
    clock->mOuterStroke = desc.outerStroke;
    clock->mInnerStroke = desc.innerStroke;

    clock->mOuterCircleColor    = desc.outerCircleColor;
    clock->mOuterOutlineColor   = desc.outerOutlineColor;
    clock->mInnerCircleColor   = desc.innerCircleColor;
    clock->mInnerOutlineColor  = desc.innerOutlineColor;

    // Create Clock Text.
    {
        clock->mStaticTimer = StaticText::Create(desc.timerTextDesc, d2dDeviceContext, dwriteFactory);
        if (!clock->mStaticTimer)
        {
            spdlog::error("Failed to create clock text");
            return nullptr;
        }
    }

    // Create Top Text.
    {
        clock->mStaticTop = StaticText::Create(desc.topTextDesc, d2dDeviceContext, dwriteFactory);
        if (!clock->mStaticTop)
        {
            spdlog::error("Failed to create top text");
            return nullptr;
        }
    }

    // Create Bottom Text.
    {
        clock->mStaticBottom = StaticText::Create(desc.bottomTextDesc, d2dDeviceContext, dwriteFactory);
        if (!clock->mStaticBottom)
        {
            spdlog::error("Failed to create bottom text");
            return nullptr;
        }
    }

    // Create Brushes.
    if (!clock->CreateBrushes())
    {
        return nullptr;
    }

    spdlog::debug("Clock created");

    return clock;
}

} // namespace Impulse::Widgets
