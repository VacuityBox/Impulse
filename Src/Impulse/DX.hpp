#pragma once

#include "Utility.hpp"
#include <spdlog/spdlog.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <comdef.h>
#include <ShlObj.h>

namespace DX {

inline auto GetErrorMessage (HRESULT hr) -> std::string
{
    auto err  = _com_error(hr);
    auto utf8 = Impulse::UTF16ToUTF8(err.ErrorMessage());
    if (utf8)
    {
        return utf8.value();
    }

    return std::string();
}

inline auto ThrowIfFailed (HRESULT hr, const char* file, int line) -> void
{
    if (FAILED(hr))
    {
        spdlog::critical(
            "DX Call Failed [{} : {}] ({})\n    {}", file, line, hr, GetErrorMessage(hr)
        );
        
        throw std::exception("DX Call Failed");
    }
}

#ifndef DX_CALL
#define DX_CALL(_hr_) DX::ThrowIfFailed(_hr_, __FILE__, __LINE__)
#endif

} // namespace DX