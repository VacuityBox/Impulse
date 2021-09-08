#include "PCH.hpp"
#include "Button.hpp"
#include "DX.hpp"
#include "Resource.h"
#include "Utility.hpp"

#include <spdlog/spdlog.h>
#include <Shlwapi.h>

namespace Impulse::Widgets {

auto Button::CreateBrushes (const Button::Desc& desc) -> bool
{
    auto hr = S_OK;

    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.defaultTextColor    , &mDefaultTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.defaultOutlineColor , &mDefaultOutlineBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.hoverTextColor      , &mHoverTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.hoverOutlineColor   , &mHoverOutlineBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.activeTextColor     , &mActiveTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.activeOutlineColor  , &mActiveOutlineBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.focusTextColor      , &mFocusTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.focusOutlineColor   , &mFocusOutlineBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.disabledTextColor   , &mDisabledTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.disabledOutlineColor, &mDisabledOutlineBrush);
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", DX::GetErrorMessage(hr));
        return false;
    }

    return true;
}

auto Button::CreateTextFormats (const Button::Desc& desc) -> bool
{
    auto hr = mDWriteFactory->CreateTextFormat(
        desc.fontName,
        nullptr,
        desc.fontWeight,
        desc.fontStyle,
        desc.fontStretch,
        desc.fontSize,
        L"",
        &mTextFormat
    );
    if (FAILED(hr))
    {
        spdlog::error("CreateTextFormat() failed: {}", DX::GetErrorMessage(hr));
        return false;
    }

    mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    return true;
}

auto Button::CrateSvg (const Button::Desc& desc) -> bool
{
    auto d2ddc5 = static_cast<ID2D1DeviceContext5*>(mD2DDeviceContext);
    if (!d2ddc5)
    {
        return false;
    }

    //auto stream = ComPtr<IStream>();
    //hr = SHCreateStreamOnFileW(desc.svg, STGM_READ, &stream);
    //if (FAILED(hr))
    //{
    //    spdlog::error("SHCreateStreamOnFileEx() failed: {}", DX::GetErrorMessage(hr));
    //    return false;
    //}
    if (desc.svg0 != 0)
    {
        const auto data = LoadSvgFromResource(desc.svg0);
        if (data.empty())
        {
            spdlog::error("Failed to load svg from resource");
            return false;
        }

        auto stream = ComPtr<IStream>();
        stream = SHCreateMemStream(reinterpret_cast<const BYTE*>(data.c_str()), data.size());
        if (!stream)
        {
            spdlog::error("SHCreateMemStream() failed");
            return false;
        }

        const auto viewport = D2D1::SizeF(32.f, 32.f);
        const auto hr = d2ddc5->CreateSvgDocument(stream.Get(), viewport, &mSvgIcon0);
        if (FAILED(hr))
        {
            spdlog::error("CreateSvgDocument() failed: {}", DX::GetErrorMessage(hr));
            return false;
        }
    }

    if (desc.svg1 != 0)
    {
        const auto data = LoadSvgFromResource(desc.svg1);
        if (data.empty())
        {
            spdlog::error("Failed to load svg from resource");
            return false;
        }

        auto stream = ComPtr<IStream>();
        stream = SHCreateMemStream(reinterpret_cast<const BYTE*>(data.c_str()), data.size());
        if (!stream)
        {
            spdlog::error("SHCreateMemStream() failed");
            return false;
        }

        const auto viewport = D2D1::SizeF(32.f, 32.f);
        const auto hr = d2ddc5->CreateSvgDocument(stream.Get(), viewport, &mSvgIcon1);
        if (FAILED(hr))
        {
            spdlog::error("CreateSvgDocument() failed: {}", DX::GetErrorMessage(hr));
            return false;
        }
    }

    return true;
}

auto Button::CreateTextFormat (
    const WCHAR*        fontName,
    FLOAT               fontSize,
    DWRITE_FONT_WEIGHT  fontWeight,
    DWRITE_FONT_STYLE   fontStyle,
    DWRITE_FONT_STRETCH fontStretch
) -> ComPtr<IDWriteTextFormat>
{
    auto textFormat = ComPtr<IDWriteTextFormat>();

    auto hr = mDWriteFactory->CreateTextFormat(
        fontName,
        nullptr,
        fontWeight,
        fontStyle,
        fontStretch,
        fontSize,
        L"",
        &textFormat
    );
    if (FAILED(hr))
    {
        spdlog::error("CreateTextFormat() failed: {}", DX::GetErrorMessage(hr));
        return nullptr;
    }

    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    return textFormat;
}

auto Button::CreateBrush (D2D_COLOR_F color) -> ComPtr<ID2D1SolidColorBrush>
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
                auto rr = D2D1::RoundedRect(Rect(), mRoundedRadius, mRoundedRadius);
                d2dDeviceContext->DrawRoundedRectangle(rr, outlineBrush.Get());
            }
            else
            {
                d2dDeviceContext->DrawRectangle(Rect(), outlineBrush.Get());
            }
        }

        if (!mSvgIcon0)
        {
            d2dDeviceContext->DrawTextW(
                mText.c_str(),
                mText.length(),
                mTextFormat.Get(),
                Rect(),
                textBrush.Get(),
                D2D1_DRAW_TEXT_OPTIONS_CLIP | D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
            );
        }
        else
        {
            auto d2ddc5 = static_cast<ID2D1DeviceContext5*>(mD2DDeviceContext);
            if (!d2ddc5)
            {
                return;
            }
            auto dpiX = FLOAT{0};
            auto dpiY = FLOAT{0};
            d2ddc5->GetDpi(&dpiX, &dpiY);
            
            const auto pad = ceil(4.f * dpiX / 96.f);
            const auto s = (mSize.width - 2*pad) / 32.f;
            const auto dx = mPosition.x + pad;
            const auto dy = mPosition.y + pad;

            const auto scale     = D2D1::Matrix3x2F::Scale(s, s);
            const auto translate = D2D1::Matrix3x2F::Translation(dx, dy);
            
            d2ddc5->SetTransform(scale * translate);

            if (mIconId == 0)
                d2ddc5->DrawSvgDocument(mSvgIcon0.Get());
            else
                d2ddc5->DrawSvgDocument(mSvgIcon1.Get());

            d2ddc5->SetTransform(D2D1::Matrix3x2F::Identity());            
        }
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
        spdlog::error("D2DDeviceContext is null");
        return nullptr;
    }

    if (!dwriteFactory)
    {
        spdlog::error("DWriteFactory is null");
        return nullptr;
    }

    auto button = std::make_unique<Button>();

    button->mD2DDeviceContext = d2dDeviceContext;
    button->mDWriteFactory    = dwriteFactory;

    button->mPosition = desc.position;
    button->mSize     = desc.size;
    button->mText     = desc.text;
    
    button->mForceOutline   = desc.forceOutline;
    button->mIntelOutline   = desc.intelOutline;
    button->mRoundedCorners = desc.roundedCorners;
    button->mRoundedRadius  = desc.roundedRadius;

    // Create TextFormat.
    if (!button->CreateTextFormats(desc))
    {
        return nullptr;
    }

    // Create Brushes.
    if (!button->CreateBrushes(desc))
    {
        return nullptr;
    }

    // Create svg icon.
    if (desc.svg0 != 0)
    {
        if (!button->CrateSvg(desc))
        {
            return nullptr;
        }
    }

    spdlog::debug("Button created");

    return button;
}

} // namespace Impulse::Widgets
