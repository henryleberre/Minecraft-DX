#ifndef __MINECRAFT__MINECRAFT_HPP
#define __MINECRAFT__MINECRAFT_HPP

#include "Pch.hpp"
#include "Image.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "Window.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Camera.hpp"
#include "Shaders.hpp"
#include "Constants.hpp"
#include "vendor/PerlinNoise.hpp"

class Minecraft {
private:
    Window m_window;
    Camera m_camera;

    Microsoft::WRL::ComPtr<ID3D11Device>           m_pDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_pDeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>         m_pSwapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pPixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_pInputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>       m_pConstantBuffer;

    Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_pDepthStencilTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_pDepthStencilView;

    Image m_textureAtlasImage;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_pTextureAtlas;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTextureAtlasSRV;
    Microsoft::WRL::ComPtr<ID3D11SamplerState>       m_pTextureAtlasSamplerState;

    ChunkCoordMap<std::unique_ptr<Chunk>> m_pChunks;

    std::vector<Chunk*> m_pChunksToRender;

    siv::PerlinNoise m_noise;

public:
    Minecraft() noexcept;

private:
    void CreateDepthBuffer() noexcept;

    void LoadAndCreateTextureAtlas() noexcept;

    void UpdateWorld() noexcept;

    void Update() noexcept;

    void Render() noexcept;

public:
    inline std::optional<Chunk*> GetChunk(const ChunkCoord& location) noexcept {
        const auto pChunkIterator = this->m_pChunks.find(location);
        if (pChunkIterator != this->m_pChunks.end()) {
            return (*pChunkIterator).second.get();
        }

        return {  };
    }

    inline std::optional<BLOCK_TYPE*> GetBlock(const ChunkCoord& chunkLocation, const size_t idx, const size_t idy, const size_t idz) noexcept {
        const std::optional<Chunk*>& chunkOpt = this->GetChunk(chunkLocation);

        if (!chunkOpt.has_value()) return {  };

        return chunkOpt.value()->GetBlock(idx, idy, idz);
    }

    inline std::optional<BLOCK_TYPE*> GetBlock(const std::int16_t worldX, const std::int16_t worldY, const std::int16_t worldZ) noexcept {
        ChunkCoord cc{
            worldX / CHUNK_X_BLOCK_COUNT,
            worldZ / CHUNK_Z_BLOCK_COUNT
        };

        return this->GetBlock(cc, std::abs(worldX) % CHUNK_X_BLOCK_COUNT, worldY, std::abs(worldZ) % CHUNK_Z_BLOCK_COUNT);
    }

    inline void Run() noexcept {
        while (this->m_window.IsRunning()) {
            this->Update();
            this->Render();
        }
    }
}; // class Minecraft

#endif // __MINECRAFT__MINECRAFT_HPP