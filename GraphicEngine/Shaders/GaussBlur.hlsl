cbuffer gConstants : register(b0)
{
    float4 weights1;
    float4 weights2;
    float4 weights3;
    float4 weights4;
    float4 weights5;
    float4 weights6;
    float4 weights7;
    float4 weights8;
    float4 weights9;
    float4 weights10;
    
    float4 offsets1;
    float4 offsets2;
    float4 offsets3;
    float4 offsets4;
    float4 offsets5;
    float4 offsets6;
    float4 offsets7;
    float4 offsets8;
    float4 offsets9;
    float4 offsets10;
    
    int blurRadius;
    int horizontal;
    float texOffset;
    float pad3;
};

Texture2D<float4> gInputTexture : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct VertexOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VertexOut VSMain(uint vertexID : SV_VertexID)
{
    VertexOut vOut = (VertexOut) 0.0f;
    
    vOut.uv = float2((vertexID << 1) & 2, vertexID & 2);
    vOut.position = float4(vOut.uv * 2.0 - 1.0, 0.0, 1.0);
    
    vOut.position.y *= -1;
    
    return vOut;
}

float4 BlurPS(VertexOut pin) : SV_Target
{
    float weight[40] =
    {
        weights1.x, weights1.y, weights1.z, weights1.w,
        weights2.x, weights2.y, weights2.z, weights2.w,
        weights3.x, weights3.y, weights3.z, weights3.w,
        weights4.x, weights4.y, weights4.z, weights4.w,
        weights5.x, weights5.y, weights5.z, weights5.w,
        weights6.x, weights6.y, weights6.z, weights6.w,
        weights7.x, weights7.y, weights7.z, weights7.w,
        weights8.x, weights8.y, weights8.z, weights8.w,
        weights9.x, weights9.y, weights9.z, weights9.w,
        weights10.x, weights10.y, weights10.z, weights10.w
    };
    float offset[40] =
    {
        offsets1.x, offsets1.y, offsets1.z, offsets1.w,
        offsets2.x, offsets2.y, offsets2.z, offsets2.w,
        offsets3.x, offsets3.y, offsets3.z, offsets3.w,
        offsets4.x, offsets4.y, offsets4.z, offsets4.w,
        offsets5.x, offsets5.y, offsets5.z, offsets5.w,
        offsets6.x, offsets6.y, offsets6.z, offsets6.w,
        offsets7.x, offsets7.y, offsets7.z, offsets7.w,
        offsets8.x, offsets8.y, offsets8.z, offsets8.w,
        offsets9.x, offsets9.y, offsets9.z, offsets9.w,
        offsets10.x, offsets10.y, offsets10.z, offsets10.w
    };
    int radius = blurRadius;
    
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    for (int i = -radius; i <= radius; ++i)
    {
        float2 texCoord;
        if (horizontal == 0)
        {
            texCoord = pin.uv + float2(offset[i + radius], 0);
        }
        else
        {
            texCoord = pin.uv + float2(0, offset[i + radius]);
        }
        
        if (texCoord.x >= 0.f && texCoord.x <= 1.f && texCoord.y >= 0.f && texCoord.y <= 1.f)
        {
            result += weight[i + radius] * gInputTexture.Sample(gsamLinearWrap, texCoord);
        }
    }
    
    return result;
}