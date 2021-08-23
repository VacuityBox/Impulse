#include "PCH.hpp"
#include "StaticText.hpp"

namespace Impulse {

auto StaticText::HitTest (D2D_POINT_2F point)  -> bool
{
    const auto rect = Rect();

    return (rect.left <= point.x && point.x <= rect.right)
        && (rect.top  <= point.y && point.y <= rect.bottom)
        ;;
}

auto StaticText::Update (Widget::State state) -> bool
{
    if (mState == state)
    {
        return false;
    }

    mState = state;
    return true;
}

auto StaticText::Draw (ComPtr<ID2D1RenderTarget> pRenderTarget) -> void
{
    pRenderTarget->DrawTextW(
        mText.c_str(), mText.length(), mTextFormat.Get(), Rect(), mTextBrush.Get()
    );
}

auto StaticText::Create (
    std::wstring                 text,
    D2D_POINT_2F                 position,
    D2D_SIZE_F                   size,
    ComPtr<IDWriteTextFormat>    pTextFormat,
    ComPtr<ID2D1SolidColorBrush> pTextBrush
) -> std::unique_ptr<StaticText>
{
    auto staticText = StaticText();

    staticText.mText       = text;
    staticText.mPosition   = position;
    staticText.mSize       = size;
    staticText.mTextFormat = pTextFormat;
    staticText.mTextBrush  = pTextBrush;

    return std::make_unique<StaticText>(std::move(staticText));
}

} // namespace Impulse
