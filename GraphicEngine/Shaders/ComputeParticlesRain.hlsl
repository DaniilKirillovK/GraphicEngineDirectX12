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
    int ParticleType;
};

RWStructuredBuffer<Particle> particlesOut : register(u0);
StructuredBuffer<Particle> particlesIn : register(t0);

cbuffer cbParticleEmitter : register(b0)
{
    float3 EmitterPosition;
    float DeltaTime;
    float3 GravityForce;
    uint SystemID;
    float4 StartColor;
    float4 EndColor;
    float StartSize;
    float EndSize;
    uint MaxParticles;
    uint EmitterIsActive;
    float TotalTime;
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
    
    if (particle.ParticleType == 0)
    {
        if (particle.Weight == 1.0f)
        {
            // Respawning particle
            if (EmitterIsActive)
            {
                particle.Position = EmitterPosition + float3(Rand1(id.x) * 40, 0.0f, Rand2(id.x) * 40);
                
                particle.Velocity = float3(0.0f, -5.0f, 0.0f);
                particle.Color = StartColor;
                particle.Size = 0.5f;
                particle.Age = 0;
                particle.Weight = 0.0f;
                particle.LifeTime = 1.0f + Rand1(particle.Position.xz) * 0.5f;
            }
        }
        else
        {
            // Updating alive particles
            particle.Position += particle.Velocity * DeltaTime;
            particle.Velocity -= float3(0.0f, 0.1f, 0.0f) * DeltaTime;
            
            if (particle.Position.y < 0.1f)
            {
                particle.ParticleType = 1;
            }

        }
    }
    else if (particle.ParticleType == 1)
    {
        if (particle.Age >= particle.LifeTime)
        {
            particle.ParticleType = 0;
            particle.Weight = 1.0f;
        }
        else
        {
            particle.Age += DeltaTime;
            particle.Size = lerp(0.8f, 1.0f, particle.Age / particle.LifeTime);
        }
    }
    
    particlesOut[id.x] = particle;
}