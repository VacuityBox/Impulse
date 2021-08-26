#pragma once

#include "ImpulseState.hpp"
#include "Utility.hpp"

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

// std::wstring serializer/deserializer.
namespace nlohmann {

template <>
struct adl_serializer<std::wstring>
{
    static void to_json(json& j, const std::wstring& opt)
    {
        auto utf8 = Impulse::UTF16ToUTF8(opt.c_str());
        j = utf8 ? utf8.value() : "";
    }

    static void from_json(const json& j, std::wstring& opt)
    {
        auto utf16 = Impulse::UTF8ToUTF16(j.get<std::string>());
        opt = utf16 ? utf16.value() : L"";
    }
};

} // namespace nlohmann

namespace Impulse {

constexpr auto MINUTES (const uint32_t m)
{
    return m * 60;
}

enum class WindowPosition : unsigned char
{
    Auto,
    LeftTop,
    LeftEdge,
    LeftBottom,
    CenterTop,
    Center,
    CenterBottom,
    RightTop,
    RightEdge,
    RightBottom
};

class Settings
{
public:
    uint32_t       WorkDuration        = 10;//MINUTES(25);
    uint32_t       ShortBreakDuration  = 4;//MINUTES(5);
    uint32_t       LongBreakDuration   = 8;//MINUTES(15);
    uint32_t       LongBreakAfter      = 4;
    bool           AutoStartTimer      = false;
    std::wstring   TaskName            = L"";
    WindowPosition WindowPosition      = WindowPosition::Auto;

    // Internal (don't save).
    ImpulseState CurrentState        = ImpulseState::Inactive;
    ImpulseState PreviousState       = ImpulseState::Inactive;
    uint32_t     WorkShiftCount      = 1;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        Settings,
        WorkDuration,
        ShortBreakDuration,
        LongBreakDuration,
        LongBreakAfter,
        AutoStartTimer,
        TaskName
    )
};

} // namespace Impulse
