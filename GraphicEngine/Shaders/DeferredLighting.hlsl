#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 3
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#include "LightingUtil.hlsl"
#include "Common.hlsl"

Texture2D<float4> gAlbedo : register(t0, space0);
Texture2D<float4> gPosition : register(t1, space0);
Texture2D<float4> gNormal : register(t2, space0);
Texture2D<float4> gSpecular : register(t3, space0);
Texture2D<float4> gDepth : register(t4, space0);

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

VertexOut VSMain(uint vertexID : SV_VertexID, VertexIn vIn)
{
    VertexOut vOut = (VertexOut) 0.0f;
    
    vOut.uv = float2((vertexID << 1) & 2, vertexID & 2);
    vOut.position = float4(vOut.uv * 2.0 - 1.0, 0.0, 1.0);
    
    vOut.position.y *= -1;
    
    return vOut;
}


float4 PSMain(VertexOut vOut) : SV_TARGET
{
    float4 albedo = gAlbedo.Sample(gsamPointWrap, vOut.uv);
    float3 worldPos = gPosition.Sample(gsamPointWrap, vOut.uv).xyz;
    float3 normal = gNormal.Sample(gsamPointWrap, vOut.uv).xyz;
    float3 normalResult = normalize(normal.rgb * 2.0 - 1.0);
    
    float Roughness = gSpecular.Sample(gsamPointWrap, vOut.uv).x;
    float AO = gSpecular.Sample(gsamPointWrap, vOut.uv).w;
    
    float depth = gDepth.Sample(gsamPointWrap, vOut.uv).r;
    
    float3 toEyeW = normalize(gEyePosW - worldPos);

    // Light terms.
    float4 ambient = gAmbientLight * albedo;

    float3 gFresnelR0 = float3(0.01f, 0.01f, 0.01f);
    
    const float shininess = 1.0f - Roughness;
    Material mat = { albedo, gFresnelR0, shininess };
    float3 shadowFactor = float3(AO, AO, AO);
    float4 directLight = ComputeLighting(gLights, mat, worldPos,
        normal, toEyeW, shadowFactor);
    
    float4 litColor = ambient + directLight;
    
    return litColor;
}