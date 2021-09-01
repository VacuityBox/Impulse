#include "PCH.hpp"
#include "Button.hpp"

#include "Utility.hpp"

#include <string>
#include <spdlog/spdlog.h>
#include <d2d1effects.h>

namespace Impulse {

auto Button::FontSize (float size, IDWriteFactory* pDWriteFactory) -> bool
{   
    //auto hr = pDWriteFactory->CreateTextLayout(
    //    mText.c_str(),
    //    mText.length(),
    //    mTextFormat.Get(),
    //    mSize.width,
    //    mSize.height,
    //    &mTextLayout
    //);
    //if (FAILED(hr))
    //{
    //    spdlog::error("CreateTextLayout() failed: {}", HResultToString(hr));
    //    return false;
    //}
    //
    //mTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    //mTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    mTextLayout->SetFontSize(size, DWRITE_TEXT_RANGE{0, static_cast<unsigned int>(mText.length())});
    mFontSize = size;

    return true;
}

auto Button::HitTest (D2D_POINT_2F point) -> bool
{
    const auto rect = Rect();

    return (rect.left <= point.x && point.x <= rect.right)
        && (rect.top  <= point.y && point.y <= rect.bottom)
        ;;
}

auto Button::Draw (ID2D1DeviceContext* d2dDeviceContext) -> void
{
    auto draw = [&](ComPtr<ID2D1SolidColorBrush> textBrush, ComPtr<ID2D1SolidColorBrush> outlineBrush)
    {
        if (mForceOutline || (mIntelOutline && (mState == State::Hover || mState == State::Active)))
        {
            if (mRoundedCorners)
            {
                auto rr = D2D1::RoundedRect(Rect(), 4.0f, 4.0f);
                d2dDeviceContext->DrawRoundedRectangle(rr, outlineBrush.Get());
            }
            else
            {
                d2dDeviceContext->DrawRectangle(Rect(), outlineBrush.Get());
            }
        }

        //pRenderTarget->DrawTextW(
        //    mText.c_str(),
        //    mText.length(),
        //    mTextFormat.Get(),
        //    Rect(),
        //    textBrush.Get(),
        //    D2D1_DRAW_TEXT_OPTIONS_CLIP | D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
        //);
        d2dDeviceContext->DrawTextLayout(
            mPosition,
            mTextLayout.Get(),
            textBrush.Get(),
            D2D1_DRAW_TEXT_OPTIONS_CLIP | D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
        );
    };

    switch (mState)
    {
    case State::Default:
        draw(mDefaultTextBrush, mDefaultOutlineBrush);
        break;

    case State::Hover:
        draw(mHoverTextBrush, mHoverOutlineBrush);
        break;

    case State::Active:
        draw(mActiveTextBrush, mActiveOutlineBrush);
        break;

    case State::Focus:
        draw(mFocusTextBrush, mFocusOutlineBrush);
        break;

    case State::Disabled:
        draw(mDisabledTextBrush, mDisabledOutlineBrush);
        break;
    }
}

auto Button::Create (
    const Button::Desc& desc,
    ID2D1DeviceContext* d2dDeviceContext,
    IDWriteFactory*     dwriteFactory
) -> std::unique_ptr<Button>
{
    spdlog::debug("Creating button");

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

    auto hr     = S_OK;
    auto button = Button();

    button.mPosition = desc.position;
    button.mSize     = desc.size;
    button.mText     = desc.text;
    button.mFontSize = desc.fontSize;

    hr = dwriteFactory->CreateTextFormat(
        desc.font,
        nullptr,
        desc.fontWeight,
        desc.fontStyle,
        desc.fontStretch,
        desc.fontSize,
        L"",
        button.mTextFormat.GetAddressOf()
    );
    if (FAILED(hr))
    {
        spdlog::error("CreateTextFormat() failed: {}", HResultToString(hr));
        return nullptr;
    }

    button.mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    button.mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    hr = dwriteFactory->CreateTextLayout(
        button.mText.c_str(),
        button.mText.length(),
        button.mTextFormat.Get(),
        button.mSize.width,
        button.mSize.height,
        button.mTextLayout.GetAddressOf()
    );
    if (FAILED(hr))
    {
        spdlog::error("CreateTextLayout() failed: {}", HResultToString(hr));
        return nullptr;
    }

    auto createBrush = [&](const D2D_COLOR_F& color, ID2D1SolidColorBrush** ppBrush)
    {
        return d2dDeviceContext->CreateSolidColorBrush(color, ppBrush);
    };

    hr = createBrush(desc.defaultTextColor    , button.mDefaultTextBrush    .GetAddressOf());
    hr = createBrush(desc.defaultOutlineColor , button.mDefaultOutlineBrush .GetAddressOf());
    hr = createBrush(desc.hoverTextColor      , button.mHoverTextBrush      .GetAddressOf());
    hr = createBrush(desc.hoverOutlineColor   , button.mHoverOutlineBrush   .GetAddressOf());
    hr = createBrush(desc.activeTextColor     , button.mActiveTextBrush     .GetAddressOf());
    hr = createBrush(desc.activeOutlineColor  , button.mActiveOutlineBrush  .GetAddressOf());
    hr = createBrush(desc.focusTextColor      , button.mFocusTextBrush      .GetAddressOf());
    hr = createBrush(desc.focusOutlineColor   , button.mFocusOutlineBrush   .GetAddressOf());
    hr = createBrush(desc.disabledTextColor   , button.mDisabledTextBrush   .GetAddressOf());
    hr = createBrush(desc.disabledOutlineColor, button.mDisabledOutlineBrush.GetAddressOf());
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", HResultToString(hr));
        return nullptr;
    }

    button.mForceOutline   = desc.forceOutline;
    button.mIntelOutline   = desc.intelOutline;
    button.mRoundedCorners = desc.roundedCorners;
    button.mRoundedRadius  = desc.roundedRadius;

    spdlog::debug("Button created");

    return std::make_unique<Button>(std::move(button));
}

} // namespace Impulse
