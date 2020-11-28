#ifndef __MINECRAFT__CHUNK_HPP
#define __MINECRAFT__CHUNK_HPP

#include "Pch.hpp"
#include "Block.hpp"
#include "Constants.hpp"
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

    std::array<std::array<std::array<BLOCK_TYPE, CHUNK_Z_BLOCK_COUNT>, CHUNK_Y_BLOCK_COUNT>, CHUNK_X_BLOCK_COUNT> m_blocks = { BLOCK_TYPE::BLOCK_TYPE_AIR };

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

    void GenerateDefaultTerrain(const siv::PerlinNoise& noise) noexcept {
        std::memset(this->m_blocks.data(), (int)BLOCK_TYPE::BLOCK_TYPE_AIR, this->m_blocks.size() * sizeof(BLOCK_TYPE));

        for (size_t x = 0u; x < CHUNK_X_BLOCK_COUNT; ++x) {
        for (size_t z = 0u; z < CHUNK_Z_BLOCK_COUNT; ++z) {
        
        const size_t yMax = static_cast<size_t>(noise.normalizedOctaveNoise2D_0_1((this->m_location.idx * CHUNK_X_BLOCK_COUNT + (std::int16_t)x) / 50.f,
                                                                                  (this->m_location.idz * CHUNK_X_BLOCK_COUNT + (std::int16_t)z) / 50.f, 3) * CHUNK_Y_BLOCK_COUNT / 2u);

        for (size_t y = 0u; y <= yMax; ++y) {
            if (y == yMax) {
                if (y > CHUNK_Y_BLOCK_COUNT / 5) this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_GRASS;
                else this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_SAND;
            } else if (y > yMax - 2)
                this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_DIRT;
            else
                this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_STONE;
        }
        }
        }
    }
}; // class Chunk

struct ChunkRenderData {
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
    size_t nVertices;
}; // struct ChunkRenderData

#endif // __MINECRAFT__CHUNK_HPP