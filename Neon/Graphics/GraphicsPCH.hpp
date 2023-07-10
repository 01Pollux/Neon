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

#include <Math/Vector.hpp>
#include <Math/Matrix.hpp>
#include <Math/Colors.hpp>

#if defined(NEON_GRAPHICS_DIRECTX12)
#include <Private/RHI/Dx12/Device.hpp>
#include <Private/RHI/Dx12/DirectXHeaders.hpp>
#include <Private/RHI/Dx12/D3D12MemAlloc.hpp>
#endif