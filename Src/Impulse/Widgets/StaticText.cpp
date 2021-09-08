#include "PCH.hpp"
#include "StaticText.hpp"
#include "DX.hpp"

#include <spdlog/spdlog.h>

namespace Impulse::Widgets {

auto StaticText::CreateBrushes (const StaticText::Desc& desc) -> bool
{
    auto hr = S_OK;

    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.defaultTextColor , &mDefaultTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.hoverTextColor   , &mHoverTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.activeTextColor  , &mActiveTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.focusTextColor   , &mFocusTextBrush);
    hr = mD2DDeviceContext->CreateSolidColorBrush(desc.disabledTextColor, &mDisabledTextBrush);
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", DX::GetErrorMessage(hr));
        return false;
    }

    return true;
}

auto StaticText::CreateTextFormats (const StaticText::Desc& desc) -> bool
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

auto StaticText::CreateTextFormat (
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

auto StaticText::CreateBrush (D2D_COLOR_F color) -> ComPtr<ID2D1SolidColorBrush>
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

auto StaticText::HitTest (D2D_POINT_2F point)  -> bool
{
    const auto rect = Rect();

    return (rect.left <= point.x && point.x <= rect.right)
        && (rect.top  <= point.y && point.y <= rect.bottom)
        ;;
}

auto StaticText::Draw (ID2D1DeviceContext* d2dContext) -> void
{
#if defined(_DEBUG)
    d2dContext->DrawRectangle(Rect(), mDefaultTextBrush.Get());
#endif

    ID2D1SolidColorBrush* brush = mDefaultTextBrush.Get();

    switch (mState)
    {
    case Widget::State::Hover:
        brush = mHoverTextBrush.Get();
        auto rect = Rect();
        auto mat = D2D1::Matrix3x2F::Scale(D2D1::SizeF(1.1f, 1.1f), D2D1::Point2F(mPosition.x + (mSize.width / 2), mPosition.y + (mSize.height / 2)));
        d2dContext->SetTransform(mat);
        break;
    }


    d2dContext->DrawTextW(
        mText.c_str(), mText.length(), mTextFormat.Get(), Rect(), brush
    );
    
    d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

auto StaticText::Create (
    const StaticText::Desc& desc,
    ID2D1DeviceContext*     d2dDeviceContext,
    IDWriteFactory*         dwriteFactory
) -> std::unique_ptr<StaticText>
{
    spdlog::debug("Creating StaticText");

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

    auto staticText = std::make_unique<StaticText>();

    staticText->mD2DDeviceContext = d2dDeviceContext;
    staticText->mDWriteFactory    = dwriteFactory;

    staticText->mPosition = desc.position;
    staticText->mSize     = desc.size;
    staticText->mText     = desc.text;

    // Create TextFormat.
    if (!staticText->CreateTextFormats(desc))
    {
        return nullptr;
    }

    // Create Brushes.
    if (!staticText->CreateBrushes(desc))
    {
        return nullptr;
    }

    spdlog::debug("StaticText created");

    return staticText;
}

} // namespace Impulse::Widgets
