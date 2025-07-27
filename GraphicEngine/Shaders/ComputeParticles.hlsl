struct Particle
{
    float3 Position;
    float Age;
    float3 Velocity;
    float Weight;
    float3 Acceleration;
    float Size;
    float4 Color;
    float LifeTime;
};

RWStructuredBuffer<Particle> particlesOut : register(u0);
StructuredBuffer<Particle> particlesIn : register(t0);

cbuffer cbParticleEmitter : register(b0)
{
    float3 EmitterPosition;
    float DeltaTime;
    float3 GravityForce;
    float Pad1;
    float4 StartColor;
    float4 EndColor;
    float StartSize;
    float EndSize;
    uint MaxParticles;
    uint EmitterIsActive;
}

float Rand1(float2 co)
{
    return sin(dot(co, float2(12.9898, 78.233)));
}

float Rand2(float2 co)
{
    return cos(dot(co, float2(35.345, 2.543)));
}

[numthreads(64, 1, 1)]
void CS_UpdateParticles(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= MaxParticles)
        return;
    
    Particle particle = particlesIn[id.x];
    
    if (particle.Age >= particle.LifeTime)
    {
        // Respawning particle
        if (EmitterIsActive)
        {
            particle.Position = EmitterPosition;
            particle.Velocity = float3(0.0f, 0.0f, 0.0f);
            particle.Color = StartColor;
            particle.Size = StartSize;
            particle.Age = 0;
            particle.LifeTime = clamp(1, 10, (float)id.x / 8);
        }
    }
    else
    {
        // Updating alive particles
        particle.Age += DeltaTime;
        particle.Position += particle.Velocity * DeltaTime;
        particle.Velocity += float3(
                Rand1((id.x + 1) * particle.Age),
                0.1f,
                Rand2((id.x + 1) * particle.Age)
            ) * DeltaTime;
        particle.Color = lerp(StartColor, EndColor, particle.Age / particle.LifeTime);
        particle.Size = lerp(StartSize, EndSize, particle.Age / particle.LifeTime);
    }
    
    particlesOut[id.x] = particle;
}