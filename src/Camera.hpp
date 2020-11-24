#ifndef __MINECRAFT__CAMERA_HPP
#define __MINECRAFT__CAMERA_HPP

#include "Vector.hpp"
#include "Matrix.hpp"

class Camera {
private:
    Vec4f32 m_position = {};
    float   m_fov = M_PI_2;
    float   m_aspectRatio = 9.f / 16.f;
    float   m_zNear = 0.1f;
    float   m_zFar = 1000.f;

    Mat4x4f32 m_transform;

public:
    inline Camera() noexcept = default;

    inline Camera(const Vec4f32& position, const float fov, const float aspectRatio, const float zNear, const float zFar) noexcept
        : m_position(position), m_fov(fov), m_aspectRatio(aspectRatio), m_zNear(zNear), m_zFar(zFar)
    {  }

    inline Vec4f32 GetPosition() const noexcept { return this->m_position; }

    inline void Translate(const Vec4f32& rhs) noexcept { this->m_position += rhs; }
    inline void SetPosition(const Vec4f32& rhs) noexcept { this->m_position = rhs; }

    inline Mat4x4f32 GetTransform() const noexcept { return this->m_transform; }

    void CalculateTransform() noexcept {
        this->m_transform =
            Mat4x4f32{ {
                1.f,                 0.f,                 0.f,                 0.f,
                0.f,                 1.f,                 0.f,                 0.f,
                0.f,                 0.f,                 1.f,                 0.f,
                -this->m_position.x, -this->m_position.y, -this->m_position.z, 1.f
            } } *Mat4x4f32{ {
                this->m_aspectRatio * this->m_fov, 0.f,                 0.f,                                                                                    0.f,
                0.f,                               this->m_fov,         0.f,                                                                                    0.f,
                0.f,                               0.f,                 this->m_zFar / (this->m_zFar - this->m_zNear),                                          1.f,
                0.f,                               0.f, (-this->m_zFar * this->m_zNear) / (this->m_zFar - this->m_zNear), 1.f,
            } };
    }
};

#endif // __MINECRAFT__CAMERA_HPP