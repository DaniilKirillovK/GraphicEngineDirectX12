struct InstanceData
{
    float4 Position;
    float4 Color;
};

RWStructuredBuffer<InstanceData> instancingUAV : register(u0);

cbuffer computeShaderBuffer : register(b3)
{
    float instancingLevel;
}

[numthreads(1, 1, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{

}