#include "Noise3DGenerator.h"

Noise3DGenerator::Noise3DGenerator(uint32_t seed) : m_rng(seed) 
{
    int p[256];
    for (int i = 0; i < 256; ++i) p[i] = i;

    std::shuffle(p, p + 256, m_rng);

    for (int i = 0; i < 512; ++i) 
    {
        m_permutation[i] = p[i & 255];
    }
}

void Noise3DGenerator::GenerateMountainDensity(uint32_t width, uint32_t height, uint32_t depth, float* output, float mountainIntensity)
{
    std::fill(output, output + width * height * depth, 0.0f);

    for (uint32_t z = 0; z < depth; ++z) 
    {
        for (uint32_t y = 0; y < height; ++y) 
        {
            for (uint32_t x = 0; x < width; ++x) 
            {
                float nx = (float)x / width * 2.0f;
                float ny = (float)y / height;
                float nz = (float)z / depth * 2.0f;

                float baseShape = PerlinNoise3D(nx * 0.8f, nz * 0.8f, 0) * 0.4f;

                float mediumDetail = PerlinNoise3D(nx * 3.0f, nz * 3.0f, 100) * 0.3f;

                float fineDetail = PerlinNoise3D(nx * 8.0f, nz * 8.0f, 200) * 0.15f;

                float softRidge = GenerateRidgeNoise(nx * 2.5f, nz * 2.5f) * 0.25f;

                float combinedNoise = baseShape +
                    mediumDetail * 0.7f +
                    fineDetail * 0.3f +
                    softRidge * 0.6f;

                combinedNoise = ApplyMountainCurve(combinedNoise);

                float surfaceHeight = 0.15f + combinedNoise * mountainIntensity * 0.8f;

                float verticalVariation = PerlinNoise3D(nx * 1.5f, ny * 2.0f, nz * 1.5f) * 0.05f;
                surfaceHeight += verticalVariation;

                float finalDensity = surfaceHeight - ny;
                output[z * width * height + y * width + x] = finalDensity;
            }
        }
    }
}


float Noise3DGenerator::PerlinNoise3D(float x, float y, float z)
{
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    float u = Fade(x);
    float v = Fade(y);
    float w = Fade(z);

    int A = m_permutation[X] + Y;
    int AA = m_permutation[A] + Z;
    int AB = m_permutation[A + 1] + Z;
    int B = m_permutation[X + 1] + Y;
    int BA = m_permutation[B] + Z;
    int BB = m_permutation[B + 1] + Z;

    float res = Lerp(w, Lerp(v, Lerp(u, Grad(m_permutation[AA], x, y, z),
        Grad(m_permutation[BA], x - 1, y, z)),
        Lerp(u, Grad(m_permutation[AB], x, y - 1, z),
            Grad(m_permutation[BB], x - 1, y - 1, z))),
        Lerp(v, Lerp(u, Grad(m_permutation[AA + 1], x, y, z - 1),
            Grad(m_permutation[BA + 1], x - 1, y, z - 1)),
            Lerp(u, Grad(m_permutation[AB + 1], x, y - 1, z - 1),
                Grad(m_permutation[BB + 1], x - 1, y - 1, z - 1))));
    return res;
}

float Noise3DGenerator::Fade(float t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float Noise3DGenerator::Lerp(float t, float a, float b)
{
    return a + t * (b - a);
}

float Noise3DGenerator::Grad(int hash, float x, float y, float z)
{
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float Noise3DGenerator::Ridge(float h, float offset)
{
    h = fabs(h);
    h = offset - h;
    h = h * h;
    return h;
}

float Noise3DGenerator::GenerateRidgeNoise(float x, float z)
{
    float noise = PerlinNoise3D(x, z, 300);
    noise = 1.0f - std::abs(noise * 0.8f);
    return noise * noise * 0.7f;
}

float Noise3DGenerator::ApplyMountainCurve(float value)
{
    if (value > 0) {
        return std::pow(value, 1.2f);
    }
    return value;
}
