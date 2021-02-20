#ifndef __MINECRAFT__CAMERA_HPP
#define __MINECRAFT__CAMERA_HPP

#include "Chunk.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"

typedef Vec4f32 CameraFrustumPlane;

inline bool IsPointInsideFrustumOfPlane(const Vec4f32& point, const CameraFrustumPlane& plane) noexcept {
    return DotProduct3D(point, plane) + plane.w > 0.f;
}

inline bool IsPointOutsideFrustumOfPlane(const Vec4f32& point, const CameraFrustumPlane& plane) noexcept {
    return DotProduct3D(point, plane) + plane.w <= 0.f;
}

enum class CAMERA_FRUSTUM_PLANE {
    CAMERA_FRUSTUM_PLANE_TOP = 0u,
    CAMERA_FRUSTUM_PLANE_NEAR,
    CAMERA_FRUSTUM_PLANE_LEFT,
    CAMERA_FRUSTUM_PLANE_RIGHT,
    CAMERA_FRUSTUM_PLANE_FAR,
    CAMERA_FRUSTUM_PLANE_BOTTOM,

    _COUNT
}; // enum class CAMERA_FRUSTUM_PLANE

struct CameraFrustum {
    std::array<CameraFrustumPlane, static_cast<std::size_t>(CAMERA_FRUSTUM_PLANE::_COUNT)> planes;

    inline bool IsChunkInFrustum(const Chunk& chunk) const noexcept {        
        const auto& cc = chunk.GetLocation();

        const Vec4f32 chunkBaseXZAxis = {
            static_cast<float>(cc.idx) * CHUNK_X_LENGTH,
            0.f,
            static_cast<float>(cc.idz) * CHUNK_Z_LENGTH
        };

        const auto& checkFrustumPlanes = {
            CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_LEFT,
            CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_RIGHT,
            CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_BOTTOM,
            CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_TOP,
            CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_NEAR,
            CAMERA_FRUSTUM_PLANE::CAMERA_FRUSTUM_PLANE_FAR
        };

        const auto& checkPoints = {
            chunkBaseXZAxis + Vec4f32{0.f,            0.f,            0.f },
            chunkBaseXZAxis + Vec4f32{CHUNK_X_LENGTH, 0.f,            0.f },
            chunkBaseXZAxis + Vec4f32{CHUNK_X_LENGTH, CHUNK_Y_LENGTH, 0.f },
            chunkBaseXZAxis + Vec4f32{0.f,            CHUNK_Y_LENGTH, 0.f },

            chunkBaseXZAxis + Vec4f32{0.f,            0.f,            CHUNK_Z_LENGTH },
            chunkBaseXZAxis + Vec4f32{CHUNK_X_LENGTH, 0.f,            CHUNK_Z_LENGTH },
            chunkBaseXZAxis + Vec4f32{CHUNK_X_LENGTH, CHUNK_Y_LENGTH, CHUNK_Z_LENGTH },
            chunkBaseXZAxis + Vec4f32{0.f,            CHUNK_Y_LENGTH, CHUNK_Z_LENGTH }
        };

        for (const auto& planeEnum : checkFrustumPlanes) {
            const CameraFrustumPlane& plane = this->planes[static_cast<std::size_t>(planeEnum)];

            bool bOneIn = false;
            for (const auto& point : checkPoints) {
                if (IsPointInsideFrustumOfPlane(point, plane)) {
                    bOneIn = true;
                    break;
                }
            }
            if (!bOneIn)
                return false;
        }

        return true;
    }

    inline const CameraFrustumPlane& operator()(const CAMERA_FRUSTUM_PLANE& planeLocation) const noexcept {
        return this->planes[static_cast<std::size_t>(planeLocation)];
    }

    inline CameraFrustumPlane& operator()(const CAMERA_FRUSTUM_PLANE& planeLocation) noexcept {
        return this->planes[static_cast<std::size_t>(planeLocation)];
    }
}; // struct CameraFrustum

class Camera {
private:
    Vec4f32 m_position    = {0.f, 0.f, 0.f, 0.f};
    Vec4f32 m_rotation    = {0.f, 0.f, 0.f, 0.f};
    float   m_fov         = static_cast<float>(M_PI_2);
    float   m_aspectRatio = 9.f / 16.f;
    float   m_zNear       = 0.1f;
    float   m_zFar        = 1000.f;

    Mat4x4f32 m_transform;

private:
    // these members are calculated each call to "CalculateTransform"
    // and are used to move the camera in the right direction when
    // handling keyboard input
    Vec4f32 m_forwardVector = {0.f, 0.f, 1.f, 0.f};
    Vec4f32 m_rightVector   = {1.f, 0.f, 0.f, 0.f};

    CameraFrustum m_frustum;

public:
    inline Camera() noexcept = default;

    inline Camera(const Vec4f32& position, const float fov, const float aspectRatio, const float zNear, const float zFar) noexcept
        : m_position(position), m_fov(fov), m_aspectRatio(aspectRatio), m_zNear(zNear), m_zFar(zFar)
    {  }

    inline Vec4f32 GetForwardVector() const noexcept { return this->m_forwardVector; }
    inline Vec4f32 GetRightVector()   const noexcept { return this->m_rightVector;   }

    inline Vec4f32 GetPosition() const noexcept { return this->m_position; }

    inline void Translate  (const Vec4f32& delta)    noexcept { this->m_position += delta;    }
    inline void SetPosition(const Vec4f32& position) noexcept { this->m_position  = position; }

    inline void Rotate     (const Vec4f32& delta)    noexcept { this->m_rotation += delta;    }
    inline void SetRotation(const Vec4f32& rotation) noexcept { this->m_rotation  = rotation; }

    inline Mat4x4f32     GetTransform() const noexcept { return this->m_transform; }
    inline CameraFrustum GetFrustum()   const noexcept { return this->m_frustum;   }

    void Update() noexcept;
};

#endif // __MINECRAFT__CAMERA_HPP