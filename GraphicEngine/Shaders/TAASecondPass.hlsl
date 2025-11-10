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
#include "Common.hlsl"

Texture2D<float4> gCurrentTexture : register(t0, space0);
Texture2D<float4> gPreviousTexture : register(t0, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);


// Constant data that varies per frame.
cbuffer cbPass : register(b0)
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

struct VertexIn
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexOut VS(uint vertexID : SV_VertexID, VertexIn vIn)
{
    VertexOut vOut = (VertexOut) 0.0f;
    
    vOut.uv = float2((vertexID << 1) & 2, vertexID & 2);
    vOut.position = float4(vOut.uv * 2.0 - 1.0, 0.0, 1.0);
    
    vOut.position.y *= -1;
    
    return vOut;
}


float4 PS(VertexOut vOut) : SV_TARGET
{
    float2 uv = vOut.uv;
    float3 currentColor = gCurrentTexture.Sample(gsamLinearWrap, uv);
    float3 previousColor = gPreviousTexture.Sample(gsamLinearWrap, uv);
    float4 litColor = float4(currentColor * 0.1f + previousColor * 0.9f, 1.0f);
    
    return litColor;
}