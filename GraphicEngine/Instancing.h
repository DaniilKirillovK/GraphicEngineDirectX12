#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"


struct alignas(16) InstanceData
{
	DirectX::XMFLOAT4X4 WorldMatrix;
	DirectX::XMFLOAT4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct InstanceDataRMDemo
{
	DirectX::XMFLOAT4X4 WorldMatrix;
	float Roughness;
	float Metallic;
};

