#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "FrameResource.h"

struct Particle
{
	DirectX::XMFLOAT3 Position;
	float Age;
	DirectX::XMFLOAT3 Velocity;
	float Weight = 1.0f;
	DirectX::XMFLOAT3 Acceleration;
	float Size;
	DirectX::XMFLOAT4 Color;
	float LifeTime;
	UINT ParticleType = 0;
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

	virtual void RenderGPU(ID3D12GraphicsCommandList* cmdList,
		const std::vector<RenderItem*>& ritems,
		UINT srvDescriptorSize,
		FrameResource* currFrameResource,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap,
		UINT ParticlesID, std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatures,
		UINT CB1, UINT SRV1, Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> texResource);

	virtual void InitializeSystem(Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12Resource> particleSRVBuffers[2],
		Microsoft::WRL::ComPtr<ID3D12Resource>& particleArgsBuffer,
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
