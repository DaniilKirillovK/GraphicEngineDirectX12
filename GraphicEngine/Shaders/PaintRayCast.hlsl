cbuffer ConstParams : register(b0)
{
    float2 ClickPos;
    float3 Color;
};

Texture2D<float4> uvTexture : register(t0);

RWTexture2D<float4> paintTexture : register(u0);


[numthreads(1, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    float2 uvData = uvTexture.Load(float3(ClickPos.x, ClickPos.y, 0.f));
    paintTexture[uvData] = float4(Color, 1.0f);
}