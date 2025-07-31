RWTexture2D<float4> OutputTexture : register(u0, space0);

cbuffer cbNoise : register(b0)
{
    float gTotalTime;
};

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    float randomVal = frac(sin(dot(id.xy * gTotalTime, float2(12.9898, 78.233))) * 43758.5453);
    randomVal = frac(randomVal);
    
    OutputTexture[id.xy] = float4(randomVal, randomVal, randomVal, 1.0);
}