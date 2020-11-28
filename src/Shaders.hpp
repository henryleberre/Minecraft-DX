#ifndef __MINECRAFT__SHADERS_HPP
#define __MINECRAFT__SHADERS_HPP

const char* vsBlockCode = R"V0G0N(
    cbuffer VS_CONSTANT_BUFFER : register(b0) {
        matrix transform;
    };

    struct VS_OUTPUT {
        float4 position  : SV_POSITION;
        float2 uv        : UV;
        float  lighting  : LIGHTING;
    };

    VS_OUTPUT main(float4 position : POSITION, float2 uv : UV, float lighting : LIGHTING) {
        VS_OUTPUT result;
        result.position = mul(transform, position);
        result.uv       = uv;
        result.lighting = lighting;

        return result;
    }
)V0G0N";

const char* psBlockCode = R"V0G0N(
    Texture2D    textureAtlas : register(t0);
    SamplerState samplerState : register(s0);

    float4 main(float4 position : POSITION, float2 uv : UV, float lighting : LIGHTING) : SV_TARGET {
        return float4(textureAtlas.Sample(samplerState, uv).xyz * lighting, 1.0f);
    }
)V0G0N";

#endif // #ifndef __MINECRAFT__SHADERS_HPP