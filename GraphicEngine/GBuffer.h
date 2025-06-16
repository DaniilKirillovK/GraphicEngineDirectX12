#pragma once

#include "d3dUtil.h"

class GBuffer
{
public:
	GBuffer();
	~GBuffer();

	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferAlbedo = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferPosition = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferNormal = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferSpecular = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferDepth = nullptr;
};

