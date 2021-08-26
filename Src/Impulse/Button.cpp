#include "PCH.hpp"
#include "Button.hpp"

#include "Utility.hpp"

#include <string>
#include <spdlog/spdlog.h>

namespace Impulse {

auto Button::FontSize (float size, IDWriteFactory* pDWriteFactory) -> bool
{
    auto hr         = S_OK;
    //auto length     = mTextFormat->GetFontFamilyNameLength();
    //auto familyName = std::wstring(length + 1, L'\0');
    //
    //hr = mTextFormat->GetFontFamilyName(familyName.data(), length + 1); // +1 for NULL char
    //if (FAILED(hr))
    //{
    //    spdlog::error("GetFontFamilyName() failed: {}", HResultToString(hr));
    //    return false;
    //}

    //auto newTextFormat = ComPtr<IDWriteTextFormat>();
    //hr = pDWriteFactory->CreateTextFormat(
    //    familyName.c_str(),
    //    nullptr,
    //    mTextFormat->GetFontWeight(),
    //    mTextFormat->GetFontStyle(),
    //    mTextFormat->GetFontStretch(),
    //    size,
    //    L"",
    //    newTextFormat.GetAddressOf()
    //);
    //if (FAILED(hr))
    //{
    //    spdlog::error("CreateTextFormat() failed: {}", HResultToString(hr));
    //    return false;
    //}

    //mTextFormat.Reset();
    //mTextFormat = std::move(newTextFormat);

    //mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    //mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    hr = pDWriteFactory->CreateTextLayout(
        mText.c_str(),
        mText.length(),
        mTextFormat.Get(),
        mSize.width,
        mSize.height,
        mTextLayout.GetAddressOf()
    );
    if (FAILED(hr))
    {
        spdlog::error("CreateTextLayout() failed: {}", HResultToString(hr));
        return false;
    }

    mTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    mTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    return true;
}

auto Button::HitTest (D2D_POINT_2F point) -> bool
{
    const auto rect = Rect();

    return (rect.left <= point.x && point.x <= rect.right)
        && (rect.top  <= point.y && point.y <= rect.bottom)
        ;;
}

auto Button::Draw (ID2D1RenderTarget* pRenderTarget) -> void
{


    auto draw = [&](ComPtr<ID2D1SolidColorBrush> textBrush, ComPtr<ID2D1SolidColorBrush> outlineBrush)
    {
        if (mButtonOutline)
        {
            if (mRoundedCorners)
            {
                auto rr = D2D1::RoundedRect(Rect(), 4.0f, 4.0f);
                pRenderTarget->DrawRoundedRectangle(rr, outlineBrush.Get());
            }
            else
            {
                pRenderTarget->DrawRectangle(Rect(), outlineBrush.Get());
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
        pRenderTarget->DrawTextLayout(
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
    ID2D1RenderTarget*  pRenderTarget,
    IDWriteFactory*     pDWriteFactory
) -> std::unique_ptr<Button>
{
    spdlog::debug("Creating button");

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

    auto hr     = S_OK;
    auto button = Button();

    button.mPosition = desc.position;
    button.mSize     = desc.size;
    button.mText     = desc.text;

    hr = pDWriteFactory->CreateTextFormat(
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

    hr = pDWriteFactory->CreateTextLayout(
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
        return pRenderTarget->CreateSolidColorBrush(color, ppBrush);
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

    button.mButtonOutline  = desc.buttonOutline;
    button.mRoundedCorners = desc.roundedCorners;
    button.mRoundedRadius  = desc.roundedRadius;

    spdlog::debug("Button created");

    return std::make_unique<Button>(std::move(button));
}

} // namespace Impulse
