#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "FrameResource.h"

struct Particle
{
	DirectX::XMFLOAT3 Position;
	float Age;
	DirectX::XMFLOAT3 Velocity;
	float Weight;
	DirectX::XMFLOAT3 Acceleration;
	float Size;
	DirectX::XMFLOAT4 Color;
	float LifeTime;
};

struct DrawInstancedArgs 
{
	UINT VertexCountPerInstance; 
	UINT InstanceCount;          
	UINT StartVertexLocation;    
	UINT StartInstanceLocation;
};

class ParticleSystem
{
public:
	ParticleSystem(int maxParticles, 
		DirectX::XMFLOAT3 position,
		int systemID);

	ParticleSystem();
	
	virtual void Update(float elapsedTime);

	virtual void Render(ID3D12GraphicsCommandList* cmdList,
		const std::vector<RenderItem*>& ritems,
		UINT srvDescriptorSize,
		FrameResource* currFrameResource,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap);

	virtual void InitializeSystem(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12Resource> particleSRVBuffers[2],
		Microsoft::WRL::ComPtr<ID3D12Resource> particleArgsBuffers[2],
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvUavHeap,
		UINT srvDescriptorSize,
		UINT offset);

	EmitterConstants emitterData;
	Particle* particlesData;

	virtual void BuildSystemVertexBuffers(
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> &geometries,
		Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr< ID3D12GraphicsCommandList> cmdList
	);
};
