#ifndef __MINECRAFT__MATRIX_HPP
#define __MINECRAFT__MATRIX_HPP

#include "Pch.hpp"
#include "Vector.hpp"

struct Mat4x4f32 {
    std::array<float, 16> m = { 0.f };

    inline       float& operator()(const size_t i)       noexcept { return this->m[i]; }
    inline const float& operator()(const size_t i) const noexcept { return this->m[i]; }

    inline       float& operator()(const size_t r, const size_t c)       noexcept { return (*this)(r * 4u + c); }
    inline const float& operator()(const size_t r, const size_t c) const noexcept { return (*this)(r * 4u + c); }

    static Mat4x4f32 Zeroes;
    static Mat4x4f32 Identity;
}; // struct Mat4x4f32

inline Mat4x4f32 operator*(const Mat4x4f32& lhs, const Mat4x4f32& rhs) noexcept {
    Mat4x4f32 result{};

    for (char r = 0; r < 4u; ++r)
        for (char c = 0; c < 4u; ++c) // :-)
            for (char k = 0; k < 4u; ++k)
                result(r, c) += lhs(r, k) * rhs(k, c);

    return result;
}

inline Vec4f32 operator*(const Mat4x4f32& lhs, const Vec4f32& rhs) noexcept {
    return Vec4f32{
        lhs(0)  * rhs.x + lhs(1)  * rhs.y + lhs(2)  * rhs.z + lhs(3)  * rhs.w,
        lhs(4)  * rhs.x + lhs(5)  * rhs.y + lhs(6)  * rhs.z + lhs(7)  * rhs.w,
        lhs(8)  * rhs.x + lhs(9)  * rhs.y + lhs(10) * rhs.z + lhs(11) * rhs.w,
        lhs(12) * rhs.x + lhs(13) * rhs.y + lhs(14) * rhs.z + lhs(15) * rhs.w
    };
}

inline Mat4x4f32 Transposed(const Mat4x4f32& m) noexcept {
    return Mat4x4f32{ {
        m(0), m(4), m(8),  m(12),
        m(1), m(5), m(9),  m(13),
        m(2), m(6), m(10), m(14),
        m(3), m(7), m(11), m(15)
    } };
}

inline Mat4x4f32 MakeRotationXMatrix(const float& angle) noexcept {
    const float sinAngle = std::sinf(angle);
    const float cosAngle = std::cosf(angle);

    return Mat4x4f32{{
        1.f, 0.f,      0.f,       0.f,
		0.f, cosAngle, -sinAngle, 0.f,
		0.f, sinAngle, +cosAngle, 0.f,
		0.f, 0.f,      0.f,       1.f
    }};
}

inline Mat4x4f32 MakeRotationYMatrix(const float& angle) noexcept {
    const float sinAngle = std::sinf(angle);
    const float cosAngle = std::cosf(angle);

    return Mat4x4f32{{
        +cosAngle, 0.f, sinAngle, 0.f,
		0.f,       1.f, 0.f,      0.f,
		-sinAngle, 0.f, cosAngle, 0.f,
		0.f,       0.f, 0.f,      1.f
    }};
}

inline Mat4x4f32 MakeRotationZMatrix(const float& angle) noexcept {
    const float sinAngle = std::sinf(angle);
    const float cosAngle = std::cosf(angle);

    return Mat4x4f32{{
        cosAngle, -sinAngle, 0.f, 0.f,
        sinAngle, +cosAngle, 0.f, 0.f,
        0.f,      0.f,       1.f, 0.f,
        0.f,      0.f,       0.f, 1.f
    }};
}

inline Mat4x4f32 MakeRotationMatrix(const Vec4f32& rotation) noexcept {
    return MakeRotationZMatrix(rotation.z)
         * MakeRotationYMatrix(rotation.y)
         * MakeRotationXMatrix(rotation.x);
}

inline Mat4x4f32 MakeTranslationMatrix(const Vec4f32& translation) noexcept {
    return Mat4x4f32{{
        1.f,           0.f,           0.f,           0.f,
        0.f,           1.f,           0.f,           0.f,
        0.f,           0.f,           1.f,           0.f,
        translation.x, translation.y, translation.z, 1.f
    }};
}

inline Mat4x4f32 MakeLookAtMatrix(const Vec4f32& dir, const Vec4f32& up) noexcept {
    const Vec4f32 zAxis = Normalized3D(dir);
	const Vec4f32 xAxis = Normalized3D(CrossProduct3D(up, zAxis));
	const Vec4f32 yAxis = CrossProduct3D(zAxis, xAxis);

    return Mat4x4f32{{
        xAxis.x, yAxis.x, zAxis.x, 0.f,
	    xAxis.y, yAxis.y, zAxis.y, 0.f,
		xAxis.z, yAxis.z, zAxis.z, 0.f,
		0.f,     0.f,     0.f,     1.f
    }};
}

inline Mat4x4f32 MakePerspectiveMatrix(const float fov, const float aspectRatio, const float zNear, const float zFar) noexcept {
    return Mat4x4f32{{
        aspectRatio * fov, 0.f, 0.f,                              0.f,
        0.f,               fov, 0.f,                              0.f,
        0.f,               0.f, zFar / (zFar - zNear),            1.f,
        0.f,               0.f, (-zFar * zNear) / (zFar - zNear), 1.f,
    }};
}

inline std::ostream& operator<<(std::ostream& stream, const Mat4x4f32& m) {
    for (char r = 0; r < 4u; ++r) {
        for (char c = 0; c < 4u; ++c)
            stream << m(r, c) << ' ';
        stream << '\n';
    }

    return stream;
}

#endif // __MINECRAFT__MATRIX_HPP