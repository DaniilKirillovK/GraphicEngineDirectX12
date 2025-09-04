struct DrawInstancedArgs
{
    uint VertexCountPerInstance;
    uint InstanceCount;
    uint StartVertexLocation;
    uint StartInstanceLocation;
};

RWStructuredBuffer<DrawInstancedArgs> ArgsBuffer : register(u0);

[numthreads(1, 1, 1)]
void CS_UpdateArgs(uint3 id : SV_DispatchThreadID)
{
    DrawInstancedArgs args;
    
    args.VertexCountPerInstance = 512;
    args.StartVertexLocation = 0;
    args.StartInstanceLocation = 0;
    
    args.InstanceCount = 1;
    
    ArgsBuffer[0] = args;
}