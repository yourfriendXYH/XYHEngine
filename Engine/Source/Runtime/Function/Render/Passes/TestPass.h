#pragma once
#include <Common.h>
#include <Runtime/Function/Render/RenderPass.h>
#include <d3d12.h>

#include <Runtime/Function/Render/Interface/OpenGL/OpenGLUtil.h>
#include <Runtime/Function/Render/Interface/DX12/D3D12Util.h>

NAMESPACE_XYH_BEGIN

class TestPass : public RenderPass
{
public:
	void Initialize(const ST_RenderPassInitInfo* initInfo) override final;

	void Draw() override;

	// 交换链重新创建时需要处理的操作
	void UpdateAfterFramebufferRecreate();

	// 准备部分数据
	void PreparePassData(std::shared_ptr<RenderResourceBase> renderResource) override final;

private:

	void SetupAttachments();	// 设置附件

	void SetupRenderPass();	// 设置渲染通道

	void SetupDescriptorSetLayout();	// 设置描述符集布局

	void SetupDescriptorSet();	// 设置描述符集

	void SetupPipelines();	// 设置管线

	void SetupSwapchainFramebuffers();	// 设置交换链帧缓冲

	void CreateVertexBuffer();
	void CreateIndexBuffer();

	void VulkanDrawTest();

	void D3D12DrawTest();

	void OpenGLDrawTest();

private:

	// 没有使用基类RenderPass中的Framebuffer指针
	std::vector<RHIFramebuffer*> m_swapchainFramebuffers;	// 交换链帧缓冲

	RHIBuffer* m_pVertexBuffer = nullptr;
	VmaAllocation m_vertexBufferAllocation;

	RHIBuffer* m_pIndexBuffer = nullptr;
	VmaAllocation m_indexBufferAllocation;

	RHIBuffer* m_pPerframeStorageBuffer = nullptr;
	RHIDeviceMemory* m_pPerframeMemory = nullptr;

	RHIBuffer* m_pPerDrawcallStorageBuffer = nullptr;
	RHIDeviceMemory* m_pPerDrawcallMemory = nullptr;


	// D3D12
	ID3D12PipelineState* m_pPSO = nullptr;

	ID3D12RootSignature* m_pRootSignature = nullptr;

	std::array<D3D12_VERTEX_BUFFER_VIEW, 1> m_vbos{};

	D3D12_INDEX_BUFFER_VIEW m_ibo;

	float m_testColor[4] = { 0.5, 0.5, 0.5, 1.0 };

	ID3D12Resource* m_pTestConstantBuffer = nullptr;

	StaticMeshComponent m_testMesh;

	ID3D12Resource* m_pTestTexture = nullptr;

	ID3D12Resource* m_pTestTexture1 = nullptr;

	ID3D12Resource* m_pStructBuffer = nullptr;

	std::vector<ID3D12DescriptorHeap*> m_descriptorHeaps;

	// Nanite

	FullScreenQuad m_FSQ;
	GLuint m_shaderProgramFSQ;


	RHIBuffer* m_workArgs[2];	// 间接绘制命令Buffer

	RHIBuffer* m_pGlobalConstants = nullptr;	// 全局使用的UniformBuffer

	RHIBuffer* m_pPerDrawcallBuffer = nullptr;	// 存储NaniteMesh的modelMatrix

	RHIBuffer* m_pNaniteMesh = nullptr;	// 

	RHIBuffer* m_pVisibleClusterSoftwareHardware = nullptr;	// 

	RHIBuffer* m_pMainAndPostNodeAndClusterBatches = nullptr;

	RHIBuffer* m_pBVH = nullptr;

	std::shared_ptr<RenderPass> m_pRasterClearPass = nullptr;

	// 多Pass遍历四叉树（目前已知只有4层）
	static constexpr size_t m_hierarchySize = 4u;
	std::array<std::shared_ptr<RenderPass>, m_hierarchySize> m_pNodeAndClusterCullPass;

	std::shared_ptr<RenderPass> m_pClusterCullPass = nullptr;

	std::shared_ptr<RenderPass> m_pHardwareRasterizePass = nullptr;


	GLuint m_visBuffer64;	// 每一个像素用64位存储（32位：深度值 ）

	OpenGLImage* m_pVisualizationTexture = nullptr;

	std::shared_ptr<RenderPass> m_pVisualizationPass = nullptr;

	// 每帧用到的通用数据（全局数据）
	ST_TestPerframeStorageBufferObject m_perframeStorageBufferObj{};

	ST_TestPerDrawcallStorageBufferObject m_perDrawcallStorageBufferObj;

	int m_tempDegree = 0;
};

NAMESPACE_XYH_END