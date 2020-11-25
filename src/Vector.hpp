#ifndef __MINECRAFT__VECTOR_HPP
#define __MINECRAFT__VECTOR_HPP

#include "Pch.hpp"

struct Vec4f32 {
    float x, y, z, w;

    inline Vec4f32(const float x = 0.f, const float y = 0.f, const float z = 0.f, const float w = 0.f) noexcept {
        this->x = x; this->y = y; this->z = z; this->w = w;
    }

    inline Vec4f32& operator+=(const Vec4f32& rhs) noexcept { this->x += rhs.x; this->y += rhs.y; this->z += rhs.z; this->w += rhs.w; return *this; }
    inline Vec4f32& operator-=(const Vec4f32& rhs) noexcept { this->x -= rhs.x; this->y -= rhs.y; this->z -= rhs.z; this->w -= rhs.w; return *this; }
    inline Vec4f32& operator*=(const Vec4f32& rhs) noexcept { this->x *= rhs.x; this->y *= rhs.y; this->z *= rhs.z; this->w *= rhs.w; return *this; }
    inline Vec4f32& operator/=(const Vec4f32& rhs) noexcept { this->x /= rhs.x; this->y /= rhs.y; this->z /= rhs.z; this->w /= rhs.w; return *this; }

    inline Vec4f32& operator+=(const float& rhs) noexcept { this->x += rhs; this->y += rhs; this->z += rhs; this->w += rhs; return *this; }
    inline Vec4f32& operator-=(const float& rhs) noexcept { this->x -= rhs; this->y -= rhs; this->z -= rhs; this->w -= rhs; return *this; }
    inline Vec4f32& operator*=(const float& rhs) noexcept { this->x *= rhs; this->y *= rhs; this->z *= rhs; this->w *= rhs; return *this; }
    inline Vec4f32& operator/=(const float& rhs) noexcept { this->x /= rhs; this->y /= rhs; this->z /= rhs; this->w /= rhs; return *this; }
}; // struct Vec4f32

inline Vec4f32 operator+(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline Vec4f32 operator-(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline Vec4f32 operator*(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
inline Vec4f32 operator/(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }

inline Vec4f32 operator+(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs}; }
inline Vec4f32 operator-(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs}; }
inline Vec4f32 operator*(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs}; }
inline Vec4f32 operator/(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs}; }

inline Vec4f32 operator+(const float& lhs, const Vec4f32& rhs) noexcept { return rhs + lhs; }
inline Vec4f32 operator-(const float& lhs, const Vec4f32& rhs) noexcept { return rhs - lhs; }
inline Vec4f32 operator*(const float& lhs, const Vec4f32& rhs) noexcept { return rhs * lhs; }
inline Vec4f32 operator/(const float& lhs, const Vec4f32& rhs) noexcept { return rhs / lhs; }

inline float DotProduct3D(const Vec4f32& lhs, const Vec4f32& rhs) noexcept {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline float DotProduct4D(const Vec4f32& lhs, const Vec4f32& rhs) noexcept {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

std::ostream& operator<<(std::ostream& stream, const Vec4f32& rhs) {
    stream << '(' << rhs.x << ',' << rhs.y << ',' << rhs.z << ',' << rhs.w << ')';

    return stream;
}

// Uses saturation
struct Coloru8 {
    union {
        // anonymous structs are illegal in C++ but they work anyway :-)
        struct { std::uint8_t x, y, z, w;              };
        struct { std::uint8_t r, g, b, a;              };
        struct { std::uint8_t red, green, blue, alpha; };
        std::uint32_t raw;
    };

    inline Coloru8() noexcept : raw(0u) {  }

    inline Coloru8(const std::uint8_t x, const std::uint8_t y = 0u, const std::uint8_t z = 0u, const std::uint8_t w = 0u) noexcept {
        this->x = x; this->y = y; this->z = z; this->w = w;
    }
}; // struct Coloru8

#endif // __MINECRAFT__VECTOR_HPP