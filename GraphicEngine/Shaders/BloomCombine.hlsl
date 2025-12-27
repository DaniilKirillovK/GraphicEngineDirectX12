cbuffer CombineConstants : register(b0)
{
    float bloomIntensity;
    float3 padding;
};

Texture2D<float4> bloomTexture : register(t0, space0);
Texture2D<float4> sourceTexture : register(t0, space1);

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


float4 PSMain(VertexOut pin) : SV_TARGET
{
    float3 sourceColor = sourceTexture.Sample(gsamLinearWrap, pin.uv).rgb;
    float3 bloomColor = bloomTexture.Sample(gsamLinearWrap, pin.uv).rgb;
    
    bloomColor *= bloomIntensity;
    
    float3 resultColor = sourceColor + bloomColor;  
    
    resultColor = resultColor / (resultColor + float3(1.0f, 1.0f, 1.0f));
    
    return float4(resultColor, 1.0f);
}