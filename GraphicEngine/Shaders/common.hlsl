float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

float2 ParallaxMapping(float2 texCoords, float3 viewDir, SamplerState samplerState, Texture2D<float4> displacementMap, float heightScale = 0.05f)
{
    // Number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = lerp(minLayers, maxLayers, abs(dot(float3(0, 1, 0), viewDir)));
    
    // Calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // Current depth of the layer
    float currentLayerDepth = 0.0;
    // Amount to shift the texture coordinates per layer
    float2 P = viewDir.xy * heightScale;
    float2 deltaTexCoords = P / numLayers;
    
    // Get initial values
    float2 currentTexCoords = texCoords;
    float currentDepthMapValue = displacementMap.SampleLevel(samplerState, currentTexCoords, 0).r;
    
    // Parallax occlusion mapping
    while (currentLayerDepth < currentDepthMapValue)
    {
        // Shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // Get depthmap value at current texture coordinates
        currentDepthMapValue = displacementMap.SampleLevel(samplerState, currentTexCoords, 0).r;
        // Get depth of next layer
        currentLayerDepth += layerDepth;
    }
    
    // Parallax occlusion mapping refinement
    float2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = displacementMap.SampleLevel(samplerState, prevTexCoords, 0).r - currentLayerDepth + layerDepth;
    
    float weight = afterDepth / (afterDepth - beforeDepth);
    float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    
    return finalTexCoords;
}