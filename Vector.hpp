#ifndef __MINECRAFT__VECTOR_HPP
#define __MINECRAFT__VECTOR_HPP

#include "Pch.hpp"

struct Vec4f32 {
    union {
        __m128 _reg;
        // anonymous structs are illegal in C++ but they work anyway :-)
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        struct { float red, green, blue, alpha; };
    };

    inline Vec4f32(const __m128& reg) noexcept : _reg(reg) {  }

    inline Vec4f32(const float x = 0.f, const float y = 0.f, const float z = 0.f, const float w = 0.f) noexcept {
        this->_reg = _mm_set_ps(w, z, y, x);
    }

    inline Vec4f32& operator+=(const Vec4f32& rhs) noexcept { this->_reg = _mm_add_ps(this->_reg, rhs._reg); return *this; }
    inline Vec4f32& operator-=(const Vec4f32& rhs) noexcept { this->_reg = _mm_sub_ps(this->_reg, rhs._reg); return *this; }
    inline Vec4f32& operator*=(const Vec4f32& rhs) noexcept { this->_reg = _mm_mul_ps(this->_reg, rhs._reg); return *this; }
    inline Vec4f32& operator/=(const Vec4f32& rhs) noexcept { this->_reg = _mm_div_ps(this->_reg, rhs._reg); return *this; }

    inline Vec4f32& operator+=(const float& rhs) noexcept { this->_reg = _mm_add_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
    inline Vec4f32& operator-=(const float& rhs) noexcept { this->_reg = _mm_sub_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
    inline Vec4f32& operator*=(const float& rhs) noexcept { this->_reg = _mm_mul_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
    inline Vec4f32& operator/=(const float& rhs) noexcept { this->_reg = _mm_div_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
}; // struct Vec4f32

inline Vec4f32 operator+(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_add_ps(lhs._reg, rhs._reg) }; }
inline Vec4f32 operator-(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_sub_ps(lhs._reg, rhs._reg) }; }
inline Vec4f32 operator*(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_div_ps(lhs._reg, rhs._reg) }; }
inline Vec4f32 operator/(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_mul_ps(lhs._reg, rhs._reg) }; }

inline Vec4f32 operator+(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_add_ps(lhs._reg, _mm_set_ps1(rhs)) }; }
inline Vec4f32 operator-(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_sub_ps(lhs._reg, _mm_set_ps1(rhs)) }; }
inline Vec4f32 operator*(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_div_ps(lhs._reg, _mm_set_ps1(rhs)) }; }
inline Vec4f32 operator/(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_mul_ps(lhs._reg, _mm_set_ps1(rhs)) }; }

inline Vec4f32 operator+(const float& lhs, const Vec4f32& rhs) noexcept { return rhs + lhs; }
inline Vec4f32 operator-(const float& lhs, const Vec4f32& rhs) noexcept { return rhs - lhs; }
inline Vec4f32 operator*(const float& lhs, const Vec4f32& rhs) noexcept { return rhs * lhs; }
inline Vec4f32 operator/(const float& lhs, const Vec4f32& rhs) noexcept { return rhs / lhs; }

std::ostream& operator<<(std::ostream& stream, const Vec4f32& rhs) {
    stream << '(' << rhs.x << ',' << rhs.y << ',' << rhs.z << ',' << rhs.w << ')';

    return stream;
}

struct Coloru8 {
    // I used multiple unions to avoid anonymous unions
    union { std::uint8_t r; std::uint8_t red;   };
    union { std::uint8_t g; std::uint8_t green; };
    union { std::uint8_t b; std::uint8_t blue;  };
    union { std::uint8_t a; std::uint8_t alpha; };
}; // struct Coloru8

#endif // __MINECRAFT__VECTOR_HPP