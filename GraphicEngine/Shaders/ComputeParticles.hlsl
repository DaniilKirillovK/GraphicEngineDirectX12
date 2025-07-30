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
    
    if (SystemID == 1)
    {
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
    }
    else if (SystemID == 2)
    {
        if (particle.Age >= particle.LifeTime)
        {
            // Respawning particle
            if (EmitterIsActive)
            {
                particle.Position = EmitterPosition + float3(sin(TotalTime * id.x) * 0.01f, 0.0f, cos(TotalTime * id.x) * 0.01f);
                particle.Velocity = float3(0.0f, 0.0f, 0.0f);
                particle.Color = StartColor;
                particle.Size = StartSize;
                particle.Age = 0;
                particle.LifeTime = clamp(3, 8, (float) id.x / 64);
            }
        }
        else
        {
            // Updating alive particles
            particle.Age += DeltaTime;
            particle.Position += particle.Velocity * DeltaTime;
            
            float3 center = EmitterPosition;

            float3 toCenter = center - particle.Position;
            float distanceToCenter = length(toCenter);
            float3 dirToCenter = toCenter / (distanceToCenter + 0.001f);

            float gravityForce = 0.001f * (1.0f - exp(-particle.Age * 0.5f)); 

            float3 tangentDir = cross(dirToCenter, float3(0, 1, 0));
            float spiralSpeed = 2.0f; 

            particle.Velocity += (dirToCenter * gravityForce + 
                tangentDir * spiralSpeed) * DeltaTime;
            
            particle.Color = lerp(StartColor, EndColor, particle.Age / particle.LifeTime);
            particle.Size = lerp(StartSize, EndSize, particle.Age / particle.LifeTime);
        }
    }
    
    particlesOut[id.x] = particle;
}