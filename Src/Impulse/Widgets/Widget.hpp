#pragma once

#include <functional>

#include <d2d1_1.h>

namespace {
    using namespace D2D1;
}

namespace Impulse::Widgets {

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
    State mState = State::Default;

public:
    std::function<void ()> OnClick     = []{};
    std::function<void ()> OnMouseOver = []{};
    std::function<void ()> OnMouseAway = []{};

public:
    virtual ~Widget () = default;

    virtual auto Update  (Widget::State state) -> bool
    {
        if (mState != state)
        {
            mState = state;
            return true;
        }

        return false;
    }

    virtual auto HitTest (D2D_POINT_2F point)                   -> bool = 0;
    virtual auto Draw    (ID2D1DeviceContext* d2dDeviceContext) -> void = 0;
};

} // namespace Impulse::Widgets
