cbuffer gConstants : register(b0)
{
    float threshold;
    float softThreshold;
    float2 pad;
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

float4 ExtractBrightPS(VertexOut pin) : SV_Target
{
    float3 color = gInputTexture.Sample(gsamLinearWrap, pin.uv).rgb;
    
    float brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    float bloomStrength = smoothstep(threshold, threshold + softThreshold, brightness);
    
    return float4(color * bloomStrength, 1.0f);
}