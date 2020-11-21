#define _USE_MATH_DEFINES
#include <cmath>
#undef _USE_MATH_DEFINES

#include <array>
#include <chrono>
#include <cctype>
#include <vector>
#include <cstdint>
#include <optional>
#include <iostream>
#include <unordered_map>

#include <immintrin.h>

#include <wrl.h>
#include <d3d11_4.h>
#include <Windows.h>
#include <wincodec.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "windowscodecs.lib")

#include "PerlinNoise.hpp"

#define FATAL_ERROR(errorMsg) { MessageBoxA(NULL, errorMsg, "Minecraft: Fatal Error", MB_ICONERROR); std::exit(-1); }

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

class Window {

    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
    HWND m_handle = NULL;

    std::uint16_t m_width, m_height;

    std::array<bool, 0xFE> m_bDownKeys = { false };

public:
    Window(const char* title, const std::uint16_t width, const std::uint16_t height) noexcept
        : m_width(width), m_height(height)
    {
        WNDCLASSA wc = {};

        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpfnWndProc = WindowProc;
        wc.lpszClassName = "Minecraft's Window Class";

        if (!RegisterClassA(&wc))
            FATAL_ERROR("Failed to register a window class");

        RECT windowRect{ 0, 0, width, height };
        if (!AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false))
            FATAL_ERROR("Failed to adjust window rect");

        this->m_handle = CreateWindowA(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, wc.hInstance, NULL);

        if (!this->m_handle)
            FATAL_ERROR("Failed to create the window");

#ifdef _WIN64
        SetWindowLongPtrA(this->m_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
#else
        SetWindowLongA(this->m_handle, GWLP_USERDATA, reinterpret_cast<LONG>(this));
#endif

        ShowWindow(this->m_handle, SW_SHOW);
    }

    inline void Destroy() noexcept {
        if (!DestroyWindow(this->m_handle))
            FATAL_ERROR("Failed to destroy the window");

        this->m_handle = NULL;
    }

    ~Window() noexcept {
        if (this->m_handle)
            this->Destroy();
    }

    inline bool IsKeyDown(const char c) const noexcept { return  this->m_bDownKeys[std::toupper(c)]; }
    inline bool IsKeyUp(const char c) const noexcept { return !this->m_bDownKeys[std::toupper(c)]; }

    inline std::uint16_t GetWidth()  const noexcept { return this->m_width; }
    inline std::uint16_t GetHeight() const noexcept { return this->m_height; }

    inline HWND GetHandle() const noexcept { return this->m_handle; }
    inline bool IsRunning() const noexcept { return this->m_handle != NULL; }

    void Update() {
        MSG message;
        while (PeekMessageA(&message, this->m_handle, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
}; // class Window


LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
#ifdef _WIN64
    Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
#else
    Window* pWindow = reinterpret_cast<Window*>(GetWindowLong(hwnd, GWLP_USERDATA));
#endif

    if (!pWindow)
        return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_DESTROY:
        pWindow->m_handle = NULL;
        return 0;
    case WM_KEYDOWN:
        pWindow->m_bDownKeys[wParam] = true;
        return 0;
    case WM_KEYUP:
        pWindow->m_bDownKeys[wParam] = false;
        return 0;
    };

    return DefWindowProc(hwnd, msg, wParam, lParam);
} // WindowProc

struct Vec4f32 {
    union {
        __m128 _reg;
        // anonymous structs are illegal in C++ but they work anyway :-)
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        struct { float red, green, blue, alpha; };
    };

    inline Vec4f32(const __m128& reg) noexcept : _reg(reg) {  }

    inline Vec4f32(const float x = 0.f, const float y = 0.f, const float z = 0.f, const float w = 0.f) noexcept {
        this->_reg = _mm_set_ps(w, z, y, x);
    }

    inline Vec4f32& operator+=(const Vec4f32& rhs) noexcept { this->_reg = _mm_add_ps(this->_reg, rhs._reg); return *this; }
    inline Vec4f32& operator-=(const Vec4f32& rhs) noexcept { this->_reg = _mm_sub_ps(this->_reg, rhs._reg); return *this; }
    inline Vec4f32& operator*=(const Vec4f32& rhs) noexcept { this->_reg = _mm_mul_ps(this->_reg, rhs._reg); return *this; }
    inline Vec4f32& operator/=(const Vec4f32& rhs) noexcept { this->_reg = _mm_div_ps(this->_reg, rhs._reg); return *this; }

    inline Vec4f32& operator+=(const float& rhs) noexcept { this->_reg = _mm_add_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
    inline Vec4f32& operator-=(const float& rhs) noexcept { this->_reg = _mm_sub_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
    inline Vec4f32& operator*=(const float& rhs) noexcept { this->_reg = _mm_mul_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
    inline Vec4f32& operator/=(const float& rhs) noexcept { this->_reg = _mm_div_ps(this->_reg, _mm_set_ps1(rhs)); return *this; }
}; // struct Vec4f32

inline Vec4f32 operator+(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_add_ps(lhs._reg, rhs._reg) }; }
inline Vec4f32 operator-(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_sub_ps(lhs._reg, rhs._reg) }; }
inline Vec4f32 operator*(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_div_ps(lhs._reg, rhs._reg) }; }
inline Vec4f32 operator/(const Vec4f32& lhs, const Vec4f32& rhs) noexcept { return Vec4f32{ _mm_mul_ps(lhs._reg, rhs._reg) }; }

inline Vec4f32 operator+(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_add_ps(lhs._reg, _mm_set_ps1(rhs)) }; }
inline Vec4f32 operator-(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_sub_ps(lhs._reg, _mm_set_ps1(rhs)) }; }
inline Vec4f32 operator*(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_div_ps(lhs._reg, _mm_set_ps1(rhs)) }; }
inline Vec4f32 operator/(const Vec4f32& lhs, const float& rhs) noexcept { return Vec4f32{ _mm_mul_ps(lhs._reg, _mm_set_ps1(rhs)) }; }

inline Vec4f32 operator+(const float& lhs, const Vec4f32& rhs) noexcept { return rhs + lhs; }
inline Vec4f32 operator-(const float& lhs, const Vec4f32& rhs) noexcept { return rhs - lhs; }
inline Vec4f32 operator*(const float& lhs, const Vec4f32& rhs) noexcept { return rhs * lhs; }
inline Vec4f32 operator/(const float& lhs, const Vec4f32& rhs) noexcept { return rhs / lhs; }

std::ostream& operator<<(std::ostream& stream, const Vec4f32& rhs) {
    stream << '(' << rhs.x << ',' << rhs.y << ',' << rhs.z << ',' << rhs.w << ')';

    return stream;
}

struct Coloru8 {
    // I used multiple unions to avoid anonymous unions
    union { std::uint8_t r; std::uint8_t red; };
    union { std::uint8_t g; std::uint8_t green; };
    union { std::uint8_t b; std::uint8_t blue; };
    union { std::uint8_t a; std::uint8_t alpha; };
}; // struct Coloru8

// TODO:: USE _mm_load_ps INSTEAD OF _mm_loadu_ps
struct Mat4x4f32 {
    std::array<float, 16> m = { 0.f };

    inline       float& operator()(const size_t i)       noexcept { return this->m[i]; }
    inline const float& operator()(const size_t i) const noexcept { return this->m[i]; }

    inline       float& operator()(const size_t r, const size_t c)       noexcept { return (*this)(r * 4u + c); }
    inline const float& operator()(const size_t r, const size_t c) const noexcept { return (*this)(r * 4u + c); }
}; // struct Mat4x4f32

inline Mat4x4f32 operator*(const Mat4x4f32& lhs, const Mat4x4f32& rhs) noexcept {
    Mat4x4f32 result;

    for (char r = 0; r < 4u; r++)
        for (char c = 0; c < 4u; c++) // :-)
            for (char k = 0u; k < 4u; k++)
                result(r, c) += lhs(r, k) * rhs(k, c);
    //result(r, c) = _mm_cvtss_f32(_mm_dp_ps(_mm_loadu_ps(&lhs.m[r * 4u]), _mm_set_ps(rhs.m[c], rhs.m[c + 4u], rhs.m[c + 8], rhs.m[c + 12]), 0b11111111));

    return result;
}

inline Mat4x4f32 Transposed(const Mat4x4f32& m) noexcept {
    return Mat4x4f32{ {
        m(0), m(4), m(8),  m(12),
        m(1), m(5), m(9),  m(13),
        m(2), m(6), m(10), m(14),
        m(3), m(7), m(11), m(15)
    } };
}

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

std::ostream& operator<<(std::ostream& stream, const Mat4x4f32& m) {
    for (char r = 0; r < 4u; ++r) {
        for (char c = 0; c < 4u; ++c)
            stream << m(r, c) << ", ";
        stream << '\n';
    }

    return stream;
}

struct UV {
    float u, v;
}; // struct UV

struct Vertex {
    Vec4f32 position;
    UV      uv;
    float   lighting = 1.0f;
}; // struct Vertex

std::ostream& operator<<(std::ostream& stream, const Vertex& v) noexcept {
    stream << v.position;

    return stream;
}

enum class BlockType : std::uint8_t {
    STONE = 0u, DIRT, GRASS, AIR
}; // enum class BlockType

enum class BlockFace : std::uint8_t {
    TOP = 0u, FRONT, LEFT, RIGHT, BACK, BOTTOM
}; // enum class BlockFace

#define CHUNK_X_BLOCK_COUNT (16u)
#define CHUNK_Y_BLOCK_COUNT (255u)
#define CHUNK_Z_BLOCK_COUNT (16u)
#define BLOCK_LENGTH        (1.f)

#define RENDER_DISTANCE (2u) // in chunks

class Minecraft;

struct ChunkCoord {
    std::int32_t idx;
    std::int32_t idz;

    inline bool operator==(const ChunkCoord& rhs) const noexcept { return this->idx == rhs.idx && this->idz == rhs.idz; }
}; // struct ChunkCoord

class ChunkCoordHash {
public:
    inline size_t operator()(const ChunkCoord& cc) const
    {
        const int64_t a = static_cast<int64_t>(cc.idx) << 32;
        const int64_t b = static_cast<int64_t>(cc.idz) << 0;

        return a | b;
    }
};

class Chunk {
    friend Minecraft;
private:
    ChunkCoord m_location;

    std::array<std::array<std::array<BlockType, CHUNK_Z_BLOCK_COUNT>, CHUNK_Y_BLOCK_COUNT>, CHUNK_X_BLOCK_COUNT> m_blocks = { BlockType::AIR };

public:
    inline Chunk() noexcept = default;

    inline Chunk(const ChunkCoord& location) noexcept
        : m_location(location)
    {  }

    inline ChunkCoord GetLocation() const noexcept { return this->m_location; }

    inline std::optional<const BlockType*> GetBlock(const size_t idx, const size_t idy, const size_t idz) const noexcept {
        if (idx >= 0 && idy >= 0 && idz >= 0 && idx < CHUNK_X_BLOCK_COUNT && idy < CHUNK_Y_BLOCK_COUNT && idz < CHUNK_Z_BLOCK_COUNT) {
            return &this->m_blocks[idx][idy][idz];
        }

        return {  };
    }

    inline std::optional<BlockType*> GetBlock(const size_t idx, const size_t idy, const size_t idz) noexcept {
        if (idx >= 0 && idy >= 0 && idz >= 0 && idx < CHUNK_X_BLOCK_COUNT && idy < CHUNK_Y_BLOCK_COUNT && idz < CHUNK_Z_BLOCK_COUNT) {
            return &this->m_blocks[idx][idy][idz];
        }

        return {  };
    }

    inline void SetBlock(const size_t idx, const size_t idy, const size_t idz, const BlockType& type) noexcept {
        if (idx >= 0 && idy >= 0 && idz >= 0 && idx < CHUNK_X_BLOCK_COUNT && idy < CHUNK_Y_BLOCK_COUNT && idz < CHUNK_Z_BLOCK_COUNT) {
            this->m_blocks[idx][idy][idz] = type;
        }
    }

    void GenerateDefaultTerrain(const siv::PerlinNoise& noise) noexcept {
        for (size_t x = 0u; x < CHUNK_X_BLOCK_COUNT; ++x) {
        for (size_t z = 0u; z < CHUNK_Z_BLOCK_COUNT; ++z) {
        for (size_t y = 0u; y < CHUNK_Y_BLOCK_COUNT; ++y) {
            const size_t yMax = static_cast<size_t>(noise.normalizedOctaveNoise2D_0_1(x/50.f, z/50.f, 4) * CHUNK_Y_BLOCK_COUNT);

            if (y < yMax) {
                if (y < yMax - 5)
                    this->m_blocks[x][y][z] = BlockType::STONE;
                else if (y < yMax - 1) 
                    this->m_blocks[x][y][z] = BlockType::DIRT;
                else
                    this->m_blocks[x][y][z] = BlockType::GRASS;
            } else {
                this->m_blocks[x][y][z] = BlockType::AIR;
            }
        }
        }
        }
    }
}; // class Chunk

struct ChunkRenderData {
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
    size_t nVertices;
};

class Image {
private:
    std::unique_ptr<Coloru8[]> m_pBuffer;

    UINT m_width, m_height, m_nPixels;

public:
    inline Image() noexcept = default;

    Image(const wchar_t* filename) noexcept {
        Microsoft::WRL::ComPtr<IWICBitmapSource>      decodedConvertedFrame;
		Microsoft::WRL::ComPtr<IWICBitmapDecoder>     bitmapDecoder;
		Microsoft::WRL::ComPtr<IWICImagingFactory>    factory;
		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frameDecoder;

		if (CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))
			FATAL_ERROR("Failed to create IWICImagingFactory");

		if (factory->CreateDecoderFromFilename(filename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &bitmapDecoder))
			FATAL_ERROR("Failed to CreateDecoderFromFilename");

		if (bitmapDecoder->GetFrame(0, &frameDecoder))
			FATAL_ERROR("Failed to GetFrame the first frame of an image");

		if (frameDecoder->GetSize((UINT*)&this->m_width, (UINT*)&this->m_height))
			FATAL_ERROR("Failed to GetSize of an image");

		this->m_nPixels = this->m_width * this->m_height;
		if (WICConvertBitmapSource(GUID_WICPixelFormat32bppRGBA, frameDecoder.Get(), &decodedConvertedFrame))
			FATAL_ERROR("Failed to WICConvertBitmapSource");

		this->m_pBuffer = std::make_unique<Coloru8[]>(this->m_nPixels * sizeof(Coloru8));
		const WICRect sampleRect{ 0, 0, static_cast<INT>(this->m_width), static_cast<INT>(this->m_height) };
		if (decodedConvertedFrame->CopyPixels(&sampleRect, this->m_width * sizeof(Coloru8), this->m_nPixels * sizeof(Coloru8), (BYTE*)this->m_pBuffer.get()))
			FATAL_ERROR("Failed to CopyPixels from an image");
    }

    inline UINT     GetWidth()         const noexcept { return this->m_width;         }
    inline UINT     GetHeight()        const noexcept { return this->m_height;        }
    inline UINT     GetPixelCount()    const noexcept { return this->m_nPixels;       }
    inline Coloru8* GetBufferPointer() const noexcept { return this->m_pBuffer.get(); }
};

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

    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_pChunks;
    std::unordered_map<ChunkCoord, ChunkRenderData,        ChunkCoordHash> m_pChunkRenderData;

    siv::PerlinNoise m_noise;

public:
    Minecraft() noexcept :
        m_window("Minecraft", 1920u / 2, 1080u / 2),
        m_noise(std::random_device())
    {
        this->m_noise.reseed(1234);

        DXGI_SWAP_CHAIN_DESC scd = {  };
        scd.BufferCount = 1u;
        scd.BufferDesc.Width = this->m_window.GetWidth();
        scd.BufferDesc.Height = this->m_window.GetHeight();
        scd.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_SEQUENTIAL;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = this->m_window.GetHandle();
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
        scd.Windowed = true;

        if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL, 0u, nullptr, 0u, D3D11_SDK_VERSION, &scd, &this->m_pSwapChain, &this->m_pDevice, NULL, &m_pDeviceContext) != S_OK)
            FATAL_ERROR("Failed to create the device and the swap chain");

        Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
        if (this->m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer) != S_OK)
            FATAL_ERROR("Failed to get a frame buffer from the swap chain");

        if (this->m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &this->m_pRenderTargetView) != S_OK)
            FATAL_ERROR("Failed to create a render target view for the first swap chain's frame buffer");

        D3D11_VIEWPORT vp;
        vp.Width = static_cast<FLOAT>(this->m_window.GetWidth());
        vp.Height = static_cast<FLOAT>(this->m_window.GetHeight());
        vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f;
        vp.TopLeftX = 0.f;
        vp.TopLeftY = 0.f;

        this->m_pDeviceContext->RSSetViewports(1u, &vp);

        Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;

        const char* vsShaderCode = R"V0G0N(
            cbuffer VS_CONSTANT_BUFFER : register(b0) {
                matrix transform;
            };

            struct VS_OUTPUT {
                float4 position : SV_POSITION;
                float2 uv : UV;
                float lighting : LIGHTING;
            };

            VS_OUTPUT main(float4 position : POSITION, float2 uv : UV, float lighting : LIGHTING) {
                VS_OUTPUT result;
                result.position = mul(transform, position);
                result.uv = uv;
                result.lighting = lighting;

                return result;
            }
        )V0G0N";
        Microsoft::WRL::ComPtr<ID3DBlob> pVShaderByteCode;

        if (D3DCompile(vsShaderCode, strlen(vsShaderCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &pVShaderByteCode, &pErrorBlob) != S_OK)
            FATAL_ERROR("Failed to compile a vertex shader");

        if (this->m_pDevice->CreateVertexShader(pVShaderByteCode->GetBufferPointer(), pVShaderByteCode->GetBufferSize(), nullptr, &this->m_pVertexShader) != S_OK)
            FATAL_ERROR("Failed to create a vertex shader");

        const char* psShaderCode = R"V0G0N(
            Texture2D textureAtlas : register(t0);
            SamplerState samplerState : register(s0);

            float4 main(float3 position : POSITION, float2 uv : UV, float lighting : LIGHTING) : SV_TARGET {
                return float4(textureAtlas.Sample(samplerState, uv).xyz * lighting, 1.0f);
            }
        )V0G0N";
        Microsoft::WRL::ComPtr<ID3DBlob> pPShaderByteCode;

        if (D3DCompile(psShaderCode, strlen(psShaderCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pPShaderByteCode, &pErrorBlob) != S_OK)
            FATAL_ERROR("Failed to compile a vertex shader");

        if (this->m_pDevice->CreatePixelShader(pPShaderByteCode->GetBufferPointer(), pPShaderByteCode->GetBufferSize(), nullptr, &this->m_pPixelShader) != S_OK)
            FATAL_ERROR("Failed to create a vertex shader");

        std::array<D3D11_INPUT_ELEMENT_DESC, 3u> ieds = {};
        ieds[0].AlignedByteOffset = 0;
        ieds[0].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
        ieds[0].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
        ieds[0].SemanticName = "POSITION";
        ieds[0].InputSlot = 0;
        ieds[0].InstanceDataStepRate = 0;

        ieds[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        ieds[1].Format = DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT;
        ieds[1].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
        ieds[1].SemanticName = "UV";
        ieds[1].InputSlot = 0;
        ieds[1].InstanceDataStepRate = 0;

        ieds[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        ieds[2].Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
        ieds[2].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
        ieds[2].SemanticName = "LIGHTING";
        ieds[2].InputSlot = 0;
        ieds[2].InstanceDataStepRate = 0;

        if (this->m_pDevice->CreateInputLayout(ieds.data(), ieds.size(), pVShaderByteCode->GetBufferPointer(), pVShaderByteCode->GetBufferSize(), &this->m_pInputLayout) != S_OK)
            FATAL_ERROR("Failed to create an input layout for a vertex shader");

        D3D11_BUFFER_DESC cbd;
        cbd.ByteWidth = sizeof(Mat4x4f32);
        cbd.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        cbd.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbd.MiscFlags = 0;
        cbd.StructureByteStride = 0;

        if (this->m_pDevice->CreateBuffer(&cbd, nullptr, &this->m_pConstantBuffer) != S_OK)
            FATAL_ERROR("Failed to create a constant buffer");

        this->CreateDepthBuffer();
        this->LoadAndCreateTextureAtlas();
        this->InitWorld();

        size_t y = 0u;
        while (*this->GetBlock(ChunkCoord{0,0}, 0, y, 0).value() != BlockType::AIR)
            ++y;

        this->m_camera = Camera({ 0.f, (y + 1.f) * BLOCK_LENGTH, 0.f, 0.f }, M_PI_2, this->m_window.GetHeight() / (float)this->m_window.GetWidth(), 0.01f, 1000.f);
    }

private:
    void CreateDepthBuffer() noexcept {
        D3D11_TEXTURE2D_DESC td;
        td.Width  = this->m_window.GetWidth();
        td.Height = this->m_window.GetHeight();
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
        td.SampleDesc.Count   = 1;
        td.SampleDesc.Quality = 0;
        td.Usage     = D3D11_USAGE::D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        td.CPUAccessFlags = 0;
        td.MiscFlags      = 0;

        if (this->m_pDevice->CreateTexture2D(&td, nullptr, &this->m_pDepthStencilTexture) != S_OK)
            FATAL_ERROR("Failed to create a texture for a constant buffer");
        
        D3D11_DEPTH_STENCIL_DESC dsd;
        dsd.DepthEnable = true;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc = D3D11_COMPARISON_LESS;
        dsd.StencilEnable = true;
        dsd.StencilReadMask = 0xFF;
        dsd.StencilWriteMask = 0xFF;
        dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        if (this->m_pDevice->CreateDepthStencilState(&dsd, &this->m_pDepthStencilState) != S_OK)
            FATAL_ERROR("Failed to create a depth stencil state for a constant buffer");
        
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
        dsvd.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
        dsvd.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvd.Texture2D.MipSlice = 0;

        if (this->m_pDevice->CreateDepthStencilView(this->m_pDepthStencilTexture.Get(), &dsvd, &this->m_pDepthStencilView) != S_OK)
            FATAL_ERROR("Failed to create a depth stencil view");
    }

    void LoadAndCreateTextureAtlas() noexcept {
        this->m_textureAtlasImage = Image(L"texture_atlas.png");
        
        D3D11_TEXTURE2D_DESC textureAtlasDesc{};
        textureAtlasDesc.Width  = this->m_textureAtlasImage.GetWidth();
        textureAtlasDesc.Height = this->m_textureAtlasImage.GetHeight();
        textureAtlasDesc.CPUAccessFlags = 0;
        textureAtlasDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureAtlasDesc.MiscFlags = 0;
        textureAtlasDesc.Usage  = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
        textureAtlasDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
        textureAtlasDesc.ArraySize = 1;
        textureAtlasDesc.MipLevels = 1;
        textureAtlasDesc.SampleDesc.Count   = 1;
        textureAtlasDesc.SampleDesc.Quality = 0;

        D3D11_SUBRESOURCE_DATA textureAtlasInitialData;
        textureAtlasInitialData.pSysMem          = this->m_textureAtlasImage.GetBufferPointer();
        textureAtlasInitialData.SysMemPitch      = this->m_textureAtlasImage.GetWidth() * sizeof(Coloru8);
        textureAtlasInitialData.SysMemSlicePitch = 0;

        if (this->m_pDevice->CreateTexture2D(&textureAtlasDesc, &textureAtlasInitialData, &this->m_pTextureAtlas) != S_OK)
            FATAL_ERROR("Failed to create a 2d texture");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
        srvd.Format = textureAtlasDesc.Format;
        srvd.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
        srvd.Texture2D.MipLevels = 1;
        srvd.Texture2D.MostDetailedMip = 0;
        if (this->m_pDevice->CreateShaderResourceView(this->m_pTextureAtlas.Get(), &srvd, &this->m_pTextureAtlasSRV) != S_OK)
            FATAL_ERROR("Failed to create a shader resource view for a 2d texture");

        D3D11_SAMPLER_DESC sd;
        sd.Filter   = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
        sd.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
        sd.MipLODBias = 0.0f;
		sd.MinLOD = 0.0f;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

        if (this->m_pDevice->CreateSamplerState(&sd, &this->m_pTextureAtlasSamplerState) != S_OK)
            FATAL_ERROR("Failed to create a sampler state");
    }

    void InitWorld() noexcept {
        this->m_pChunks.insert({ ChunkCoord{0, 0}, std::make_unique<Chunk>(ChunkCoord{0, 0}) });
        this->m_pChunks.at(ChunkCoord{ 0, 0 })->GenerateDefaultTerrain(this->m_noise);
        this->GenerateChunkMesh(this->GetChunk(ChunkCoord{0,0}).value());
    }

private:
    std::optional<Chunk*> GetChunk(const ChunkCoord& location) noexcept {
        const auto pChunkIterator = this->m_pChunks.find(location);
        if (pChunkIterator != this->m_pChunks.end()) {
            return (*pChunkIterator).second.get();
        }

        return {  };
    }

    std::optional<BlockType*> GetBlock(const ChunkCoord& chunkLocation, const size_t idx, const size_t idy, const size_t idz) {
        const std::optional<Chunk*>& chunkOpt = this->GetChunk(chunkLocation);

        if (!chunkOpt.has_value()) return {  };

        return chunkOpt.value()->GetBlock(idx, idy, idz);
    }

    void GenerateChunkMesh(const Chunk* const pChunk) noexcept {
        std::vector<Vertex> vertices(CHUNK_X_BLOCK_COUNT * CHUNK_Y_BLOCK_COUNT * CHUNK_Z_BLOCK_COUNT);
        size_t nVertices = 0u;

        BlockType airBlock = BlockType::AIR;

        for (size_t x = 0u; x < CHUNK_X_BLOCK_COUNT; ++x) {
        for (size_t y = 0u; y < CHUNK_Y_BLOCK_COUNT; ++y) {
        for (size_t z = 0u; z < CHUNK_Z_BLOCK_COUNT; ++z) {
            const BlockType& block = *pChunk->GetBlock(x, y, z).value();

            if (block != BlockType::AIR) {
                const Vec4f32 baseCornerPoint = {
                    BLOCK_LENGTH * (x + pChunk->GetLocation().idx * (std::int16_t)CHUNK_X_BLOCK_COUNT),
                    (y + 1) * BLOCK_LENGTH,
                    BLOCK_LENGTH * (z + pChunk->GetLocation().idz * (std::int16_t)CHUNK_Z_BLOCK_COUNT),
                    1.f
                };

                const float baseUInc = 16.f / this->m_textureAtlasImage.GetWidth();
                const float baseVInc = 16.f / this->m_textureAtlasImage.GetHeight();

                const float baseU = static_cast<float>(block) * baseUInc;

                // in clockwise order with "a" in the top left position
                const auto AddFace = [baseUInc, baseVInc, &nVertices, &baseU, &vertices, this](const Vec4f32& a, const Vec4f32& b, const Vec4f32& c, const Vec4f32& e, const BlockFace& face) {
                    const float baseV = static_cast<float>(face) * baseVInc;

                    float lightingPerFace[6] = { 1.0f, 0.9f, 0.5f, 0.9f, 0.5f, 0.25f };

                    const float faceLighting = lightingPerFace[static_cast<std::uint8_t>(face)];

                    vertices[nVertices++] = Vertex{a, UV{baseU,            baseV},            faceLighting};
                    vertices[nVertices++] = Vertex{b, UV{baseU + baseUInc, baseV},            faceLighting};
                    vertices[nVertices++] = Vertex{c, UV{baseU + baseUInc, baseV + baseVInc}, faceLighting};

                    vertices[nVertices++] = Vertex{a, UV{baseU,            baseV},            faceLighting};
                    vertices[nVertices++] = Vertex{c, UV{baseU + baseUInc, baseV + baseVInc}, faceLighting};
                    vertices[nVertices++] = Vertex{e, UV{baseU,            baseV + baseVInc}, faceLighting};
                };

                // Front
                if (*this->GetBlock(pChunk->GetLocation(), x, y, z - 1).value_or(&airBlock) == BlockType::AIR) {
                    AddFace(baseCornerPoint,
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0,            +0},
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +0},
                            baseCornerPoint + Vec4f32{+0.f,          -BLOCK_LENGTH, +0}, BlockFace::FRONT);               
                }

                // Back
                if (*this->GetBlock(pChunk->GetLocation(), x, y, z + 1).value_or(&airBlock) == BlockType::AIR) {
                    AddFace(baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0,            +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+0,            +0,            +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+0.f,          -BLOCK_LENGTH, +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +BLOCK_LENGTH}, BlockFace::FRONT);               
                }

                // Left
                if (*this->GetBlock(pChunk->GetLocation(), x - 1, y, z).value_or(&airBlock) == BlockType::AIR) {
                    AddFace(baseCornerPoint + Vec4f32{0, +0,            +BLOCK_LENGTH},
                            baseCornerPoint,
                            baseCornerPoint + Vec4f32{0, -BLOCK_LENGTH, +0},
                            baseCornerPoint + Vec4f32{0, -BLOCK_LENGTH, +BLOCK_LENGTH}, BlockFace::LEFT);               
                }

                // Right
                if (*this->GetBlock(pChunk->GetLocation(), x + 1, y, z).value_or(&airBlock) == BlockType::AIR) {
                    AddFace(baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0,            +0},       
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0,            +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +0}, BlockFace::RIGHT);
                }
                
                // Top
                if (*this->GetBlock(pChunk->GetLocation(), x, y + 1, z).value_or(&airBlock) == BlockType::AIR) {
                    AddFace(baseCornerPoint + Vec4f32{+0,            +0, +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, +0, +0},
                            baseCornerPoint, BlockFace::TOP);
                }

                // Bottom
                if (*this->GetBlock(pChunk->GetLocation(), x, y - 1, z).value_or(&airBlock) == BlockType::AIR) {
                    AddFace(baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+0,            -BLOCK_LENGTH, +BLOCK_LENGTH},
                            baseCornerPoint + Vec4f32{+0,            -BLOCK_LENGTH, +0},
                            baseCornerPoint + Vec4f32{+BLOCK_LENGTH, -BLOCK_LENGTH, +0}, BlockFace::TOP);
                }
            }
        }
        }
        }
    
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.ByteWidth = nVertices * sizeof(Vertex);
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = sizeof(Vertex);
        bufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    
        D3D11_SUBRESOURCE_DATA sd = {};
        sd.pSysMem = vertices.data();
        sd.SysMemPitch = 0;
        sd.SysMemSlicePitch = 0;
    
        auto pVertexBufferIt = this->m_pChunkRenderData.find(pChunk->GetLocation());
        if (pVertexBufferIt == this->m_pChunkRenderData.end()) {
            this->m_pChunkRenderData.insert({ pChunk->GetLocation(), ChunkRenderData{{}, nVertices} });
            pVertexBufferIt = this->m_pChunkRenderData.find(pChunk->GetLocation());
        }

        if (this->m_pDevice->CreateBuffer(&bufferDesc, &sd, (*pVertexBufferIt).second.pVertexBuffer.GetAddressOf()) != S_OK)
            FATAL_ERROR("Failed to create a vertex buffer");
    }

    void Update() noexcept {
        this->m_window.Update();

        float speed = 0.1f;
        if (this->m_window.IsKeyDown('A'))
            this->m_camera.Translate(Vec4f32(-speed, 0.f, 0.f, 0.f));
        if (this->m_window.IsKeyDown('D'))
            this->m_camera.Translate(Vec4f32(+speed, 0.f, 0.f, 0.f));
        if (this->m_window.IsKeyDown(VK_SPACE))
            this->m_camera.Translate(Vec4f32(0.f, speed, 0.f, 0.f));
        if (this->m_window.IsKeyDown(VK_SHIFT))
            this->m_camera.Translate(Vec4f32(0.f, -speed, 0.f, 0.f));
        if (this->m_window.IsKeyDown('W'))
            this->m_camera.Translate(Vec4f32(0.f, 0.f, speed, 0.f));
        if (this->m_window.IsKeyDown('S'))
            this->m_camera.Translate(Vec4f32(0.f, 0.f, -speed, 0.f));

        this->m_camera.CalculateTransform();

        D3D11_MAPPED_SUBRESOURCE resource;
        if (this->m_pDeviceContext->Map(this->m_pConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &resource) != S_OK)
            FATAL_ERROR("Failed to map constant buffer memory");

        const Mat4x4f32 tTransform = this->m_camera.GetTransform();

        std::memcpy(resource.pData, &tTransform, sizeof(tTransform));
        this->m_pDeviceContext->Unmap(this->m_pConstantBuffer.Get(), 0u);
    }

    void Render() {
        float clearColor[4] = { 1.f, 0.f, 0.f, 1.f };
        this->m_pDeviceContext->ClearRenderTargetView(this->m_pRenderTargetView.Get(), clearColor);
        this->m_pDeviceContext->ClearDepthStencilView(this->m_pDepthStencilView.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.f, 0u);

        this->m_pDeviceContext->OMSetRenderTargets(1u, this->m_pRenderTargetView.GetAddressOf(), this->m_pDepthStencilView.Get());

        const UINT stride = sizeof(Vertex);
        const UINT offset = 0;

        std::vector<ID3D11Buffer*> pVertexBuffers(this->m_pChunks.size());
        size_t i = 0;
        size_t vertexCount = 0u;
        for (const std::pair<ChunkCoord, ChunkRenderData>& p : this->m_pChunkRenderData) {
            const ChunkRenderData& crd = p.second;

            pVertexBuffers[i++] =  crd.pVertexBuffer.Get();
            vertexCount         += crd.nVertices;
        }

        this->m_pDeviceContext->IASetVertexBuffers(0u, pVertexBuffers.size(), pVertexBuffers.data(), &stride, &offset);
        this->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        this->m_pDeviceContext->IASetInputLayout(this->m_pInputLayout.Get());

        this->m_pDeviceContext->VSSetShader(this->m_pVertexShader.Get(), nullptr, 0u);
        this->m_pDeviceContext->PSSetShader(this->m_pPixelShader.Get(), nullptr, 0u);
        this->m_pDeviceContext->VSSetConstantBuffers(0u, 1u, this->m_pConstantBuffer.GetAddressOf());
        
        this->m_pDeviceContext->PSSetSamplers(0u, 1u, this->m_pTextureAtlasSamplerState.GetAddressOf());
        this->m_pDeviceContext->PSSetShaderResources(0u, 1u, this->m_pTextureAtlasSRV.GetAddressOf());

        this->m_pDeviceContext->Draw(vertexCount, 0u);

        this->m_pSwapChain->Present(1u, 0u);
    }

public:
    void Run() noexcept {
        while (this->m_window.IsRunning()) {
            this->Update();
            this->Render();
        }
    }
}; // class Minecraft

int main(int argc, char** argv) {
    Minecraft minecraft;
    minecraft.Run();

    return 0;
}