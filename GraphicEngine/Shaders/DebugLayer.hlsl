#include "LightingUtil.hlsl"

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct GBuffer
{
    float4 Albedo : SV_TARGET0;
    float4 Position : SV_TARGET1;
    float4 Normal : SV_TARGET2;
    float4 Specular : SV_TARGET3;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    float isTesselationNeeded;
    float scale;
};

// Constant data that varies per frame.
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

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
	// Transform to homogeneous clip space.
    float4 posW = mul(float4(vin.PosL * scale, 1.0f), gWorld);
    vout.PosW = posW;
    vout.PosH = mul(posW, gViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return vout;
}

GBuffer PS(VertexOut pin)
{
    GBuffer gBuffer;
    gBuffer.Position = float4(pin.PosW, 1.0f);
    gBuffer.Albedo = pin.Color;
    gBuffer.Normal = float4(0.0f, 0.0f, 0.0f, 0.0f);
    gBuffer.Specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return gBuffer;
}