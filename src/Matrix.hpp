#ifndef __MINECRAFT__MATRIX_HPP
#define __MINECRAFT__MATRIX_HPP

#include "Pch.hpp"

struct Mat4x4f32 {
    std::array<float, 16> m = { 0.f };

    inline       float& operator()(const size_t i)       noexcept { return this->m[i]; }
    inline const float& operator()(const size_t i) const noexcept { return this->m[i]; }

    inline       float& operator()(const size_t r, const size_t c)       noexcept { return (*this)(r * 4u + c); }
    inline const float& operator()(const size_t r, const size_t c) const noexcept { return (*this)(r * 4u + c); }
}; // struct Mat4x4f32

inline Mat4x4f32 operator*(const Mat4x4f32& lhs, const Mat4x4f32& rhs) noexcept {
    Mat4x4f32 result;

    for (char r = 0; r < 4u; r++)
        for (char c = 0; c < 4u; c++) // :-)
            result(r, c) = DotProduct4D(Vec4f32(lhs(r, 0), lhs(r, 1), lhs(r, 2), lhs(r, 3)), Vec4f32(rhs(0, c), rhs(1, c), rhs(2, c), rhs(3, c)));

    return result;
}

inline Mat4x4f32 Transposed(const Mat4x4f32& m) noexcept {
    return Mat4x4f32{ {
        m(0), m(4), m(8),  m(12),
        m(1), m(5), m(9),  m(13),
        m(2), m(6), m(10), m(14),
        m(3), m(7), m(11), m(15)
    } };
}

std::ostream& operator<<(std::ostream& stream, const Mat4x4f32& m) {
    for (char r = 0; r < 4u; ++r) {
        for (char c = 0; c < 4u; ++c)
            stream << m(r, c) << ' ';
        stream << '\n';
    }

    return stream;
}

#endif // __MINECRAFT__MATRIX_HPP