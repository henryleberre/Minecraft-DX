#include "Minecraft.hpp"

Minecraft::Minecraft() noexcept : m_window("Minecraft", 1920u, 1080u),
                                  m_noise(std::random_device()),
                                  m_camera(Camera(Vec4f32{0.f, 40, 0.01f, 1000.f}, M_PI_2, 9.f / 16.f, 0.1f, 1000.f))
{
    this->m_window.ClipCursor();
    this->m_window.HideCursor();

    this->m_noise.reseed(1234);

    DXGI_SWAP_CHAIN_DESC scd = {};
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
    Microsoft::WRL::ComPtr<ID3DBlob> pVShaderByteCode;

    if (D3DCompile(vsBlockCode, strlen(vsBlockCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &pVShaderByteCode, &pErrorBlob) != S_OK)
        FATAL_ERROR("Failed to compile a vertex shader");

    if (this->m_pDevice->CreateVertexShader(pVShaderByteCode->GetBufferPointer(), pVShaderByteCode->GetBufferSize(), nullptr, &this->m_pVertexShader) != S_OK)
        FATAL_ERROR("Failed to create a vertex shader");

    Microsoft::WRL::ComPtr<ID3DBlob> pPShaderByteCode;

    if (D3DCompile(psBlockCode, strlen(psBlockCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pPShaderByteCode, &pErrorBlob) != S_OK)
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

    if (this->m_pDevice->CreateInputLayout(ieds.data(), static_cast<UINT>(ieds.size()), pVShaderByteCode->GetBufferPointer(), pVShaderByteCode->GetBufferSize(), &this->m_pInputLayout) != S_OK)
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
}

void Minecraft::CreateDepthBuffer() noexcept
{
    D3D11_TEXTURE2D_DESC td;
    td.Width = this->m_window.GetWidth();
    td.Height = this->m_window.GetHeight();
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;

    if (this->m_pDevice->CreateTexture2D(&td, nullptr, &this->m_pDepthStencilTexture) != S_OK)
        FATAL_ERROR("Failed to create a texture for a constant buffer");

    D3D11_DEPTH_STENCIL_DESC dsd{};
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

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
    dsvd.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvd.Texture2D.MipSlice = 0;

    if (this->m_pDevice->CreateDepthStencilView(this->m_pDepthStencilTexture.Get(), &dsvd, &this->m_pDepthStencilView) != S_OK)
        FATAL_ERROR("Failed to create a depth stencil view");
}

void Minecraft::LoadAndCreateTextureAtlas() noexcept
{
    this->m_textureAtlasImage = Image(L"texture_atlas.png");

    D3D11_TEXTURE2D_DESC textureAtlasDesc{};
    textureAtlasDesc.Width = this->m_textureAtlasImage.GetWidth();
    textureAtlasDesc.Height = this->m_textureAtlasImage.GetHeight();
    textureAtlasDesc.CPUAccessFlags = 0;
    textureAtlasDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureAtlasDesc.MiscFlags = 0;
    textureAtlasDesc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
    textureAtlasDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    textureAtlasDesc.ArraySize = 1;
    textureAtlasDesc.MipLevels = 1;
    textureAtlasDesc.SampleDesc.Count = 1;
    textureAtlasDesc.SampleDesc.Quality = 0;

    D3D11_SUBRESOURCE_DATA textureAtlasInitialData;
    textureAtlasInitialData.pSysMem = this->m_textureAtlasImage.GetBufferPointer();
    textureAtlasInitialData.SysMemPitch = this->m_textureAtlasImage.GetWidth() * sizeof(Coloru8);
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
    sd.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
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

void Minecraft::UpdateWorld() noexcept
{
    const Vec4f32 cameraPosition = this->m_camera.GetPosition();

    for (Chunk* pChunk : this->m_pChunksToRender) {
        const ChunkCoord cc = pChunk->GetLocation();

        if (cc.idx <  (cameraPosition.x / BLOCK_LENGTH) / CHUNK_X_BLOCK_COUNT - RENDER_DISTANCE - 1 ||
            cc.idx >= (cameraPosition.x / BLOCK_LENGTH) / CHUNK_X_BLOCK_COUNT + RENDER_DISTANCE     ||
            cc.idz <  (cameraPosition.z / BLOCK_LENGTH) / CHUNK_Z_BLOCK_COUNT - RENDER_DISTANCE - 1 ||
            cc.idz >= (cameraPosition.z / BLOCK_LENGTH) / CHUNK_Z_BLOCK_COUNT + RENDER_DISTANCE) {
           pChunk->UnloadDXMesh();
        }
    }

    this->m_pChunksToRender.clear();

    

    ChunkCoord cc;
    for (cc.idx = (cameraPosition.x / BLOCK_LENGTH) / CHUNK_X_BLOCK_COUNT - RENDER_DISTANCE - 1; cc.idx < (cameraPosition.x / BLOCK_LENGTH) / CHUNK_X_BLOCK_COUNT + RENDER_DISTANCE; ++cc.idx) {
        for (cc.idz = (cameraPosition.z / BLOCK_LENGTH) / CHUNK_Z_BLOCK_COUNT - RENDER_DISTANCE - 1; cc.idz < (cameraPosition.z / BLOCK_LENGTH) / CHUNK_Z_BLOCK_COUNT + RENDER_DISTANCE; ++cc.idz) {
            std::optional<Chunk *> pChunkOpt = this->GetChunk(cc);

            if (!pChunkOpt.has_value()) {
                this->m_pChunks.insert({cc, std::make_unique<Chunk>(cc)});
                pChunkOpt = this->GetChunk(cc);
                
                pChunkOpt.value()->GenerateDefaultTerrain(this->m_noise);
            }

            if (!pChunkOpt.value()->HasDXMesh())
                pChunkOpt.value()->GenerateDXMesh(this->m_pDevice, this->m_textureAtlasImage.GetWidth(), this->m_textureAtlasImage.GetHeight());

            this->m_pChunksToRender.push_back(pChunkOpt.value());
        }
    }
}

void Minecraft::Update() noexcept
{
    this->m_window.Update();

    this->m_camera.Rotate(Vec4f32{(float)this->m_window.GetMouseYDelta() / 4000.f, (float)this->m_window.GetMouseXDelta() / 4000.f, 0.f, 0.f});
    this->m_camera.Update();

    float speed = 0.1f;
    if (this->m_window.IsKeyDown('A'))
        this->m_camera.Translate(-1.f * this->m_camera.GetRightVector() * speed);
    if (this->m_window.IsKeyDown('D'))
        this->m_camera.Translate(this->m_camera.GetRightVector() * speed);
    if (this->m_window.IsKeyDown(VK_SPACE))
        this->m_camera.Translate(Vec4f32(0.f, speed, 0.f, 0.f));
    if (this->m_window.IsKeyDown(VK_SHIFT))
        this->m_camera.Translate(Vec4f32(0.f, -speed, 0.f, 0.f));
    if (this->m_window.IsKeyDown('W'))
        this->m_camera.Translate(this->m_camera.GetForwardVector() * speed);
    if (this->m_window.IsKeyDown('S'))
        this->m_camera.Translate(-speed * this->m_camera.GetForwardVector());

    this->m_camera.Update();
    this->UpdateWorld();

    D3D11_MAPPED_SUBRESOURCE resource;
    if (this->m_pDeviceContext->Map(this->m_pConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &resource) != S_OK)
        FATAL_ERROR("Failed to map constant buffer memory");

    const Mat4x4f32 tTransform = this->m_camera.GetTransform();

    std::memcpy(resource.pData, &tTransform, sizeof(tTransform));
    this->m_pDeviceContext->Unmap(this->m_pConstantBuffer.Get(), 0u);
}

void Minecraft::Render() noexcept
{
    float clearColor[4] = {0.2284f, 0.3486f, 0.4230f, 1.f};
    this->m_pDeviceContext->ClearRenderTargetView(this->m_pRenderTargetView.Get(), clearColor);
    this->m_pDeviceContext->ClearDepthStencilView(this->m_pDepthStencilView.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.f, 0u);

    this->m_pDeviceContext->OMSetRenderTargets(1u, this->m_pRenderTargetView.GetAddressOf(), this->m_pDepthStencilView.Get());

    const UINT stride = sizeof(Vertex);
    const UINT offset = 0;

    this->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    this->m_pDeviceContext->IASetInputLayout(this->m_pInputLayout.Get());

    this->m_pDeviceContext->VSSetShader(this->m_pVertexShader.Get(), nullptr, 0u);
    this->m_pDeviceContext->PSSetShader(this->m_pPixelShader.Get(), nullptr, 0u);
    this->m_pDeviceContext->VSSetConstantBuffers(0u, 1u, this->m_pConstantBuffer.GetAddressOf());

    this->m_pDeviceContext->PSSetSamplers(0u, 1u, this->m_pTextureAtlasSamplerState.GetAddressOf());
    this->m_pDeviceContext->PSSetShaderResources(0u, 1u, this->m_pTextureAtlasSRV.GetAddressOf());
    for (const Chunk* pChunk : this->m_pChunksToRender)
    {
        this->m_pDeviceContext->IASetVertexBuffers(0u, 1u, pChunk->m_dxData.value().pVertexBuffer.GetAddressOf(), &stride, &offset);
        this->m_pDeviceContext->Draw(static_cast<UINT>(pChunk->m_dxData.value().nVertices), 0u);
    }

    this->m_pSwapChain->Present(1u, 0u);
}