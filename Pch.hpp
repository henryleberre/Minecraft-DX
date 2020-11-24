#ifndef __MINECRAFT__PCH_HPP
#define __MINECRAFT__PCH_HPP

// Standard Library Includes
#define _USE_MATH_DEFINES

#include <cmath>
#include <array>
#include <chrono>
#include <cctype>
#include <vector>
#include <cstdint>
#include <optional>
#include <iostream>
#include <unordered_map>

#undef _USE_MATH_DEFINES

// Intrinsics
#ifndef __MINECRAFT__NO_SIMD
    #include <immintrin.h>
#endif // __MINECRAFT__NO_SIMD

#ifdef _WIN32

    // Windows Includes
    #include <wrl.h>
    #include <d3d11_4.h>
    #include <Windows.h>
    #include <wincodec.h>
    #include <d3dcompiler.h>

    // Windows Linking
    #pragma comment(lib, "d3d11")
    #pragma comment(lib, "User32.lib")
    #pragma comment(lib, "d3dcompiler")
    #pragma comment(lib, "windowscodecs.lib")

#endif // _WIN32

#endif // __MINECRAFT__PCH_HPP