#include "LightingUtil.hlsl"
#include "Common.hlsl"

Texture2D<float4> gTextures[5] : register(t0, space0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

#define MaxDecals 3

struct Decal
{
    float3 Position;
    float DisplacementScale;
    int IsActive;
    float Scale;
    float2 pad0;
};

struct GBuffer
{
    float4 Albedo : SV_TARGET0;
    float4 Position : SV_TARGET1;
    float4 Normal : SV_TARGET2;
    float4 Specular : SV_TARGET3;
};

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

cbuffer cbTess : register(b3)
{
    float gTessFactor;
    int bIsBackCulling;
    int bIsDisplacementAdaptiveTess;
    int bIsDistantAdaptiveTess;
    
    Decal gDecals[MaxDecals];
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
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin, uint instanceID : SV_InstanceID)
{
    VertexOut vout = (VertexOut) 0.0f;
    
    InstanceData instance = instanceBuffer[instanceID];
   
    // Transform to world space.
    float4 posW = mul(mul(float4(vin.PosL * scale, 1.0f), gWorld), instance.WorldMatrix);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
    
    vout.TangentW = mul(vin.TangentU, (float3x3) gWorld);
	
	// Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC * tilesCount, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;
    vout.Color = instance.Color;

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
    
    float3 vPos0 = patch[0].PosW;
    float3 vPos1 = patch[1].PosW;
    float3 vPos2 = patch[2].PosW;
    
    // find two triangle patch edges
    float3 vEdge0 = vPos1 - vPos0;
    float3 vEdge2 = vPos2 - vPos0;
    
    // Create the normal and view vector
    float3 vFaceNormal = normalize(cross(vEdge2, vEdge0));
    float3 vView = normalize(vPos0 - gEyePosW);
    
    // A negative dot product means facing away from view direction.
    // Use a small epsilon to avoid popping, since displaced vertices
    // may still be visible with dot product = 0
    if (dot(vView, vFaceNormal) < -0.25 && bIsBackCulling == 1)
    {
        // Cull the triangle by setting the tessellation factors to 0
        pt.EdgeTess[0] = 0;
        pt.EdgeTess[1] = 0;
        pt.EdgeTess[2] = 0;
        pt.InsideTess = 0;
        return pt; // early exit
    }
	else
    {
        float3 centerL = 0.25f * (patch[0].PosW + patch[1].PosW + patch[2].PosW);
        float3 centerW = mul(float4(centerL, 1.0f), gWorld).xyz;
	
        float d = distance(centerW, gEyePosW);

	    // Tessellate the patch based on distance from the eye such that
	    // the tessellation is 0 if d >= d1 and 64 if d <= d0.  The interval
	    // [d0, d1] defines the range we tessellate in.
	
        const float d0 = 5.0f;
        const float d1 = 30.0f;
        float tess = 64.0f;
        if (bIsDistantAdaptiveTess == 1)
        {
            tess = gTessFactor * saturate((d1 - d) / (d1 - d0)) * saturate((d1 - d) / (d1 - d0));
        }

	    // Uniformly tessellate the patch.
        if (tess > gTessFactor)
            tess = gTessFactor;
        if (tess < 1.f)  
            tess = 1.f;

        // Distance Adaptive Tess
        if (bIsDisplacementAdaptiveTess)
        {
            pt.InsideTess = 1.f;
            bool isEdgeTesselated = false;
            for (int i = 0; i < 3; ++i) // Edges
            {
                pt.EdgeTess[i] = 1.f;
                for (int j = 0; j < MaxDecals; ++j)
                {
                    if (gDecals[j].IsActive == 1)
                    {
                        if (distance(gDecals[j].Position, patch[i].PosW) < gDecals[j].Scale ||
                            distance(gDecals[j].Position, patch[(i + 1) % 3].PosW) < gDecals[j].Scale)
                        {
                            isEdgeTesselated = true;
                            pt.EdgeTess[i] = tess;
                        }
                    }
                }
            }
            
            if (isEdgeTesselated)
            {
                pt.InsideTess = tess;
            }
        }
        else
        {
            pt.EdgeTess[0] = tess;
            pt.EdgeTess[1] = tess;
            pt.EdgeTess[2] = tess;
	
            pt.InsideTess = tess;
        }
	
        return pt;
    }
}

struct HullOut
{
    float3 PosL : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
    float4 Color : COLOR;
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
    hout.Color = p[i].Color;
	
    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
    float4 Color : COLOR;
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
    
    float4 color =
        barycentric.x * tri[0].Color +
        barycentric.y * tri[1].Color +
        barycentric.z * tri[2].Color;
    
    float resultDisplacement = 0.0f;

    for (int i = 0; i < MaxDecals; ++i)
    {
        if (gDecals[i].IsActive == 1 
            && distance(gDecals[i].Position, position) < gDecals[i].Scale)
        {
            float displacement = gTextures[2].SampleLevel(gsamLinearWrap, texCoord, 0).r;
            float displacementScale = 0.1f * gDecals[i].DisplacementScale;
            displacement = (2.f * displacement - 1.0f) * displacementScale;
            resultDisplacement += displacement;
        }
    }

    position += normal * resultDisplacement;
    
    output.PosH = mul(float4(position, 1.0), gViewProj);
    output.PosW = position;
    output.NormalW = normal;
    output.TexC = texCoord;
    output.TangentW = tangent;
    output.Color = color;
    
    return output;
}


GBuffer PS(DomainOut pin)
{
    GBuffer gBuffer;
    gBuffer.Position = float4(pin.PosW, 1.0f);
    
    float2 texCoord = pin.TexC;
    gBuffer.Albedo = gTextures[0].Sample(gsamLinearWrap, texCoord) * gDiffuseAlbedo * pin.Color;
    
    float4 normalMap = gTextures[1].Sample(gsamLinearWrap, texCoord);
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap.rgb, pin.NormalW, pin.TangentW);
    
    gBuffer.Normal = float4(normalize(bumpedNormalW), 1.0f);
    
    float3 Roughness = gTextures[3].Sample(gsamLinearWrap, texCoord);
    float3 AO = gTextures[4].Sample(gsamLinearWrap, texCoord);
    gBuffer.Specular = float4(Roughness.xyz, AO.x);

    return gBuffer;
}