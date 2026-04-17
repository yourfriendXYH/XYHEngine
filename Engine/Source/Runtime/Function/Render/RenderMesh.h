#pragma once
#include <Common.h>
#include <array>
#include <Runtime/Core/Math/Vector2.h>
#include <Runtime/Core/Math/Vector3.h>
#include <Runtime/Core/Math/Vector4.h>
#include "Interface/RHI.h"


NAMESPACE_XYH_BEGIN

struct ST_MeshVertex
{
	struct ST_VulkanMeshVertexPostition
	{
		Vector3 m_position;
	};

	struct ST_VulkanMeshVertexVaryingEnableBlending	// 启用蒙皮的可变顶点属性
	{
		Vector3 m_normal;
		Vector3 m_tangent;
	};

	struct ST_VulkanMeshVertexVarying
	{
		Vector2 m_texcoord;
	};

	struct ST_VulkanMeshVertexJointBinding
	{
		int m_indices[4];
		Vector4 m_weights;
	};

	// 获取顶点输入绑定描述
	static std::array<ST_RHIVertexInputBindingDescription, 3> GetBindingDescriptions()
	{
		std::array<ST_RHIVertexInputBindingDescription, 3> bindingDescriptions{};

		// position
		bindingDescriptions[0].m_binding = 0;
		bindingDescriptions[0].m_stride = sizeof(ST_VulkanMeshVertexPostition);
		bindingDescriptions[0].m_inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
		// varying blending
		bindingDescriptions[1].m_binding = 1;
		bindingDescriptions[1].m_stride = sizeof(ST_VulkanMeshVertexVaryingEnableBlending);
		bindingDescriptions[1].m_inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
		// varying
		bindingDescriptions[2].m_binding = 2;
		bindingDescriptions[2].m_stride = sizeof(ST_VulkanMeshVertexVarying);
		bindingDescriptions[2].m_inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	// 获取顶点输入属性描述
	static std::array<ST_RHIVertexInputAttributeDescription, 4> GetAttributeDescriptions()
	{
		std::array<ST_RHIVertexInputAttributeDescription, 4> attributeDescriptions{};

		// position
		attributeDescriptions[0].m_binding = 0;
		attributeDescriptions[0].m_location = 0;
		attributeDescriptions[0].m_format = RHI_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].m_offset = offsetof(ST_VulkanMeshVertexPostition, m_position);

		// varying blending
		attributeDescriptions[1].m_binding = 1;
		attributeDescriptions[1].m_location = 1;
		attributeDescriptions[1].m_format = RHI_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].m_offset = offsetof(ST_VulkanMeshVertexVaryingEnableBlending, m_normal);
		attributeDescriptions[2].m_binding = 1;
		attributeDescriptions[2].m_location = 2;
		attributeDescriptions[2].m_format = RHI_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].m_offset = offsetof(ST_VulkanMeshVertexVaryingEnableBlending, m_tangent);

		// varying
		attributeDescriptions[3].m_binding = 2;
		attributeDescriptions[3].m_location = 3;
		attributeDescriptions[3].m_format = RHI_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].m_offset = offsetof(ST_VulkanMeshVertexVarying, m_texcoord);

		return attributeDescriptions;
	}

	// testPass Vertex Input
	struct ST_TestVertexInput
	{
		Vector2 m_position;
		Vector3 m_color;
	};

	static std::array<ST_RHIVertexInputBindingDescription, 1> GetBindingDescriptionsTest()
	{
		std::array<ST_RHIVertexInputBindingDescription, 1> bindingDescriptions{};
		// position and color
		bindingDescriptions[0].m_binding = 0;
		bindingDescriptions[0].m_stride = sizeof(ST_TestVertexInput);
		bindingDescriptions[0].m_inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	static std::array<ST_RHIVertexInputAttributeDescription, 2> GetAttributeDescriptionsTest()
	{
		std::array<ST_RHIVertexInputAttributeDescription, 2> attributeDescriptions{};

		// position
		attributeDescriptions[0].m_binding = 0;
		attributeDescriptions[0].m_location = 0;
		attributeDescriptions[0].m_format = RHI_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].m_offset = offsetof(ST_TestVertexInput, m_position);

		// color
		attributeDescriptions[1].m_binding = 0;
		attributeDescriptions[1].m_location = 1;
		attributeDescriptions[1].m_format = RHI_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].m_offset = offsetof(ST_TestVertexInput, m_color);

		return attributeDescriptions;
	}

};

NAMESPACE_XYH_END