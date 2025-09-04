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

TextureCube gIrradianceMap : register(t0, space1);
TextureCube gPreFilteredEnvMap : register(t1, space1);
Texture2D<float4> gIntegrationMap : register(t2, space1);

struct InstanceData
{
    float4x4 WorldMatrix;
    float Roughness;
    float Metallic;
};

StructuredBuffer<InstanceData> instanceBuffer : register(t0, space2);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

float3 CalculateIBL(float3 N, float3 V, float4 albedo, float3 F0, float roughness, float metallic)
{
    float3 R = reflect(-V, N);
    
    // Specular IBL
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    // Diffuse IBL
    float3 irradiance = gIrradianceMap.Sample(gsamLinearWrap, N).rgb;
    float3 diffuse = irradiance * albedo.rgb;
    
    // Specular IBL
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = gPreFilteredEnvMap.SampleLevel(gsamLinearWrap, R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = gIntegrationMap.Sample(gsamLinearWrap, float2(max(dot(N, V), 0.0), roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    return kD * diffuse + specular;
}


cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
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
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
    uint instanceID : INSTANCE_ID;
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
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;
    vout.instanceID = instanceID;

    return vout;
}


float4 PS(VertexOut pin) : SV_Target
{
    InstanceData instanceData = instanceBuffer[pin.instanceID];
    float3 position = pin.PosW;
    
    float2 texCoord = pin.TexC;
    float4 albedo = gAlbedoTex.Sample(gsamLinearWrap, texCoord) * gDiffuseAlbedo;
    
    float4 normalMap = gNormalTex.Sample(gsamLinearWrap, texCoord);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap.rgb, pin.NormalW, pin.TangentW);
    
    float3 normal = normalize(bumpedNormalW);
    
    float Roughness = instanceData.Roughness;
    float AO = gAOTex.Sample(gsamLinearWrap, texCoord).r;
    float Metallic = instanceData.Metallic;
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.rgb, Metallic);
    
    float3 toEyeW = normalize(gEyePosW - position);

    // Light terms.
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo.rgb * AO;
    float3 V = normalize(gEyePosW - position);
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < 1; ++i)
    {
        float3 L = normalize(-gLights[i].Direction);
        float intensity = GetAdjustedIntensity(gLights[i].Strength.r, Metallic);
        float3 radiance = gLights[i].Color.rgb * gLights[i].Strength * intensity * 20;
        
        Lo += CalculatePBR(normal, V, L, radiance, Roughness, F0, Metallic, albedo);
    }
    Lo += CalculateIBL(normal, V, albedo, F0, Roughness, Metallic);
    
    float4 litColor = float4(ambient + Lo, 1.0f);
    
    // Tone mapping
    //litColor.rgb = litColor.rgb / (litColor.rgb + float3(1.0, 1.0, 1.0));
    
    // Gamma correction
    //litColor.rgb = pow(litColor.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return litColor;
}