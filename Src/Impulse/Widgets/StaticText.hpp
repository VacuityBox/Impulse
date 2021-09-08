#pragma once

#include "Widget.hpp"

#include <memory>
#include <string>
#include <utility>

#include <d2d1_3.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse::Widgets {

class StaticText : public Widget
{
public:
    struct Desc
    {
        D2D_POINT_2F        position             = D2D1::Point2F();
        D2D_SIZE_F          size                 = D2D1::SizeF();
        const WCHAR*        text                 = L"StaticText";

        const WCHAR*        fontName             = L"Segoe UI";
        FLOAT               fontSize             = 11.0f;
        DWRITE_FONT_WEIGHT  fontWeight           = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE   fontStyle            = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH fontStretch          = DWRITE_FONT_STRETCH_NORMAL;

        D2D_COLOR_F         defaultTextColor     = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         hoverTextColor       = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         activeTextColor      = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         focusTextColor       = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         disabledTextColor    = D2D1::ColorF(D2D1::ColorF::Black);
    };

private:
    D2D_POINT_2F                 mPosition          = D2D1::Point2F();
    D2D_SIZE_F                   mSize              = D2D1::SizeF();
    std::wstring                 mText              = L"";

    ComPtr<IDWriteTextFormat>    mTextFormat;
    ComPtr<ID2D1SolidColorBrush> mDefaultTextBrush;
    ComPtr<ID2D1SolidColorBrush> mHoverTextBrush;
    ComPtr<ID2D1SolidColorBrush> mActiveTextBrush;
    ComPtr<ID2D1SolidColorBrush> mFocusTextBrush;
    ComPtr<ID2D1SolidColorBrush> mDisabledTextBrush;

    ID2D1DeviceContext*          mD2DDeviceContext  = nullptr;
    IDWriteFactory*              mDWriteFactory     = nullptr;

private:
    auto CreateBrushes     (const StaticText::Desc& desc) -> bool;
    auto CreateTextFormats (const StaticText::Desc& desc) -> bool;

    auto CreateBrush      (D2D_COLOR_F color) -> ComPtr<ID2D1SolidColorBrush>;
    auto CreateTextFormat (
        const WCHAR*        fontName,
        FLOAT               fontSize,
        DWRITE_FONT_WEIGHT  fontWeight,
        DWRITE_FONT_STYLE   fontStyle,
        DWRITE_FONT_STRETCH fontStretch
    ) -> ComPtr<IDWriteTextFormat>;

    
public:
    StaticText  () = default;
    ~StaticText () = default;

    auto Position (float x, float y)  { mPosition.x = x; mPosition.y = y; }
    auto Size     (float w, float h)  { mSize.width = w; mSize.height = h; }
    auto Text     (std::wstring text) { mText = std::move(text); }

    const auto  Position () const { return mPosition; }
    const auto  Size     () const { return mSize; }
    const auto& Text     () const { return mText; }
    const auto  Rect     () const
    {
        return D2D1::RectF(
            mPosition.x,
            mPosition.y,
            mPosition.x + mSize.width,
            mPosition.y + mSize.height
        );
    }

    auto FontSize (float size) -> bool
    {
        auto length   = static_cast<size_t>(mTextFormat->GetFontFamilyNameLength());
        auto fontName = std::wstring(length + 1, '\0');
        if (FAILED(mTextFormat->GetFontFamilyName(fontName.data(), fontName.length())))
        {
            return false;
        }

        auto textFormat = CreateTextFormat(
            fontName.c_str(),
            size,
            mTextFormat->GetFontWeight(),
            mTextFormat->GetFontStyle(),
            mTextFormat->GetFontStretch()
        );
        if (textFormat)
        {
            mTextFormat = textFormat;
            return true;
        }

        return false;
    }

    #pragma region Set Brush Color

    auto DefaultColor (D2D_COLOR_F color) -> bool
    {
        if (auto brush = CreateBrush(color))
        {            
            mDefaultTextBrush = brush;
            return true;
        }

        return false;
    }

    auto HoverColor (D2D_COLOR_F color) -> bool
    {
        if (auto brush = CreateBrush(color))
        {
            mHoverTextBrush = brush;
            return true;
        }

        return false;
    }

    auto ActiveColor (D2D_COLOR_F color) -> bool
    {
        if (auto brush = CreateBrush(color))
        {
            mActiveTextBrush = brush;
            return true;
        }

        return false;
    }

    auto FocusColor (D2D_COLOR_F color) -> bool
    {
        if (auto brush = CreateBrush(color))
        {
            mFocusTextBrush = brush;
            return true;
        }

        return false;
    }

    auto DisabledColor (D2D_COLOR_F color) -> bool
    {
        if (auto brush = CreateBrush(color))
        {
            mDisabledTextBrush = brush;
            return true;
        }

        return false;
    }

    #pragma endregion

    virtual auto HitTest (D2D_POINT_2F point)                   -> bool override;
    virtual auto Draw    (ID2D1DeviceContext* d2dDeviceContext) -> void override;

    static auto Create (
        const StaticText::Desc& desc,
        ID2D1DeviceContext*     d2dDeviceContext,
        IDWriteFactory*         dwriteFactory
    ) -> std::unique_ptr<StaticText>;
};

} // namespace Impulse::Widgets
