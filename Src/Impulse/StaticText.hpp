#pragma once

#include "Widget.hpp"

#include <memory>
#include <string>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse {

class StaticText : public Widget
{
    D2D_POINT_2F mPosition = D2D1::Point2F();
    D2D_SIZE_F   mSize     = D2D1::SizeF();
    std::wstring mText;

    ComPtr<IDWriteTextFormat>    mTextFormat;
    ComPtr<ID2D1SolidColorBrush> mTextBrush;

public:
    StaticText  () = default;
    ~StaticText () = default;

    auto Position (float x, float y) { mPosition.x = x; mPosition.y = y; }
    auto Size     (float w, float h) { mSize.width = w; mSize.height = h; }

    const auto& Text () const { return mText; }
    const auto  State() const { return mState; }
    const auto  Rect () const
    {
        return D2D_RECT_F{
            mPosition.x,
            mPosition.y,
            mPosition.x + mSize.width,
            mPosition.y + mSize.height,
        };
    }

    virtual auto HitTest (D2D_POINT_2F point)  -> bool override;
    virtual auto Update  (Widget::State state) -> bool override;
    virtual auto Draw    (ComPtr<ID2D1RenderTarget> pRenderTarget) -> void override;

    static auto Create (
        std::wstring                 text,
        D2D_POINT_2F                 position,
        D2D_SIZE_F                   size,
        ComPtr<IDWriteTextFormat>    pTextFormat,
        ComPtr<ID2D1SolidColorBrush> pTextBrush
    ) -> std::unique_ptr<StaticText>;
};

} // namespace Impulse
