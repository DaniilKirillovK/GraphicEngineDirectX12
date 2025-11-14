#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <vector>
#include <string>
#include "d3dx12.h"
#include "d3dUtil.h"

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetMoreStaticSamplers();

std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> GetLODStaticSamplers();

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 9> GetLODStaticSamplersShadow();

