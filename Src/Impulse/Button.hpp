#pragma once

#include "Utility.hpp"
#include "Widget.hpp"

#include <memory>
#include <spdlog/spdlog.h>
#include <string>

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
    D2D_POINT_2F mPosition;
    D2D_SIZE_F   mSize;
    std::wstring mText;

    ComPtr<IDWriteTextFormat>    mDWriteTextFormat;

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
    auto Text     (std::wstring text) { mText = text; }

    const auto& Text  () const { return mText; }
    const auto  State () const { return mState; }
    const auto  Rect  () const
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
        ComPtr<IDWriteTextFormat>    pDWriteTextFormat,
        ComPtr<ID2D1SolidColorBrush> pDefaultTextBrush,
        ComPtr<ID2D1SolidColorBrush> pDefaultOutlineBrush,
        ComPtr<ID2D1SolidColorBrush> pHoverTextBrush,
        ComPtr<ID2D1SolidColorBrush> pHoverOutlineBrush,
        ComPtr<ID2D1SolidColorBrush> pActiveTextBrush,
        ComPtr<ID2D1SolidColorBrush> pActiveOutlineBrush,
        ComPtr<ID2D1SolidColorBrush> pFocusTextBrush,
        ComPtr<ID2D1SolidColorBrush> pFocusOutlineBrush,
        ComPtr<ID2D1SolidColorBrush> pDisabledTextBrush,
        ComPtr<ID2D1SolidColorBrush> pDisabledOutlineBrush
    ) -> std::unique_ptr<Button>;
};

} // namespace Impulse
