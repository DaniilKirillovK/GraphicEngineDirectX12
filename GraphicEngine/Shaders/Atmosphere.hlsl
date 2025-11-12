#include "AtmosphereUtil.hlsl"

struct VertexOut
{
    float2 Tex : TEXCOORD;
    float4 Pos : SV_Position;
};

Texture2D<float4> gDepthTexture : register(t0, space0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);


float4 PS(in VertexOut Input) : SV_Target0
{
    float2 UV = Input.Tex;
    float4 NDCPos = float4(2 * UV.x - 1.0, 1 - 2 * UV.y, 1.0, 1.0);
    float4 ViewPos = mul(NDCPos, InvProjectionMatrix);
    ViewPos /= ViewPos.w;
    float depth = gDepthTexture.Sample(gsamPointWrap, UV).r;
    
    float3 WorldPos;
    WorldPos = ReconstructWorldPos(UV, depth);
    
    float3 transmittance = float3(1.0f, 1.0f, 1.0f);

    float3 rayOrigin = CameraPosition;
    float3 rayDir = normalize(WorldPos - CameraPosition);
    
    float maxDistance = length(WorldPos - CameraPosition);
    
    float3 scattering = CalculateAtmosphericScattering(rayOrigin, rayDir, maxDistance, transmittance);
    
    scattering *= 10.f;
    
    if (depth > 0.99999f)
    {
        float3 finalColor = scattering;
        finalColor = finalColor / (finalColor + 1.0);
        return float4(finalColor, 1.0f);
    }
    else
    {
        float3 finalColor = scattering;
        finalColor = finalColor / (finalColor + 1.0);
        float alpha = 1.0 - min(transmittance.r, min(transmittance.g, transmittance.b));
        
        return float4(finalColor, alpha);
    }
}


VertexOut VS(in uint VertID : SV_VertexID)
{
    VertexOut Output;
    float2 Tex = float2(uint2(VertID, VertID << 1) & 2);
    Output.Tex = Tex;
    Output.Pos = float4(lerp(float2(-1, 1), float2(1, -1), Tex), 0, 1);
    return Output;
}