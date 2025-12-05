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
#include "LightingUtilPBR.hlsl"
#include "Common.hlsl"

Texture2D<float4> gAlbedoTex : register(t0, space0);
Texture2D<float4> gNormalTex : register(t1, space0);
Texture2D<float4> gMetallicTex : register(t2, space0);
Texture2D<float4> gRoughnessTex : register(t3, space0);
Texture2D<float4> gAOTex : register(t4, space0);

struct TAABuffer
{
    float4 MainRTV : SV_TARGET0;
    float4 VelocityRTV : SV_TARGET1;
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);


cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    float4x4 gPrevWorld;
    float isTessellationNeeded;
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
    
    float4x4 gPrevView;
    float4x4 gPrevInvView;
    float4x4 gPrevProj;
    float4x4 gPrevInvProj;
    float4x4 gPrevViewProj;
    float4x4 gPrevInvViewProj;
    
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
    float IsMetallic;
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
    float4 PrevPosH : TEXCOORD1;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD0;
    float3 TangentW : TANGENT;
};

float2 CalcVelocity(float4 currentClipPos, float4 previousClipPos)
{
    float3 currentNDC = currentClipPos.xyz / currentClipPos.w;
    float3 previousNDC = previousClipPos.xyz / previousClipPos.w;
    
    float2 currentUV = currentNDC.xy * 0.5 + 0.5;
    float2 previousUV = previousNDC.xy * 0.5 + 0.5;
    
    float2 uvVelocity = currentUV - previousUV;
    
    return uvVelocity;
}

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
    VertexOut vout = (VertexOut) 0.0f;
   
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;
    
    float4x4 prevFrame_modelMatrix = gPrevWorld;
    float3 prevFrame_worldPos = mul(float4(vin.PosL, 1.0), prevFrame_modelMatrix).xyz;
    float4 prevFrame_clipPos = mul(float4(prevFrame_worldPos, 1.0f), gPrevViewProj);
    vout.PrevPosH = prevFrame_clipPos;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
    
    vout.TangentW = mul(vin.TangentU, (float3x3) gWorld);
	
	// Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;

    return vout;
}


TAABuffer PS(VertexOut pin)
{
    float3 position = pin.PosW;
    
    float2 texCoord = pin.TexC;
    float4 albedo = gAlbedoTex.Sample(gsamLinearWrap, texCoord) * gDiffuseAlbedo;
    
    float4 normalMap = gNormalTex.Sample(gsamLinearWrap, texCoord);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap.rgb, pin.NormalW, pin.TangentW);
    
    float3 normal = normalize(bumpedNormalW);
    
    float Roughness = gRoughnessTex.Sample(gsamLinearWrap, texCoord).r;
    float AO = gAOTex.Sample(gsamLinearWrap, texCoord).r;
    float Metallic = gMetallicTex.Sample(gsamLinearWrap, texCoord).r;
    if (IsMetallic == 1.f)
        Metallic = 0.f;
    
    float F0 = lerp(float3(0.04, 0.04, 0.04), albedo.rgb, Metallic);
    
    float3 toEyeW = normalize(gEyePosW - position);

    // Light terms.
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo.rgb * AO;
    float3 V = normalize(gEyePosW - position);
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < 1; ++i)
    {
        float3 L = normalize(-gLights[i].Direction);
        float intensity = GetAdjustedIntensity(gLights[i].Strength.r, Metallic);
        float3 radiance = gLights[i].Color.rgb * gLights[i].Strength * intensity * 50;
        
        Lo += CalculatePBR(normal, V, L, radiance, Roughness, F0, Metallic, albedo);
    }
    
    float4 litColor = float4(ambient + Lo, 1.0f);
    
    TAABuffer buffer;
    buffer.MainRTV = litColor;
    buffer.VelocityRTV = float4(CalcVelocity(mul(float4(pin.PosW, 1.0f), gViewProj), pin.PrevPosH), 0.0f, 1.0f);

    return buffer;
}