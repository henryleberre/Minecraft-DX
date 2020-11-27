#ifndef __MINECRAFT__CAMERA_HPP
#define __MINECRAFT__CAMERA_HPP

#include "Vector.hpp"
#include "Matrix.hpp"

class Camera {
private:
    Vec4f32 m_position    = {};
    Vec4f32 m_rotation    = {};
    float   m_fov         = static_cast<float>(M_PI_2);
    float   m_aspectRatio = 9.f / 16.f;
    float   m_zNear       = 0.1f;
    float   m_zFar        = 1000.f;

    Mat4x4f32 m_transform;

public:
    inline Camera() noexcept = default;

    inline Camera(const Vec4f32& position, const float fov, const float aspectRatio, const float zNear, const float zFar) noexcept
        : m_position(position), m_fov(fov), m_aspectRatio(aspectRatio), m_zNear(zNear), m_zFar(zFar)
    {  }

    inline Vec4f32 GetPosition() const noexcept { return this->m_position; }

    inline void Translate  (const Vec4f32& delta)    noexcept { this->m_position += delta;    }
    inline void SetPosition(const Vec4f32& position) noexcept { this->m_position  = position; }

    inline void Rotate     (const Vec4f32& delta)    noexcept { this->m_rotation += delta;    }
    inline void SetRotation(const Vec4f32& rotation) noexcept { this->m_rotation  = rotation; }

    inline Mat4x4f32 GetTransform() const noexcept { return this->m_transform; }

    void CalculateTransform() noexcept {
        const Mat4x4f32 translation = MakeTranslationMatrix(this->m_position * (-1.f));
        const Mat4x4f32 lookAt      = MakeLookAtMatrix(Vec4f32{0.f, 0.f, 1.f, 0.f}, Vec4f32{0.f, 1.f, 0.f, 0.f});
        const Mat4x4f32 perspective = MakePerspectiveMatrix(this->m_fov, this->m_aspectRatio, this->m_zNear, this->m_zFar);

        this->m_transform = translation * lookAt * perspective;
    }
};

#endif // __MINECRAFT__CAMERA_HPP