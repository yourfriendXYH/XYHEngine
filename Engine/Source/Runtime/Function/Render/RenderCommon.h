#pragma once
#include <Common.h>
#include <Runtime/Core/Math/Matrix4.h>
#include "RenderType.h"
#include "Interface/RHI.h"

NAMESPACE_XYH_BEGIN

struct ST_VulkanMesh
{
	bool m_enableVertexBlending;    // �Ƿ����ö�����

	// ���㻺����
	uint32_t m_meshVertexCount; // ���񶥵�����
	RHIBuffer* m_meshVertexPositionBuffer;  // ���񶥵�λ�û�����
	VmaAllocation m_meshVertexPositionBufferAllocation; // ���񶥵�λ�û���������

	RHIBuffer* m_meshVertexVaryingEnableBlendingBuffer; // ���񶥵�������û�ϻ�����
    VmaAllocation m_meshVertexVaryingEnableBlendingBufferAllocation;

	RHIBuffer* m_meshVertexJointBindingBuffer;  // ���񶥵�ؽڰ󶨻�����
	VmaAllocation m_meshVertexJointBindingBufferAllocation; // ���񶥵�ؽڰ󶨻���������

	RHIDescriptorSet* m_meshVertexBlendingDescriptorSet;    // ���񶥵�����������

	RHIBuffer* m_meshVertexVaryingBuffer;   // ���񶥵���컺����
    VmaAllocation m_meshVertexVaryingBufferAllocation;

	// ����������
    uint32_t m_meshIndexCount;
    RHIBuffer* m_meshIndexBuffer;
    VmaAllocation m_meshIndexBufferAllocation;
};

// material
struct ST_VulkanPBRMaterial
{
	RHIImage* m_baseColorTextureImage;  // ������ɫ����ͼ��
	RHIImageView* m_baseColorImageView;
    VmaAllocation m_baseColorImageAllocation;

	RHIImage* m_metallicRoughnessTextureImage;  // �����Ⱥʹֲڶ�����ͼ��
    RHIImageView* m_metallicRoughnessImageView;
    VmaAllocation m_metallicRoughnessImageAllocation;

	RHIImage* m_normalTextureImage;	// ��������ͼ��
    RHIImageView* m_normalImageView;
    VmaAllocation m_normalImageAllocation;

	RHIImage* m_occlusionTextureImage;  // �ڵ�����ͼ��
    RHIImageView* m_occlusionImageView;
    VmaAllocation m_occlusionImageAllocation;

	RHIImage* m_emissiveTextureImage;   // �Է�������ͼ��
    RHIImageView* m_emissiveImageView;
    VmaAllocation m_emissiveImageAllocation;

	RHIBuffer* m_materialUniformBuffer; // ����ͳһ������
	VmaAllocation m_materialUniformBufferAllocation;

	RHIDescriptorSet* m_materialDescriptorSet;  // ������������
};

struct ST_RenderMeshNode
{
	const Matrix4x4* m_modelMatrix = nullptr;  // ģ�;���
	const Matrix4x4* m_jointMatrices = nullptr;    // �ؽھ���
	uint32_t m_jointCount = 0u; // �ؽ�����
    ST_VulkanMesh* m_refMesh = nullptr;	// ��������
	ST_VulkanPBRMaterial* m_refMaterial = nullptr;	// ��������
    uint32_t m_nodeId;
	bool m_enableVertexBlending = false;	// �Ƿ����ö�����
};

struct ST_RenderAxisNode
{
	Matrix4x4 m_modelMatrix = Matrix4x4::IDENTITY;
	ST_VulkanMesh* m_refMesh = nullptr;
	uint32_t m_nodeId;
	bool m_enableVertexBlending = false;
};

NAMESPACE_XYH_END