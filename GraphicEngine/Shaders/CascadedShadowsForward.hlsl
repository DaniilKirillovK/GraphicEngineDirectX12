#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#include "Common.hlsl"
#include "LightingUtil.hlsl"

Texture2D<float4> gShadowMap[4] : register(t0, space0);
Texture2D<float4> gTextures[1] : register(t0, space1);
Texture2D<float4> gShadowTextures[3] : register(t0, space2);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

SamplerComparisonState gsamShadowPoint : register(s6);
SamplerComparisonState gsamShadowLinear : register(s7);
SamplerComparisonState gsamShadowAnisotropic : register(s8);

// Constant data that varies per frame.
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
    float4x4 gViewMain;
    float4x4 gProjMain;
    
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gShadowTransform;
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
    int isTexturedShadows;
    int shadowTextureID;
    int isCascadedShadows;
    
    int shadowMapSizeID;
    int shadowFilteringID;
    int cascadedShadowMapID;
    float pad0;

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

float CalcShadowFactor(float4 shadowPosH, float4 posH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;
    posH.xyz /= posH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;
    float depthCascade = posH.z;

    uint width, height, numMips;
    
    int cascadedMapID = 0;
    if (depthCascade > 0.070)
        cascadedMapID = 3;
    else if (depthCascade > 0.050)
        cascadedMapID = 2;
    else if (depthCascade > 0.025)
        cascadedMapID = 1;
    else
        cascadedMapID = 0;
    
    if (isCascadedShadows == 0)
    {
        gShadowMap[shadowMapSizeID].GetDimensions(0, width, height, numMips);
    }
    else
    {
        gShadowMap[cascadedMapID].GetDimensions(0, width, height, numMips);
    }

    // Texel size.
    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        if (isCascadedShadows == 0)
        {
            if (shadowFilteringID == 0)
                percentLit += gShadowMap[shadowMapSizeID].SampleCmpLevelZero(gsamShadowPoint, shadowPosH.xy + offsets[i], depth).r;
            else if (shadowFilteringID == 1)
                percentLit += gShadowMap[shadowMapSizeID].SampleCmpLevelZero(gsamShadowLinear, shadowPosH.xy + offsets[i], depth).r;
            else
                percentLit += gShadowMap[shadowMapSizeID].SampleCmpLevelZero(gsamShadowAnisotropic, shadowPosH.xy + offsets[i], depth).r;
        }
        else
        {
            if (shadowFilteringID == 0)
                percentLit += gShadowMap[cascadedMapID].SampleCmpLevelZero(gsamShadowPoint, shadowPosH.xy + offsets[i], depth).r;
            else if (shadowFilteringID == 1)
                percentLit += gShadowMap[cascadedMapID].SampleCmpLevelZero(gsamShadowLinear, shadowPosH.xy + offsets[i], depth).r;
            else
                percentLit += gShadowMap[cascadedMapID].SampleCmpLevelZero(gsamShadowAnisotropic, shadowPosH.xy + offsets[i], depth).r;
        }
    }
    
    return percentLit / 9.0f;
}

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
    float4 ShadowPosH : POSITION0;
    float3 PosW : POSITION1;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);
	
    vout.TangentW = mul(vin.TangentU, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;

    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(posW, gShadowTransform);
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	// Fetch the material data.
    float4 diffuseAlbedo = gDiffuseAlbedo;
    float3 fresnelR0 = gFresnelR0;
    float roughness = gRoughness;
    
    // Only the first light casts a shadow.
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH, pin.PosH);
    shadowFactor[1] = shadowFactor[0];
    shadowFactor[2] = shadowFactor[0];
    if (isTexturedShadows)
    {
        if (shadowFactor.r != 1.0f)
        {
            shadowFactor += 0.3f * gShadowTextures[shadowTextureID - 1].Sample(gsamAnisotropicWrap, pin.TexC).r;
        }
    }
	
    // Dynamically look up the texture in the array.
    //if (isTexturedShadows)
    //{
    //    if (shadowFactor.r != 1.0f)
    //        diffuseAlbedo *= gShadowTextures[shadowTextureID - 1].Sample(gsamAnisotropicWrap, pin.TexC);
    //    else diffuseAlbedo *= gTextures[0].Sample(gsamAnisotropicWrap, pin.TexC);
    //}
    diffuseAlbedo *= gTextures[0].Sample(gsamAnisotropicWrap, pin.TexC);

	// Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = (1.0f - roughness);
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;
	
    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}
