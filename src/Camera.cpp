#include "Camera.hpp"

void Camera::Update() noexcept {
    // Clamp rotation
    if (this->m_rotation.x > +M_PI_2) { this->m_rotation.x = +M_PI_2; }
    if (this->m_rotation.x < -M_PI_2) { this->m_rotation.x = -M_PI_2; }

    // Set member variables
    const Mat4x4f32 rotationMatrix = MakeRotationMatrix(this->m_rotation);
    this->m_forwardVector = rotationMatrix * Vec4f32{ 0.f, 0.f, 1.f, 0.f };
    this->m_rightVector   = rotationMatrix * Vec4f32{ 1.f, 0.f, 0.f, 0.f };

    // Calculate the transform
    const Mat4x4f32 translationMatrix = MakeTranslationMatrix(this->m_position * (-1.f));
    const Mat4x4f32 lookAtMatrix      = MakeLookAtMatrix     (this->m_forwardVector, rotationMatrix * Vec4f32{ 0.f, 1.f, 0.f, 0.f });
    const Mat4x4f32 perspectiveMatrix = MakePerspectiveMatrix(this->m_fov, this->m_aspectRatio, this->m_zNear, this->m_zFar);

    this->m_transform = translationMatrix * lookAtMatrix * perspectiveMatrix;

    const Mat4x4f32& m = this->m_transform;

    // Calculate the frustum (http://web.archive.org/web/20120531231005/http://crazyjoke.free.fr/doc/3D/plane%20extraction.pdf)
    Vec4f32 np{ m(1,4) + m(1,3), m(2,4) + m(2,3), m(3,4) + m(3,3), m(4,4) + m(4, 3) };
    Vec4f32 fp{ m(1,4) - m(1,3), m(2,4) - m(2,3), m(3,4) - m(3,3), m(4,4) - m(4, 3) };

    Vec4f32 lp{ m(1,4) + m(1,1), m(2,4) + m(2,1), m(3,4) + m(3,1), m(4,4) + m(4, 1) };
    Vec4f32 rp{ m(1,4) - m(1,1), m(2,4) - m(2,1), m(3,4) - m(3,1), m(4,4) - m(4, 1) };
    
    Vec4f32 tp{ m(1,4) - m(1,2), m(2,4) - m(2,2), m(3,4) - m(3,2), m(4,4) - m(4, 2) };
    Vec4f32 bp{ m(1,4) + m(1,2), m(2,4) + m(2,2), m(3,4) + m(3,2), m(4,4) + m(4, 2) };

    this->m_frustum(CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_TOP)    = tp / tp.GetLength3D();
    this->m_frustum(CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_NEAR)   = np / np.GetLength3D();
    this->m_frustum(CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_LEFT)   = lp / lp.GetLength3D();
    this->m_frustum(CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_RIGHT)  = rp / rp.GetLength3D();
    this->m_frustum(CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_FAR)    = fp / fp.GetLength3D();
    this->m_frustum(CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_BOTTOM) = bp / bp.GetLength3D();
}