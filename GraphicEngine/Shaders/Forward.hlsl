#include "LightingUtil.hlsl"
#include "Common.hlsl"

Texture2D<float4> gTextures[5] : register(t0);
Texture2D<float4> decalTexture : register(t15);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct GBuffer
{
    float4 Color : SV_TARGET0;
    float4 Albedo : SV_TARGET1;
    float4 Position : SV_TARGET2;
    float4 Normal : SV_TARGET3;
    float4 Specular : SV_TARGET4;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
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
    float cbPerObjectPad3;

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
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
    
    vout.TangentW = mul(vin.TangentU, (float3x3) gWorld);
	
	// Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC * tilesCount, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;

    return vout;
}
 
struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 3> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
	
    float3 centerL = 0.25f * (patch[0].PosW + patch[1].PosW + patch[2].PosW);
    float3 centerW = mul(float4(centerL, 1.0f), gWorld).xyz;
	
    float d = distance(centerW, gEyePosW);

	// Tessellate the patch based on distance from the eye such that
	// the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
	// [d0, d1] defines the range we tessellate in.
	
    const float d0 = 5.0f;
    const float d1 = 30.0f;
    float tess = tessFactor * saturate((d1 - d) / (d1 - d0)) * saturate((d1 - d) / (d1 - d0));

	// Uniformly tessellate the patch.
    if (tess < 1.f)  
        tess = 1.f;
    
    if (tess > tessFactor)
        tess = tessFactor;

    pt.EdgeTess[0] = tess;
    pt.EdgeTess[1] = tess;
    pt.EdgeTess[2] = tess;
	
    pt.InsideTess = tess;
	
    return pt;
}

struct HullOut
{
    float3 PosL : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 3> p,
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
    HullOut hout;
	
    hout.PosL = p[i].PosW;
    hout.NormalW = p[i].NormalW;
    hout.TexC = p[i].TexC;
    hout.TangentW = p[i].TangentW;
	
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
};


[domain("tri")]
DomainOut DS(PatchTess patchTess,
             float3 barycentric : SV_DomainLocation,
             const OutputPatch<HullOut, 3> tri)
{
    DomainOut output;
    
    // Position interpolation
    float3 position =
        barycentric.x * tri[0].PosL +
        barycentric.y * tri[1].PosL +
        barycentric.z * tri[2].PosL;
    
    // Normal interpolation (should normalize after)
    float3 normal =
        barycentric.x * tri[0].NormalW +
        barycentric.y * tri[1].NormalW +
        barycentric.z * tri[2].NormalW;
    normal = normalize(normal);
    
    // Texture coordinate interpolation
    float2 texCoord =
        barycentric.x * tri[0].TexC +
        barycentric.y * tri[1].TexC +
        barycentric.z * tri[2].TexC;
    
    float3 tangent =
        barycentric.x * tri[0].TangentW +
        barycentric.y * tri[1].TangentW +
        barycentric.z * tri[2].TangentW;

    float displacement = gTextures[2].SampleLevel(gsamLinearWrap, texCoord, 0).r;
    float displacementScale = 0.1f;
    displacement = (2.f * displacement - 1.0f) * displacementScale;

    position += normal * displacement;
    
    
    output.PosH = mul(float4(position, 1.0), gViewProj);
    output.PosW = position;
    output.NormalW = normal;
    output.TexC = texCoord;
    output.TangentW = tangent;
    
    return output;
}

GBuffer PSForward(DomainOut pin) : SV_Target
{
    GBuffer gBuffer;
    
    float4 diffuseAlbedo = gTextures[0].Sample(gsamLinearWrap, pin.TexC) * gDiffuseAlbedo;
    float4 normalMap = gTextures[1].Sample(gsamLinearWrap, pin.TexC);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap.rgb, pin.NormalW, pin.TangentW);

    // Interpolating normal can unnormalize it, so renormalize it.
    //pin.NormalW = normalize(normalMap.xyz);
    pin.NormalW = normalMap.xyz;

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        bumpedNormalW, toEyeW, shadowFactor);
    
    Material matDeferred = { float4(1.0f, 1.0f, 1.0f, 1.0f), gFresnelR0, shininess };
    
    float4 directLightDeferred = ComputeLighting(gLights, matDeferred, pin.PosW,
        bumpedNormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material.
    litColor.a = diffuseAlbedo.a;
    
    gBuffer.Color = litColor;
    gBuffer.Albedo = float4(1.0f, 1.0f, 1.0f, 1.0f);
    gBuffer.Normal = float4(1.0f, 1.0f, 1.0f, 1.0f);
    gBuffer.Position = float4(1.0f, 1.0f, 1.0f, 1.0f);
    gBuffer.Specular = float4(1.0f, 1.0f, 1.0f, 1.0f);

    return gBuffer;
}