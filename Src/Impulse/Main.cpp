#include "PCH.hpp"

#include "Impulse.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

auto WINAPI wWinMain (
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nShowCmd
) -> int
{
    // Init logger.
    auto logger = spdlog::basic_logger_mt("ImpulseFileLog", "Impulse.log", true);
    spdlog::set_default_logger(logger);

#if defined(_DEBUG)
    spdlog::set_level(spdlog::level::level_enum::debug);
#endif

    // Init ImpulseApp.
    auto impulse = Impulse::ImpulseApp();
    if (!impulse.Init(hInstance))
    {
        return 1;
    }
    
    auto ret = impulse.MainLoop();

    impulse.Release();

    return ret;
}
