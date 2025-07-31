Texture2D<float4> sceneTexture : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer cbPostProcessing : register(b0)
{
    float gGammaRatio;
    float gTextureSize;
    float2 caDistortion;
    float2 caDirection;
    bool gbIsHorizontal;
    float caIntensity;
    float caPadding;
    float2 vCenter;
    float vIntensity; 
    float vSmoothness; 
    float vRoundness; 
};

struct VSInput
{
    float3 Position : POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 TexC : TEXCOORD;
};

VertexOut VS(uint vertexID : SV_VertexID, VSInput vin)
{
    VertexOut vOut = (VertexOut) 0.0f;
    
    vOut.TexC = float2((vertexID << 1) & 2, vertexID & 2);
    vOut.Position = float4(vOut.TexC * 2.0 - 1.0, 0.0, 1.0);
    
    vOut.Position.y *= -1;
    
    return vOut;
}

float4 DefaultPS(VertexOut input) : SV_TARGET
{
    float3 color = sceneTexture.Sample(gsamPointWrap, input.TexC).rgb;
    return float4(color, 1.0);
}

float4 PSGammaCorrection(VertexOut input) : SV_TARGET
{
    float3 color = sceneTexture.Sample(gsamPointWrap, input.TexC).rgb;
    
    color = pow(color, 1.0 / gGammaRatio);
    
    return float4(color, 1.0);
}

static const float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
static const float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

float4 PSGaussianBlur(VertexOut input) : SV_TARGET
{
    float2 tex_offset = 1.0 / gTextureSize;
    float3 result = sceneTexture.Sample(gsamPointWrap, input.TexC).rgb * weight[0];
    
    if (gbIsHorizontal)
    {
        for (int i = 1; i < 3; ++i)
        {
            result += sceneTexture.Sample(gsamPointWrap, input.TexC + float2(offset[i], 0.0) * tex_offset).rgb * weight[i];
            result += sceneTexture.Sample(gsamPointWrap, input.TexC - float2(offset[i], 0.0) * tex_offset).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 3; ++i)
        {
            result += sceneTexture.Sample(gsamPointWrap, input.TexC + float2(0.0, offset[i]) * tex_offset).rgb * weight[i];
            result += sceneTexture.Sample(gsamPointWrap, input.TexC - float2(0.0, offset[i]) * tex_offset).rgb * weight[i];
        }
    }
    
    return float4(result, 1.0);
}

float4 PSChromaticAberration(VertexOut input) : SV_TARGET
{
    float2 centeredUV = input.TexC - 0.5;
    
    float distanceFromCenter = length(centeredUV);
    
    float distortionFactor = distanceFromCenter * caIntensity;
    
    float2 redOffset = input.TexC + caDirection * distortionFactor * caDistortion.x;
    float2 greenOffset = input.TexC + caDirection * distortionFactor * caDistortion.y * 0.5;
    float2 blueOffset = input.TexC - caDirection * distortionFactor * caDistortion.x;

    redOffset = clamp(redOffset, 0.001, 0.999);
    greenOffset = clamp(greenOffset, 0.001, 0.999);
    blueOffset = clamp(blueOffset, 0.001, 0.999);
    
    float red = sceneTexture.Sample(gsamPointWrap, redOffset).r;
    float green = sceneTexture.Sample(gsamPointWrap, greenOffset).g;
    float blue = sceneTexture.Sample(gsamPointWrap, blueOffset).b;
    
    float alpha = sceneTexture.Sample(gsamPointWrap, input.TexC).a;
    
    return float4(red, green, blue, alpha);
}

float4 PSVignette(VertexOut input) : SV_TARGET
{
    float4 color = sceneTexture.Sample(gsamPointWrap, input.TexC);
    
    float2 uv = (input.TexC - vCenter) * float2(1.0, 1.0 / vRoundness);
    float distanceFromCenter = length(uv);
    
    float vignette = 1.0 - smoothstep(vSmoothness, 1.0, distanceFromCenter * vIntensity);
    
    color.rgb *= vignette;
    
    return color;
}