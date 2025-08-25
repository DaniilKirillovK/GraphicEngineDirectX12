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

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float scale;
    int lightTypeID;
    int lightID;
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
    float pad0;
    float2 gResolution;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
   
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL * scale, 1.0f), gWorld);

    vout.PosW = posW.xyz;

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
    
    vout.Color = vin.Color;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float2 pixelCoords = pin.PosH.xy / gResolution;
    
    float4 albedo = gAlbedo.Sample(gsamPointWrap, pixelCoords);
    float3 worldPos = gPosition.Sample(gsamPointWrap, pixelCoords).xyz;
    float3 normal = gNormal.Sample(gsamPointWrap, pixelCoords).xyz;
    float3 normalResult = normalize(normal.rgb * 2.0 - 1.0);
    
    float Roughness = gSpecular.Sample(gsamPointWrap, pixelCoords).x;
    float AO = gSpecular.Sample(gsamPointWrap, pixelCoords).w;
    
    float depth = gDepth.Sample(gsamPointWrap, pixelCoords).r;
    
    float3 toEyeW = normalize(gEyePosW - worldPos);

    float3 gFresnelR0 = float3(0.01f, 0.01f, 0.01f);
    
    const float shininess = 1.0f - Roughness;
    Material mat = { albedo, gFresnelR0, shininess };
    
    float4 directLight = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    if (lightTypeID == 1)
        directLight = float4(ComputePointLight(gLights[lightID], mat, worldPos,
            normal, toEyeW) * gLights[lightID].Color.xyz * float3(AO, AO, AO), 1.0f);
    else if (lightTypeID == 2)
        directLight = float4(ComputeSpotLight(gLights[lightID], mat, worldPos,
            normal, toEyeW) * gLights[lightID].Color.xyz * float3(AO, AO, AO), 1.0f);
    
    float4 litColor = directLight;
    
    return litColor;
}