#pragma once
#include <DirectXMath.h>
#include <vector>
#include <random>

class Noise3DGenerator 
{
public:
    Noise3DGenerator(uint32_t seed = 12345);
    ~Noise3DGenerator() = default;

    void GenerateMountainDensity(uint32_t width, uint32_t height, uint32_t depth,
        float* output, float mountainIntensity = 1.0f);

private:
    float PerlinNoise3D(float x, float y, float z);
    float Fade(float t);
    float Lerp(float t, float a, float b);
    float Grad(int hash, float x, float y, float z);
    float Ridge(float h, float offset);

    float GenerateRidgeNoise(float x, float z);
    float ApplyMountainCurve(float value);

    std::mt19937 m_rng;
    int m_permutation[512];
};
