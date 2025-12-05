#include "TAAUtility.h"
#include "Common.h"



void TAAUtility::GenerateJitter()
{
	JitterValue = DirectX::XMFLOAT2(
		Halton(SamplerIndex & 1023, 2),
		Halton(SamplerIndex & 1023, 3)
	);

	SamplerIndex = (SamplerIndex + 1) % SampleCount;
}

DirectX::XMMATRIX TAAUtility::GetJitterPerspectiveProjectionMatrix(float nearP, float farP, float verticalFOV,
	float aspect, DirectX::XMMATRIX projMatrix)
{
	float n = nearP;
	float f = farP;

	float v = verticalFOV * n;
	float h = v * aspect;

	DirectX::XMFLOAT2 jitter = DirectX::XMFLOAT2(
		JitterValue.x * h / (0.5f * WINDOW_WIDTH),
		JitterValue.y * v / (0.5f * WINDOW_HEIGHT)
	);

	DirectX::XMFLOAT4X4 matrix;
	DirectX::XMStoreFloat4x4(&matrix, projMatrix);

	matrix._31 += jitter.x / h;
	matrix._32 += jitter.y / v;

	DirectX::XMMATRIX result = DirectX::XMLoadFloat4x4(&matrix);

	return result;
}

float TAAUtility::Halton(int index, int base)
{
	float f = 1.0f;
	float result = 0.0f;

	while (index > 0)
	{
		f /= static_cast<float>(base);
		result = result + f * static_cast<float>(index % base);
		index = static_cast<int>(floorf(static_cast<float>(index) / static_cast<float>(base)));
	}

	return result;
}
