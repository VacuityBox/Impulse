#pragma once

namespace Impulse {

enum class ImpulseState : unsigned char
{
    Inactive,
    WorkShift,
    ShortBreak,
    LongBreak,
    Paused
};

} // namespace Impulse
