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

struct InstanceDataMoreLight
{
	DirectX::XMFLOAT4X4 WorldMatrix;
	UINT lightID;
	DirectX::XMFLOAT3 Pad = { 0.0f, 0.0f, 0.0f };
};

struct InstanceDataGameObject
{
	DirectX::XMFLOAT4X4 WorldMatrix;
	DirectX::XMFLOAT4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

