static const float PI = 3.14159265f;

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    cosTheta = saturate(cosTheta);
    float oneMinusCosTheta = 1.0 - cosTheta;
    float oneMinusCosTheta2 = oneMinusCosTheta * oneMinusCosTheta;
    float oneMinusCosTheta5 = oneMinusCosTheta2 * oneMinusCosTheta2 * oneMinusCosTheta;
    
    return lerp(F0, float3(1.0, 1.0, 1.0), oneMinusCosTheta5);
}

float3 CalculatePBR(float3 N, float3 V, float3 L, float3 radiance, float roughness,
    float3 F0, float metallic, float4 albedo)
{
    float3 H = normalize(V + L);
    
    // BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * albedo.rgb / PI + specular) * radiance * NdotL;
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) *
           pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float GetAdjustedIntensity(float baseIntensity, float metallic)
{
    return baseIntensity * lerp(0.1, 1.0, metallic);
}
