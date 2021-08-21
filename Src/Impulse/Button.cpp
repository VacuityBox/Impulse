#include "PCH.hpp"
#include "Button.hpp"

namespace Impulse {

auto Button::HitTest (D2D_POINT_2F point) -> bool
{
    const auto rect = Rect();

    return (rect.left <= point.x && point.x <= rect.right)
        && (rect.top  <= point.y && point.y <= rect.bottom)
        ;;
}

auto Button::Update (Widget::State state) -> bool
{
    if (mState == state || mState == Widget::State::Disabled)
    {
        return false;
    }

    mState = state;
    return true;
}

auto Button::Draw (ComPtr<ID2D1RenderTarget> pRenderTarget) -> void
{
    auto draw = [&](ComPtr<ID2D1SolidColorBrush> textBrush, ComPtr<ID2D1SolidColorBrush> outlineBrush)
    {
        pRenderTarget->DrawRectangle(Rect(), outlineBrush.Get());
        pRenderTarget->DrawTextW(
            mText.c_str(), mText.length(), mDWriteTextFormat.Get(), Rect(), textBrush.Get()
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
) -> std::unique_ptr<Button>
{
    auto button = Button();

    button.mPosition = position;
    button.mSize     = size;
    button.mText     = text;

    button.mDWriteTextFormat = pDWriteTextFormat;

    button.mDefaultTextBrush     = pDefaultTextBrush;
    button.mDefaultOutlineBrush  = pDefaultOutlineBrush;
    button.mHoverTextBrush       = pHoverTextBrush;
    button.mHoverOutlineBrush    = pHoverOutlineBrush;
    button.mActiveTextBrush      = pActiveTextBrush;
    button.mActiveOutlineBrush   = pActiveOutlineBrush;
    button.mFocusTextBrush       = pFocusTextBrush;
    button.mFocusOutlineBrush    = pFocusOutlineBrush;
    button.mDisabledTextBrush    = pDisabledTextBrush;
    button.mDisabledOutlineBrush = pDisabledOutlineBrush;

    return std::make_unique<Button>(button);
    
}

} // namespace Impulse
