#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#include "LightingUtil.hlsl"

Texture2D<float4> gTexture : register(t0, space0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    float isTessellationNeeded;
    float scale;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 cbPerObjectPad2;
    
    float tessFactor;
    float pixelationFactor;
    float isParallaxMapping;
    float displacementLevel;
    
    float isNegative;
    float3 cbPad;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    
    float4 worldPos = mul(gWorld, float4(input.position, 1.0f));
    output.worldPos = worldPos.xyz;
    output.position = mul(worldPos, gViewProj);
    output.normal = normalize(mul((float3x3) gWorld, input.normal));
    
    return output;
}

float4 PS(VertexOutput input) : SV_TARGET
{
    float2 xCoord = (input.worldPos.zy + 150.f) / 300.f;
    float2 yCoord = (input.worldPos.xz + 150.f) / 300.f;
    float2 zCoord = (input.worldPos.xy + 150.f) / 300.f;
    
    float4 xSample = gTexture.Sample(gsamLinearWrap, xCoord);
    float4 ySample = gTexture.Sample(gsamLinearWrap, yCoord);
    float4 zSample = gTexture.Sample(gsamLinearWrap, zCoord);
    
    float4 Position = float4(input.worldPos, 1.0f);
    float3 Normal = -input.normal;
    
    float3 weights = abs(Normal);
    weights = pow(weights, 2.0f);
    weights = weights / (weights.x + weights.y + weights.z);
    
    float4 Color = xSample * weights.x + ySample * weights.y + zSample * weights.z;
    
    float3 Roughness = float3(0.1f, 0.1f, 0.1f);
    float3 AO = float3(0.5f, 0.5f, 0.5f);
    
    float3 toEyeW = normalize(gEyePosW - Position.xyz);
    float4 ambient = gAmbientLight * Color;

    float3 gFresnelR0 = float3(0.01f, 0.01f, 0.01f);
    
    const float shininess = 1.0f - Roughness.x;
    Material mat = { Color, gFresnelR0, shininess };
    float3 shadowFactor = float3(AO.x, AO.x, AO.x);
    float4 directLight = ComputeLighting(gLights, mat, Position.xyz,
        Normal.xyz, toEyeW, shadowFactor);
    
    float4 litColor = ambient + directLight * 1.5f;

    return litColor;
}