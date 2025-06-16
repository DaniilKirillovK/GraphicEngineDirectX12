#pragma once

#include "D3D12Engine.h"


class GBuffer
{
public:
	GBuffer();
	~GBuffer();

	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferAlbedo = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferPosition = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> gBufferNormal = nullptr;
};

