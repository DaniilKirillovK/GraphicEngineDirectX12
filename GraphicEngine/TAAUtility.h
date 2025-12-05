#pragma once
#include <DirectXCollision.h>
#include <vector>
#include <memory>
#include <cstdint>
#include <d3d12.h>
#include "FrameResource.h"
#include <queue>
#include "GeometryGenerator.h"
#include <cmath>


class TAAUtility
{
public:
	static void GenerateJitter();
	static DirectX::XMFLOAT2 GetJitterValue() { return JitterValue; }

	static DirectX::XMMATRIX GetJitterPerspectiveProjectionMatrix(float nearP, float farP, float verticalFOV,
		float aspect, DirectX::XMMATRIX projMatrix);

	static inline Microsoft::WRL::ComPtr<ID3D12Resource> m_RTVs[2];
	static inline Microsoft::WRL::ComPtr<ID3D12Resource> m_VelocityBuffer;
	static inline int m_CurrentRTV = 0;

private:
	static inline int SamplerIndex = 0;
	static inline const int SampleCount = 8;

	static inline DirectX::XMFLOAT2 JitterValue;
	static float Halton(int index, int base);
};

