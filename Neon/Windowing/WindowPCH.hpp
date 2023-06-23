#pragma once

#include <memory>
#include <functional>

#include <unordered_map>
#include <map>
#include <list>
#include <string>
#include <set>
#include <vector>
#include <array>
#include <span>
#include <ranges>

#include <chrono>
#include <numbers>
#include <random>

#if defined(NEON_PLATFORM_WINDOWS)
#include <Private/Window/WindowHeaders.hpp>
#endif

#include <Math/Vector.hpp>
#include <Math/Matrix.hpp>
#include <Math/Colors.hpp>
