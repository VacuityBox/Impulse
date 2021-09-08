#include "PCH.hpp"
#include "Utility.hpp"
#include "Resource.h"

#include <ShlObj.h>

namespace Impulse {

auto UTF8ToUTF16 (const std::string_view str) -> std::optional<std::wstring>
{
    // Get size.
    auto size = ::MultiByteToWideChar(
        CP_UTF8,
        0,
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        0
    );

    if (size <= 0)
    {
        return std::nullopt;
    }

    // Convert.
    auto utf16 = std::wstring(size, L'\0');
    auto ret = ::MultiByteToWideChar(
        CP_UTF8,
        0,
        str.data(),
        static_cast<int>(str.size()),
        utf16.data(),
        static_cast<int>(utf16.size())
    );

    if (ret == 0)
    {
        return std::nullopt;
    }

    return utf16;
}

auto UTF16ToUTF8 (const std::wstring_view str) -> std::optional<std::string>
{
    // Get size.
    auto size = ::WideCharToMultiByte(
        CP_UTF8,
        0,
        str.data(),
        static_cast<int>(str.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (size <= 0)
    {
        return std::nullopt;
    }
    
    // Convert.
    auto utf8 = std::string(size, '\0');
    auto ret = ::WideCharToMultiByte(
        CP_UTF8,
        0,
        str.data(),
        static_cast<int>(str.size()),
        utf8.data(),
        static_cast<int>(utf8.size()),
        nullptr,
        nullptr
    );

    if (ret == 0)
    {
        return std::nullopt;
    }

    return utf8;
}

auto GetLastErrorMessage () -> std::string
{
    const auto messageMaxSize = 4096;

    auto message = std::wstring(messageMaxSize, L'\0');
    auto code    = GetLastError();

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        message.data(),
        message.size(),
        nullptr
    );

    auto utf8 = UTF16ToUTF8(message.c_str());
    if (utf8)
    {
        return utf8.value();
    }

    return std::string();
}

auto GetAppDataPath () -> std::filesystem::path
{
    auto appDataPath = std::array<wchar_t, MAX_PATH>();
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appDataPath.data())))
    {
        const auto dir = std::wstring(appDataPath.data());
        return std::filesystem::path(dir);
    }

    return std::filesystem::path();
}

auto LoadSvgFromResource (int id) -> std::string
{
    const auto hInstance = GetModuleHandleW(nullptr);
    if (!hInstance)
    {
        return std::string();
    }
    
    auto resource = FindResourceW(hInstance, MAKEINTRESOURCEW(id), RT_SVG);
    if (!resource)
    {
        return std::string();
    }

    auto handle = LoadResource(hInstance, resource);
    if (!handle)
    {
        return std::string();
    }

    const auto size = SizeofResource(hInstance, resource);
    const auto data = static_cast<const char *>(LockResource(handle));
    
    auto output = std::string(data, size);

    FreeResource(handle);

    return output;
}

} // namespace Impulse
