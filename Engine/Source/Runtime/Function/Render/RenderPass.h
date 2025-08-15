#pragma once
#include <Common.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include "Interface/RHIStruct.h"
#include "RenderPassBase.h"
#include "RenderCommon.h"

NAMESPACE_XYH_BEGIN

enum
{
	_main_camera_pass_gbuffer_a = 0,
	_main_camera_pass_gbuffer_b = 1,
	_main_camera_pass_gbuffer_c = 2,
	_main_camera_pass_backup_buffer_odd = 3,
	_main_camera_pass_backup_buffer_even = 4,
	_main_camera_pass_post_process_buffer_odd = 5,
	_main_camera_pass_post_process_buffer_even = 6,
	_main_camera_pass_depth = 7,
	_main_camera_pass_swap_chain_image = 8,
	_main_camera_pass_custom_attachment_count = 5,
	_main_camera_pass_post_process_attachment_count = 2,
	_main_camera_pass_attachment_count = 9,
};

enum
{
	_main_camera_subpass_basepass = 0,
	_main_camera_subpass_deferred_lighting,
	_main_camera_subpass_forward_lighting,
	_main_camera_subpass_tone_mapping,
	_main_camera_subpass_color_grading,
	_main_camera_subpass_fxaa,
	_main_camera_subpass_ui,
	_main_camera_subpass_combine_ui,
	_main_camera_subpass_count
};

struct ST_VisibleNodes
{
	std::vector<ST_RenderMeshNode>* m_pDirectionalLightVisibleMeshNodes = nullptr;	// 平行光可见网格节点
	std::vector<ST_RenderMeshNode>* m_pPointLightsVisibleMeshNodes = nullptr;	// 点光源可见网格节点
	std::vector<ST_RenderMeshNode>* m_pMainCameraVisibleMeshNodes = nullptr;	// 主摄像机可见网格节点
	ST_RenderAxisNode* m_pAxisNode = nullptr;	// 坐标轴节点
};

class RenderPass : public RenderPassBase
{
public:
	struct ST_FramebufferAttachment	// 帧缓冲区附件
	{
		RHIImage* m_pImage = nullptr;
		RHIDeviceMemory* m_pMemory = nullptr;
		RHIImageView* m_pView = nullptr;
		ERHIFormat m_format;
	};

	struct ST_Framebuffer	// 帧缓冲区
	{
		int m_width = 0;
		int m_height = 0;
		RHIFramebuffer* m_pFramebuffer = nullptr;
		RHIRenderPass* m_pRenderPass = nullptr;
		std::vector<ST_FramebufferAttachment> m_attachments;
	};

	struct ST_Descriptor	// 描述符
	{
		RHIDescriptorSetLayout* m_descriptorSetLayout = nullptr;	// 描述符集布局
		RHIDescriptorSet* m_descriptorSet = nullptr;	// 描述符集
	};

	struct ST_RenderPipelineBase	// 渲染管线
	{
		RHIPipelineLayout* m_pipelineLayout = nullptr;	// 管线布局
		RHIPipeline* m_pipeline = nullptr;	// 管线
	};

public:
	void Initialize(const ST_RenderPassInitInfo* init_info) override;
	void PostInitialize() override;

	virtual void Draw();

	virtual RHIRenderPass* GetRenderPass() const;
	virtual std::vector<RHIImageView*> GetFramebufferImageViews() const;	// 获取帧缓冲区图像视图列表
	virtual std::vector<RHIDescriptorSetLayout*> GetDescriptorSetLayouts() const;	// 获取描述符集布局列表

public:
	//GlobalRenderResource* m_global_render_resource{ nullptr };

	std::vector<ST_Descriptor> m_descriptorInfos;	// 描述符列表
	std::vector<ST_RenderPipelineBase> m_renderPipelines;	// 渲染管线列表
	ST_Framebuffer m_framebuffer;	// 帧缓冲区

	static ST_VisibleNodes s_visibleNodes;	// 可见节点
};

NAMESPACE_XYH_END