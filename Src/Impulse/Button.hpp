#pragma once

#include "Widget.hpp"

#include <memory>
#include <string>
#include <utility>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse {

class Button : public Widget
{
public:
    struct Desc
    {
        D2D_POINT_2F        position             = D2D1::Point2F();
        D2D_SIZE_F          size                 = D2D1::SizeF();
        const WCHAR*        text                 = L"StaticText";

        const WCHAR*        font                 = L"SegoeUI";
        FLOAT               fontSize             = 11.0f;
        DWRITE_FONT_WEIGHT  fontWeight           = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE   fontStyle            = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH fontStretch          = DWRITE_FONT_STRETCH_NORMAL;

        D2D_COLOR_F         defaultTextColor     = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         defaultOutlineColor  = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         hoverTextColor       = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         hoverOutlineColor    = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         activeTextColor      = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         activeOutlineColor   = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         focusTextColor       = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         focusOutlineColor    = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         disabledTextColor    = D2D1::ColorF(D2D1::ColorF::Black);
        D2D_COLOR_F         disabledOutlineColor = D2D1::ColorF(D2D1::ColorF::Black);
    };

private:
    D2D_POINT_2F mPosition = D2D1::Point2F();
    D2D_SIZE_F   mSize     = D2D1::SizeF();
    std::wstring mText;

    ComPtr<IDWriteTextFormat>    mTextFormat;

    ComPtr<ID2D1SolidColorBrush> mDefaultTextBrush;
    ComPtr<ID2D1SolidColorBrush> mDefaultOutlineBrush;
    ComPtr<ID2D1SolidColorBrush> mHoverTextBrush;
    ComPtr<ID2D1SolidColorBrush> mHoverOutlineBrush;
    ComPtr<ID2D1SolidColorBrush> mActiveTextBrush;
    ComPtr<ID2D1SolidColorBrush> mActiveOutlineBrush;
    ComPtr<ID2D1SolidColorBrush> mFocusTextBrush;
    ComPtr<ID2D1SolidColorBrush> mFocusOutlineBrush;
    ComPtr<ID2D1SolidColorBrush> mDisabledTextBrush;
    ComPtr<ID2D1SolidColorBrush> mDisabledOutlineBrush;

public:
    Button  () = default;
    ~Button () = default;

    auto Position (float x, float y)  { mPosition.x = x; mPosition.y = y; }
    auto Size     (float w, float h)  { mSize.width = w; mSize.height = h; }
    auto Text     (std::wstring text) { mText = std::move(text); }

    const auto& Text  () const { return mText; }
    const auto  Rect  () const
    {
        return D2D1::RectF(
            mPosition.x,
            mPosition.y,
            mPosition.x + mSize.width,
            mPosition.y + mSize.height
        );
    }

    virtual auto HitTest (D2D_POINT_2F point)               -> bool override;
    virtual auto Draw    (ID2D1RenderTarget* pRenderTarget) -> void override;

    static auto Create (
        const Button::Desc& desc,
        ID2D1RenderTarget*  pRenderTarget,
        IDWriteFactory*     pDWriteFactory
    ) -> std::unique_ptr<Button>;
};

} // namespace Impulse
