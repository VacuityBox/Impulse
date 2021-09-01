#include "PCH.hpp"

#include "Impulse.hpp"

#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>

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
    auto logger = spdlog::basic_logger_mt("file_logger", "Impulse.log", true);
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::err);

#if defined(_DEBUG)
    auto msvcSink    = std::make_shared<spdlog::sinks::windebug_sink_mt>();
    auto debugLogger = std::make_shared<spdlog::logger>("msvc_logger", msvcSink);

    spdlog::set_default_logger(debugLogger);
    spdlog::set_level(spdlog::level::level_enum::debug);
#endif

    // Init ImpulseApp.
    auto impulse = Impulse::ImpulseApp();
    if (!impulse.Init(hInstance))
    {
        return 1;
    }
    
    auto ret = impulse.GfxLoop();

    impulse.Release();

    logger->flush();

    return ret;
}
