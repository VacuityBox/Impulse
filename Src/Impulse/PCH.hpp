#pragma once

/// Standard library
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

/// Windows libraries
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <ShlObj.h>

/// Direct2D
#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

/// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

/// json
#include <nlohmann/json.hpp>
