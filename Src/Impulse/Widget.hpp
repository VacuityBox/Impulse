#pragma once

#include <functional>

#include <d2d1.h>
#include <wrl.h>

namespace {
    using namespace D2D1;
    using namespace Microsoft::WRL;
}

namespace Impulse {

class Widget
{
public:
    enum class State : unsigned char
    {
        Default, // Enabled
        Hover,   // Mouse Over
        Active,  // Pressed
        Focus,
        Disabled
    };

protected:
    State mState;

public:
    std::function<void ()> OnClick = []{};

public:
    virtual ~Widget () = default;

    virtual auto HitTest (D2D_POINT_2F point)  -> bool = 0;
    virtual auto Update  (Widget::State state) -> bool = 0;
    virtual auto Draw    (ComPtr<ID2D1RenderTarget> pRenderTarget) -> void = 0;
};

} // namespace Impulse
