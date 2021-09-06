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

namespace Impulse::Widgets {

class StaticText : public Widget
{
public:
    struct Desc
    {
        D2D_POINT_2F        position             = D2D1::Point2F();
        D2D_SIZE_F          size                 = D2D1::SizeF();
        const WCHAR*        text                 = L"StaticText";

        const WCHAR*        font                 = L"Segoe UI";
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
    D2D_POINT_2F mPosition = D2D1::Point2F();
    D2D_SIZE_F   mSize     = D2D1::SizeF();
    std::wstring mText;

    ComPtr<IDWriteTextFormat>    mTextFormat;
    ComPtr<IDWriteTextLayout>    mTextLayout;

    ComPtr<ID2D1SolidColorBrush> mDefaultTextBrush;
    ComPtr<ID2D1SolidColorBrush> mHoverTextBrush;
    ComPtr<ID2D1SolidColorBrush> mActiveTextBrush;
    ComPtr<ID2D1SolidColorBrush> mFocusTextBrush;
    ComPtr<ID2D1SolidColorBrush> mDisabledTextBrush;

public:
    StaticText  () = default;
    ~StaticText () = default;

    auto Position (float x, float y)  { mPosition.x = x; mPosition.y = y; }
    auto Size     (float w, float h)  { mSize.width = w; mSize.height = h; }
    auto Text     (std::wstring text) { mText = std::move(text); }

    auto FontSize (float size, IDWriteFactory* pDWriteFactory) -> bool;

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

    virtual auto HitTest (D2D_POINT_2F point)                   -> bool override;
    virtual auto Draw    (ID2D1DeviceContext* d2dDeviceContext) -> void override;

    static auto Create (
        const StaticText::Desc& desc,
        ID2D1DeviceContext*     d2dDeviceContext,
        IDWriteFactory*         dwriteFactory
    ) -> std::unique_ptr<StaticText>;
};

} // namespace Impulse::Widgets
