#pragma once

/// Standard library
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

/// Direct2D
#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

/// spdlog
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
