#ifndef __MINECRAFT__CHUNK_HPP
#define __MINECRAFT__CHUNK_HPP

#include "Pch.hpp"
#include "Block.hpp"
#include "Constants.hpp"
#include "ErrorHandler.hpp"
#include "vendor/PerlinNoise.hpp"

class Minecraft;

struct ChunkCoord {
    std::int16_t idx;
    std::int16_t idz;

    inline bool operator==(const ChunkCoord& rhs) const noexcept { return this->idx == rhs.idx && this->idz == rhs.idz; }
}; // struct ChunkCoord

class ChunkCoordHash {
public:
    inline size_t operator()(const ChunkCoord& cc) const
    {
        const std::uint16_t a = ((cc.idx < 0 ? (static_cast<std::uint16_t>(1u) << 15) : 0) | static_cast<std::uint16_t>(std::abs(cc.idx)));
        const std::uint16_t b = ((cc.idz < 0 ? (static_cast<std::uint16_t>(1u) << 15) : 0) | static_cast<std::uint16_t>(std::abs(cc.idz)));

        return (static_cast<std::uint32_t>(a) << 16u) | static_cast<std::uint32_t>(b);
    }
};

template <typename T>
using ChunkCoordMap = std::unordered_map<ChunkCoord, T, ChunkCoordHash>;

class Chunk {
    friend Minecraft;
private:
    ChunkCoord m_location;

    std::array<std::array<std::array<BLOCK_TYPE, CHUNK_Z_BLOCK_COUNT>, CHUNK_Y_BLOCK_COUNT>, CHUNK_X_BLOCK_COUNT> m_blocks{};

    struct Chunk_DX_Data {
        Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
        size_t nVertices = 0u;
    };

    std::optional<Chunk_DX_Data> m_dxData;

public:
    inline Chunk() noexcept = default;

    inline Chunk(const ChunkCoord& location) noexcept
        : m_location(location)
    {  }

    inline ChunkCoord GetLocation() const noexcept { return this->m_location; }

    inline std::optional<const BLOCK_TYPE*> GetBlock(const size_t idx, const size_t idy, const size_t idz) const noexcept {
        if (idx >= 0 && idy >= 0 && idz >= 0 && idx < CHUNK_X_BLOCK_COUNT && idy < CHUNK_Y_BLOCK_COUNT && idz < CHUNK_Z_BLOCK_COUNT) {
            return &this->m_blocks[idx][idy][idz];
        }

        return {  };
    }

    inline std::optional<BLOCK_TYPE*> GetBlock(const size_t idx, const size_t idy, const size_t idz) noexcept {
        if (idx >= 0 && idy >= 0 && idz >= 0 && idx < CHUNK_X_BLOCK_COUNT && idy < CHUNK_Y_BLOCK_COUNT && idz < CHUNK_Z_BLOCK_COUNT) {
            return &this->m_blocks[idx][idy][idz];
        }

        return {  };
    }

    inline void SetBlock(const size_t idx, const size_t idy, const size_t idz, const BLOCK_TYPE& type) noexcept {
        if (idx >= 0 && idy >= 0 && idz >= 0 && idx < CHUNK_X_BLOCK_COUNT && idy < CHUNK_Y_BLOCK_COUNT && idz < CHUNK_Z_BLOCK_COUNT) {
            this->m_blocks[idx][idy][idz] = type;
        }
    }

    void GenerateDefaultTerrain(const siv::PerlinNoise& noise) noexcept;

    inline bool HasDXMesh() const noexcept { return this->m_dxData.has_value(); }

    inline void UnloadDXMesh() noexcept { this->m_dxData.reset(); }

    void GenerateDXMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::size_t textureAtlasWidth, const std::size_t textureAtlasHeight) noexcept;
}; // class Chunk

#endif // __MINECRAFT__CHUNK_HPP