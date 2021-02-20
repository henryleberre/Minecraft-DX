#ifndef __MINECRAFT__SHADERS_HPP
#define __MINECRAFT__SHADERS_HPP

inline const char* vsBlockCode = R"V0G0N(
    cbuffer VS_CONSTANT_BUFFER : register(b0) {
        matrix transform;
    };

    struct VS_OUTPUT {
        float4 position  : SV_POSITION;
        float2 uv        : UV;
        float  lighting  : LIGHTING;
        float  fog       : FOG;
    };

    VS_OUTPUT main(float4 position : POSITION, float2 uv : UV, float lighting : LIGHTING) {
        VS_OUTPUT result;
        result.position = mul(transform, position);
        result.uv       = uv;
        result.lighting = lighting;

        result.fog      = 1.0 - 1.0/pow(2.71, 0.0025 * distance(position, float4(0.f, position.y, 0.f, 0.f)));

        return result;
    }
)V0G0N";

inline const char* psBlockCode = R"V0G0N(
    Texture2D    textureAtlas : register(t0);
    SamplerState samplerState : register(s0);

    float4 main(float4 position : POSITION, float2 uv : UV, float lighting : LIGHTING, float fog : FOG) : SV_TARGET {
        return float4(textureAtlas.Sample(samplerState, uv).xyz * lighting * (1 - fog) + fog * float3(1.f, 1.f, 1.f), 1.0f);
    }
)V0G0N";

#endif // #ifndef __MINECRAFT__SHADERS_HPP