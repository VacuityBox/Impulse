#pragma once

/// Standard library
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

/// Windows libraries
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <shellapi.h>
#include <ShlObj.h>

/// Direct2D
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <wrl.h>

/// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

/// json
#include <nlohmann/json.hpp>
