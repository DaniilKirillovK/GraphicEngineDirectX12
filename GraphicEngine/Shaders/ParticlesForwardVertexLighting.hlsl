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

Texture2D gParticleTexture : register(t0, space0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct Particle
{
    float3 Position;
    float Age;
    float3 Velocity;
    float Weight;
    float3 Acceleration;
    float Size;
    float4 Color;
    float LifeTime;
};

StructuredBuffer<Particle> particlesIn : register(t0, space1);

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
    int lightingID;
    float2 cbPad;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
	// are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

// Constant data that varies per material.
cbuffer cbMaterial : register(b1)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float gRoughness;
    float4x4 gMatTransform;
    float tilesCount;
};


struct VertexIn
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

	// Just pass data over to geometry shader.
    vout.CenterW = vin.PosW;
    vout.SizeW = vin.SizeW;

    return vout;
}

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
    float4 Color : COLOR;
};
 
 // We expand each point into a quad (4 vertices), so the maximum number of vertices
 // we output per geometry shader invocation is 4.
[maxvertexcount(4)]
void GS(point VertexOut gin[1],
        uint primID : SV_PrimitiveID,
        inout TriangleStream<GeoOut> triStream)
{
    float size = gin[0].SizeW.x * particlesIn[primID].Size;
    float3 position = gin[0].CenterW + particlesIn[primID].Position;
	//
	// Compute the local coordinate system of the sprite relative to the world
	// space such that the billboard is aligned with the y-axis and faces the eye.
	//

    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - position;
    look.y = 0.0f; // y-axis aligned, so project to xz-plane
    look = normalize(look);
    float3 right = cross(up, look);

	//
	// Compute triangle strip vertices (quad) in world space.
	//
    float halfWidth = 0.5f * size;
    float halfHeight = 0.5f * size;
	
    float4 v[4];
    v[0] = float4(position + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(position + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(position - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(position - halfWidth * right + halfHeight * up, 1.0f);

	//
	// Transform quad vertices to world space and output 
	// them as a triangle strip.
	//
	
    float2 texC[4] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
    };
    
    float3 Roughness = float3(gRoughness, gRoughness, gRoughness);
    float3 AO = float3(0.5f, 0.5f, 0.5f);
    
    float3 toEyeW = normalize(gEyePosW - position);
    
    float3 normal = normalize(float3(1.0f, 1.0f, 1.0f));

    // Light terms.
    float4 ambient = gAmbientLight;

    float3 gFresnelR0 = float3(0.01f, 0.01f, 0.01f);
    
    const float shininess = 1.0f - Roughness.x;
    Material mat = { float4(1.0f, 1.0f, 1.0f, 1.0f), gFresnelR0, shininess };
    float3 shadowFactor = float3(AO.x, AO.x, AO.x);
    
    float4 Color = ComputeLighting(gLights, mat, position,
        normal, toEyeW, shadowFactor);
	
    GeoOut gout;
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(v[i], gViewProj);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.TexC = texC[i];
        gout.PrimID = primID;
        gout.Color = Color;
		
        triStream.Append(gout);
    }
}

float4 PS(GeoOut pin) : SV_Target
{
    float4 Position = float4(pin.PosW, 1.0f);
    
    float2 TexCoord = pin.TexC;
    float4 Albedo = (gParticleTexture.Sample(gsamLinearWrap, TexCoord) * gDiffuseAlbedo);

    // Light terms.
    float4 ambient = gAmbientLight * Albedo;
    clip(ambient.a - 0.1f);
    
    float4 litColor = ambient + pin.Color;

    return litColor;
}