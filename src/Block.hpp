#ifndef __MINECRAFT__BLOCK_HPP
#define __MINECRAFT__BLOCK_HPP

#include "Vector.hpp"

struct UV {
    float u, v;
}; // struct UV

struct Vertex {
    Vec4f32 position;
    UV      uv;
    float   lighting = 1.0f;
}; // struct Vertex

enum class BLOCK_FACE : std::uint8_t {
    BLOCK_FACE_TOP = 0u, BLOCK_FACE_FRONT, BLOCK_FACE_LEFT, BLOCK_FACE_RIGHT, BLOCK_FACE_BACK, BLOCK_FACE_BOTTOM,

    _COUNT
}; // enum class BlockFace

enum class BLOCK_VISIBILITY : std::uint8_t {
    BLOCK_VISIBILITY_OPAQUE = 0u, BLOCK_VISIBILITY_TRANSPARENT, BLOCK_VISIBILITY_TRANSLUCENT
}; // enum class BlockVisibility

enum class BLOCK_TYPE : std::uint8_t {
    BLOCK_TYPE_AIR = 0u, BLOCK_TYPE_STONE, BLOCK_TYPE_DIRT, BLOCK_TYPE_GRASS, BLOCK_TYPE_SAND, BLOCK_TYPE_WATER, 
    
    _COUNT // used to know at compile time then umber of block 
}; // enum class BlockType

struct BlockProperties {
    BLOCK_TYPE                                                   type;
    BLOCK_VISIBILITY                                             visibility;
};

inline BlockProperties GetBlockProperties(const BLOCK_TYPE& type) noexcept {
    // can set as constexpr: cl.exe has an internal error
    static std::array<BlockProperties, static_cast<std::size_t>(BLOCK_TYPE::_COUNT)> properties = {
        BlockProperties{ BLOCK_TYPE::BLOCK_TYPE_AIR,   BLOCK_VISIBILITY::BLOCK_VISIBILITY_TRANSPARENT },
        BlockProperties{ BLOCK_TYPE::BLOCK_TYPE_DIRT,  BLOCK_VISIBILITY::BLOCK_VISIBILITY_OPAQUE      },
        BlockProperties{ BLOCK_TYPE::BLOCK_TYPE_GRASS, BLOCK_VISIBILITY::BLOCK_VISIBILITY_OPAQUE      },
        BlockProperties{ BLOCK_TYPE::BLOCK_TYPE_SAND,  BLOCK_VISIBILITY::BLOCK_VISIBILITY_OPAQUE      },
        BlockProperties{ BLOCK_TYPE::BLOCK_TYPE_STONE, BLOCK_VISIBILITY::BLOCK_VISIBILITY_OPAQUE      },
        BlockProperties{ BLOCK_TYPE::BLOCK_TYPE_WATER, BLOCK_VISIBILITY::BLOCK_VISIBILITY_TRANSLUCENT }
    };

    return properties[static_cast<std::size_t>(type)];
}

inline bool IsBlockTransparent(const BLOCK_TYPE& blockType) noexcept { return GetBlockProperties(blockType).visibility == BLOCK_VISIBILITY::BLOCK_VISIBILITY_TRANSPARENT; }
inline bool IsBlockTranslucent(const BLOCK_TYPE& blockType) noexcept { return GetBlockProperties(blockType).visibility == BLOCK_VISIBILITY::BLOCK_VISIBILITY_TRANSLUCENT; }
inline bool IsBlockOpaque     (const BLOCK_TYPE& blockType) noexcept { return GetBlockProperties(blockType).visibility == BLOCK_VISIBILITY::BLOCK_VISIBILITY_OPAQUE;      }

inline UV   GetBlockFaceBaseUV(const BLOCK_TYPE& blockType, const BLOCK_FACE& blockFace, const UV& textureUVSize) noexcept {
    return UV{
        textureUVSize.u * (static_cast<float>(blockType) - 1.f),
        textureUVSize.v * (static_cast<float>(blockFace))
    };
}

inline float GetBlockFaceLighting(const BLOCK_FACE& blockFace) noexcept {
    constexpr static std::array<float, static_cast<std::size_t>(BLOCK_FACE::_COUNT)> lightingValues = {
        1.0f, 0.9f, 0.5f, 0.9f, 0.5f, 0.25f 
    };

    return lightingValues[static_cast<std::size_t>(blockFace)];
}

#endif // __MINECRAFT__BLOCK_HPP