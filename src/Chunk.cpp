#include "Chunk.hpp"

void Chunk::GenerateDefaultTerrain(const siv::PerlinNoise &noise) noexcept {
    std::memset(this->m_blocks.data(), (int)BLOCK_TYPE::BLOCK_TYPE_AIR, this->m_blocks.size() * sizeof(BLOCK_TYPE));

    for (size_t x = 0u; x < CHUNK_X_BLOCK_COUNT; ++x) {
        for (size_t z = 0u; z < CHUNK_Z_BLOCK_COUNT; ++z) {
            const size_t yMax = static_cast<size_t>(noise.normalizedOctaveNoise2D_0_1((this->m_location.idx * CHUNK_X_BLOCK_COUNT + (std::int16_t)x) / 50.f,
                                                                                      (this->m_location.idz * CHUNK_X_BLOCK_COUNT + (std::int16_t)z) / 50.f, 3) * CHUNK_Y_BLOCK_COUNT / 2u);

            for (size_t y = 0u; y <= yMax; ++y) {
                if (y == yMax) {
                    if (y > CHUNK_Y_BLOCK_COUNT / 5)
                        this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_GRASS;
                    else
                        this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_SAND;
                } else if (y > yMax - 2)
                    this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_DIRT;
                else
                    this->m_blocks[x][y][z] = BLOCK_TYPE::BLOCK_TYPE_STONE;
            }
        }
    }
}

void Chunk::GenerateDXMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::size_t textureAtlasWidth, const std::size_t textureAtlasHeight) noexcept {
    std::vector<Vertex> vertices(CHUNK_X_BLOCK_COUNT * CHUNK_Y_BLOCK_COUNT * CHUNK_Z_BLOCK_COUNT * 3u);
    size_t nVertices = 0u;

    BLOCK_TYPE dummyAirBlock = BLOCK_TYPE::BLOCK_TYPE_AIR;

    const UV uvTextureSize = {
        TEXTURE_SIDE_LENGTH / static_cast<float>(textureAtlasWidth),
        TEXTURE_SIDE_LENGTH / static_cast<float>(textureAtlasHeight)
    };

    for (size_t x = 0u; x < CHUNK_X_BLOCK_COUNT; ++x) {
        for (size_t y = 0u; y < CHUNK_Y_BLOCK_COUNT; ++y) {
            for (size_t z = 0u; z < CHUNK_Z_BLOCK_COUNT; ++z) {
                const BLOCK_TYPE& blockType = *this->GetBlock(x, y, z).value();

                if (IsBlockOpaque(blockType)) {
                    // top left corner point of the current block
                    const Vec4f32 baseCornerPoint = {
                        BLOCK_LENGTH * ((std::int16_t)x + this->m_location.idx * (std::int16_t)CHUNK_X_BLOCK_COUNT),
                        (y + 1) * BLOCK_LENGTH,
                        BLOCK_LENGTH * ((std::int16_t)z + this->m_location.idz * (std::int16_t)CHUNK_Z_BLOCK_COUNT),
                        1.f
                    };

                    // in clockwise order with "a" in the top left position
                    const auto AddFace = [&uvTextureSize, &nVertices, &blockType, &vertices, this](const Vec4f32 &a, const Vec4f32 &b, const Vec4f32 &c, const Vec4f32 &e, const BLOCK_FACE &blockFace) {
                        const UV baseFaceUV = GetBlockFaceBaseUV(blockType, blockFace, uvTextureSize);
                        const float faceLighting = GetBlockFaceLighting(blockFace);

                        vertices[nVertices++] = Vertex{a, UV{baseFaceUV.u, baseFaceUV.v}, faceLighting};
                        vertices[nVertices++] = Vertex{b, UV{baseFaceUV.u + uvTextureSize.u, baseFaceUV.v}, faceLighting};
                        vertices[nVertices++] = Vertex{c, UV{baseFaceUV.u + uvTextureSize.u, baseFaceUV.v + uvTextureSize.v}, faceLighting};

                        vertices[nVertices++] = Vertex{a, UV{baseFaceUV.u, baseFaceUV.v}, faceLighting};
                        vertices[nVertices++] = Vertex{c, UV{baseFaceUV.u + uvTextureSize.u, baseFaceUV.v + uvTextureSize.v}, faceLighting};
                        vertices[nVertices++] = Vertex{e, UV{baseFaceUV.u, baseFaceUV.v + uvTextureSize.v}, faceLighting};
                    };

                    // Front
                    if (!IsBlockOpaque(*this->GetBlock(x, y, z - 1).value_or(&dummyAirBlock))) {
                        AddFace(baseCornerPoint,
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +0},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +0},
                                baseCornerPoint + Vec4f32{+0.f, -BLOCK_LENGTH, +0}, BLOCK_FACE::BLOCK_FACE_FRONT);
                    }

                    // Back
                    if (!IsBlockOpaque(*this->GetBlock(x, y, z + 1).value_or(&dummyAirBlock))) {
                        AddFace(baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+0, +0, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+0.f, -BLOCK_LENGTH, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +BLOCK_LENGTH}, BLOCK_FACE::BLOCK_FACE_FRONT);
                    }

                    // Left
                    if (!IsBlockOpaque(*this->GetBlock(x - 1, y, z).value_or(&dummyAirBlock))) {
                        AddFace(baseCornerPoint + Vec4f32{0, +0, +BLOCK_LENGTH},
                                baseCornerPoint,
                                baseCornerPoint + Vec4f32{0, -BLOCK_LENGTH, +0},
                                baseCornerPoint + Vec4f32{0, -BLOCK_LENGTH, +BLOCK_LENGTH}, BLOCK_FACE::BLOCK_FACE_LEFT);
                    }

                    // Right
                    if (!IsBlockOpaque(*this->GetBlock(x + 1, y, z).value_or(&dummyAirBlock))) {
                        AddFace(baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +0},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +0}, BLOCK_FACE::BLOCK_FACE_RIGHT);
                    }

                    // Top
                    if (!IsBlockOpaque(*this->GetBlock(x, y + 1, z).value_or(&dummyAirBlock))) {
                        AddFace(baseCornerPoint + Vec4f32{+0, +0, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +0},
                                baseCornerPoint, BLOCK_FACE::BLOCK_FACE_TOP);
                    }

                    // Bottom
                    if (!IsBlockOpaque(*this->GetBlock(x, y - 1, z).value_or(&dummyAirBlock))) {
                        AddFace(baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+0, -BLOCK_LENGTH, +BLOCK_LENGTH},
                                baseCornerPoint + Vec4f32{+0, -BLOCK_LENGTH, +0},
                                baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +0}, BLOCK_FACE::BLOCK_FACE_BOTTOM);
                    }
                }
            }
        }
    }

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.ByteWidth = static_cast<UINT>(nVertices * sizeof(Vertex));
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Vertex);
    bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices.data();
    sd.SysMemPitch = 0;
    sd.SysMemSlicePitch = 0;
    
    Chunk::Chunk_DX_Data newDXData;
    newDXData.nVertices = nVertices;

    if (device->CreateBuffer(&bufferDesc, &sd, &newDXData.pVertexBuffer) != S_OK)
        FATAL_ERROR("Failed to create a vertex buffer");

    this->m_dxData.emplace(std::move(newDXData));
}