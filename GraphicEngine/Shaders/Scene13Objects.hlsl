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

Texture2D<float4> gTextures[1] : register(t0, space0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct InstanceData
{
    float4x4 WorldMatrix;
    float4 Color;
};

StructuredBuffer<InstanceData> instanceBuffer : register(t0, space1);

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
    
    float isNegative;
    float3 cbPad;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

// Constant data that varies per material.
cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;
    float4x4 gMatTransform;
    float tilesCount;
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
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
    VertexOut vout = (VertexOut) 0.0f;
    
    InstanceData instanceData = instanceBuffer[instanceID];
   
    // Transform to world space.
    float4 posW = mul(mul(float4(vin.PosL * scale, 1.0f), gWorld), instanceData.WorldMatrix);

    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
    
    vout.TangentW = mul(vin.TangentU, (float3x3) gWorld);
	
	// Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC * tilesCount, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;
    vout.Color = instanceData.Color;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 Position = float4(pin.PosW, 1.0f);
    
    float2 TexCoord = pin.TexC;
    float4 Albedo = float4((gTextures[0].Sample(gsamLinearWrap, TexCoord) * gDiffuseAlbedo * pin.Color).xyz, 1.0f);
    
    float3 normalMap = pin.NormalW;
    
    float4 Normal = float4(normalMap, 1.0f);
    
    float3 Roughness = float3(gRoughness, gRoughness, gRoughness);
    float3 AO = float3(0.5f, 0.5f, 0.5f);
    
    float3 toEyeW = normalize(gEyePosW - Position.xyz);

    // Light terms.
    float4 ambient = gAmbientLight * Albedo;

    float3 gFresnelR0 = float3(0.01f, 0.01f, 0.01f);
    
    const float shininess = 1.0f - Roughness.x;
    Material mat = { Albedo, gFresnelR0, shininess };
    float3 shadowFactor = float3(AO.x, AO.x, AO.x);
    float4 directLight = ComputeLighting(gLights, mat, Position.xyz,
        Normal.xyz, toEyeW, shadowFactor);
    
    float4 litColor = ambient + directLight;

    return litColor;
}