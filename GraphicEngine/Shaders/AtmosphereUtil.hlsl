#pragma pack_matrix(row_major)

static const float PI = 3.14159265f;

static const float StepCount = 16.f;


cbuffer Constants : register(b0)
{
    float4x4 InvProjectionMatrix;
    float4x4 InvViewMatrix;
    float4x4 InvViewProjMatrix;
    float4 LightDirAndIntensity;
    float4 EarthCenterAndRadius;
    float2 DensityScaleHeight;
    float AtmosphereRadius;
    float MieG;
    float MieCoef;
    float3 CameraPosition;
    float3 RayleiCoef;
    float ScaterringIntensity;
    float3 SunColor;
}


float3 ReconstructWorldPos(float2 uv, float depth)
{
    float4 clipPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    clipPos.y = -clipPos.y;
    
    float4 viewSpacePos = mul(clipPos, InvProjectionMatrix);
    
    viewSpacePos /= viewSpacePos.w;
    
    float4 worldSpacePos = mul(viewSpacePos, InvViewMatrix);
    return worldSpacePos.xyz;
}

float RayleighPhase(float cosTheta)
{
    return (3.0f / (16.0f * PI)) * (1.0f + cosTheta * cosTheta);
}

float MiePhase(float cosTheta, float g)
{
    float g2 = g * g;
    return (1.0f - g2) / (4.0f * PI * pow(1.0f + g2 - 2.0f * g * cosTheta, 1.5f));
}

float DensityAtHeight(float height, float densityFalloff)
{
    float h = max(0.0f, height);
    return exp(-h / densityFalloff);
}

float3 CalculateAtmosphericScattering(float3 rayOrigin, float3 rayDir, float maxDistance, out float3 transmittance)
{
    float densityFalloff = 800.0f;
    float3 totalRayleigh = float3(0.0f, 0.0f, 0.0f);
    float totalMie = 0.0f;
    
    float stepSize = maxDistance / StepCount;
    float lightStepSize = stepSize / StepCount;
    
    for (int i = 0; i < StepCount; ++i)
    {
        float t = (i + 0.5f) * stepSize;
        float3 samplePos = rayOrigin + rayDir * t;
        
        float height = length(samplePos - EarthCenterAndRadius.xyz) - EarthCenterAndRadius.w;
        
        if (height < 0.0f)
            continue;
        
        float density = DensityAtHeight(height, densityFalloff);
        float3 rayleighDensity = RayleiCoef * density;
        float mieDensity = MieCoef * density;
        
        float3 lightRayDir = LightDirAndIntensity.xyz;
        float lightDistance = AtmosphereRadius * 2.0f;
        
        transmittance = float3(1.0f, 1.0f, 1.0f);
        
        for (int j = 0; j < StepCount; ++j)
        {
            float lightT = (j + 0.5f) * lightStepSize;
            float3 lightSamplePos = samplePos + lightRayDir * lightT;
            float lightHeight = length(lightSamplePos - EarthCenterAndRadius.xyz) - EarthCenterAndRadius.w;
            
            if (lightHeight < 0.0f)
            {
                transmittance = float3(0.0f, 0.0f, 0.0f);
                break;
            }
            
            float lightDensity = DensityAtHeight(lightHeight, densityFalloff);
            transmittance *= exp(-(RayleiCoef * lightDensity + MieCoef * lightDensity) * lightStepSize);
        }
        
        totalRayleigh += rayleighDensity * transmittance * stepSize * SunColor;
        totalMie += mieDensity * transmittance * stepSize * dot(SunColor, float3(0.333f, 0.333f, 0.333f));
    }
    
    float cosTheta = dot(rayDir, LightDirAndIntensity.xyz);
    float rayleighPhase = RayleighPhase(cosTheta);
    float miePhase = MiePhase(cosTheta, 0.76f);
    
    return (totalRayleigh * rayleighPhase + totalMie * miePhase) * SunColor * ScaterringIntensity;
}