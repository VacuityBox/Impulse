#include "PCH.hpp"
#include "StaticText.hpp"

#include "Utility.hpp"
#include <spdlog/spdlog.h>

namespace Impulse {

auto StaticText::FontSize (float size, IDWriteFactory* pDWriteFactory) -> bool
{
    auto hr         = S_OK;
    auto length     = mTextFormat->GetFontFamilyNameLength();
    auto familyName = std::wstring(length + 1, L'\0');
    
    hr = mTextFormat->GetFontFamilyName(familyName.data(), length + 1); // +1 for NULL char
    if (FAILED(hr))
    {
        spdlog::error("GetFontFamilyName() failed: {}", HResultToString(hr));
        return false;
    }

    auto newTextFormat = ComPtr<IDWriteTextFormat>();
    hr = pDWriteFactory->CreateTextFormat(
        familyName.c_str(),
        nullptr,
        mTextFormat->GetFontWeight(),
        mTextFormat->GetFontStyle(),
        mTextFormat->GetFontStretch(),
        size,
        L"",
        newTextFormat.GetAddressOf()
    );
    if (FAILED(hr))
    {
        spdlog::error("CreateTextFormat() failed: {}", HResultToString(hr));
        return false;
    }

    mTextFormat.Reset();
    mTextFormat = std::move(newTextFormat);

    mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    return true;
}

auto StaticText::HitTest (D2D_POINT_2F point)  -> bool
{
    const auto rect = Rect();

    return (rect.left <= point.x && point.x <= rect.right)
        && (rect.top  <= point.y && point.y <= rect.bottom)
        ;;
}

auto StaticText::Draw (ID2D1RenderTarget* pRenderTarget) -> void
{
#if defined(_DEBUG)
    pRenderTarget->DrawRectangle(Rect(), mDefaultTextBrush.Get());
#endif

    pRenderTarget->DrawTextW(
        mText.c_str(), mText.length(), mTextFormat.Get(), Rect(), mDefaultTextBrush.Get()
    );
}

auto StaticText::Create (
    const StaticText::Desc& desc,
    ID2D1RenderTarget*      pRenderTarget,
    IDWriteFactory*         pDWriteFactory
) -> std::unique_ptr<StaticText>
{
    spdlog::debug("Creating StaticText");

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

    auto hr         = S_OK;
    auto staticText = StaticText();

    staticText.mPosition = desc.position;
    staticText.mSize     = desc.size;
    staticText.mText     = desc.text;

    hr = pDWriteFactory->CreateTextFormat(
        desc.font,
        nullptr,
        desc.fontWeight,
        desc.fontStyle,
        desc.fontStretch,
        desc.fontSize,
        L"",
        staticText.mTextFormat.GetAddressOf()
    );
    if (FAILED(hr))
    {
        spdlog::error("CreateTextFormat() failed: {}", HResultToString(hr));
        return nullptr;
    }

    staticText.mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    staticText.mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    auto createBrush = [&](const D2D_COLOR_F& color, ID2D1SolidColorBrush** ppBrush)
    {
        return pRenderTarget->CreateSolidColorBrush(color, ppBrush);
    };

    hr = createBrush(desc.defaultTextColor    , staticText.mDefaultTextBrush    .GetAddressOf());
    hr = createBrush(desc.hoverTextColor      , staticText.mHoverTextBrush      .GetAddressOf());
    hr = createBrush(desc.activeTextColor     , staticText.mActiveTextBrush     .GetAddressOf());
    hr = createBrush(desc.focusTextColor      , staticText.mFocusTextBrush      .GetAddressOf());
    hr = createBrush(desc.disabledTextColor   , staticText.mDisabledTextBrush   .GetAddressOf());
    if (FAILED(hr))
    {
        spdlog::error("CreateSolidColorBrush() failed: {}", HResultToString(hr));
        return nullptr;
    }

    spdlog::debug("StaticText created");

    return std::make_unique<StaticText>(std::move(staticText));
}

} // namespace Impulse
