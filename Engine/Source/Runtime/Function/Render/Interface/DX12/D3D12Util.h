#pragma once
#include "D3D12RHI.h"
#include <unordered_map>

NAMESPACE_XYH_BEGIN

class D3D12Util
{
public:

	// 创建resource
	static void CreateResource(
		ID3D12Resource*& outResource,
		ID3D12Device* pDevice,
		D3D12_HEAP_TYPE heapType,
		D3D12_RESOURCE_DIMENSION dimension,
		UINT width,
		UINT height,
		UINT16 mipLevels,
		DXGI_FORMAT format,
		UINT sampleCount,
		D3D12_TEXTURE_LAYOUT layout,
		D3D12_RESOURCE_FLAGS flags,
		D3D12_RESOURCE_STATES resStates,
		D3D12_CLEAR_VALUE* pClearValue
	);

	//
	static ID3D12Resource* CreateBufferObject(ID3D12GraphicsCommandList* pCommandList, ID3D12Device* pDevice, void* pData, int dataLength, D3D12_RESOURCE_STATES dstStates);

	// 创建ConstantBuffer
	static ID3D12Resource* CreateConstantBufferObject(ID3D12Device* pDevice, int dataLength);

	// 更新ConstantBuffer
	static void UpdateConstantBuffer(ID3D12Resource* pConstantBuffer, const void* pData, int dataLength);

	static D3D12_RESOURCE_BARRIER InitResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES srcState, D3D12_RESOURCE_STATES dstState);

private:

};

// 静态网格组件，用于封装顶点数据（临时使用）
struct ST_StaticMeshComponentVertexData
{
	float m_position[4];
	float m_texcoord[4];
	float m_normal[4];
	float m_tangent[4];
};

class SubMesh
{
public:
	ID3D12Resource* m_pIBO = nullptr;

	D3D12_INDEX_BUFFER_VIEW m_iboView;

	unsigned int m_indexCount = 0;
};

class StaticMeshComponent
{
public:

	void InitFromFile(ID3D12GraphicsCommandList* pCommandList, ID3D12Device* pDevice, const char* filePath);

	void SetVertexCount(int vertexCount);

	void SetVertexPosition(int index, float x, float y, float z, float w = 1.0f);

	void SetVertexTexcoord(int index, float x, float y, float z, float w = 1.0f);

	void SetVertexNormal(int index, float x, float y, float z, float w = 1.0f);

	void SetVertexTangent(int index, float x, float y, float z, float w = 1.0f);

	const int GetVertexDataSize() const;

	const int OnceVertexDataSize() const;

public:
	ID3D12Resource* m_pVBO = nullptr;

	D3D12_VERTEX_BUFFER_VIEW m_vboView;

	int m_vertexCount;
	ST_StaticMeshComponentVertexData* m_vertexData;

	// 模型的索引数据
	std::unordered_map<std::string, SubMesh*> m_subMeshData;
};

NAMESPACE_XYH_END