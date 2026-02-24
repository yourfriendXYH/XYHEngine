#include "RenderScene.h"
#include "RenderResource.h"
#include "RenderCamera.h"
#include "RenderPass.h"
#include <set>
#include "RenderHelper.h"

NAMESPACE_XYH_BEGIN

void RenderScene::Clear()
{
}

void RenderScene::UpdateVisibleObjects(std::shared_ptr<RenderResource> renderResource, std::shared_ptr<RenderCamera> camera)
{
	// 更新可见对象
	UpdateVisibleObjectsDirectionalLight(renderResource, camera);

	UpdateVisibleObjectsPointLight(renderResource);

	UpdateVisibleObjectsMainCamera(renderResource, camera);

	UpdateVisibleObjectsAxis(renderResource);

	UpdateVisibleObjectsParticle(renderResource);
}

void RenderScene::SetVisibleNodesReference()
{
	RenderPass::s_visibleNodes.m_pDirectionalLightVisibleMeshNodes = &m_directionalLightVisibleMeshNodes;
	RenderPass::s_visibleNodes.m_pPointLightsVisibleMeshNodes = &m_pointLightsVisibleMeshNodes;
	RenderPass::s_visibleNodes.m_pMainCameraVisibleMeshNodes = &m_mainCameraVisibleMeshNodes;
	RenderPass::s_visibleNodes.m_pAxisNode = &m_axisNode;
}

GuidAllocator<ST_GameObjectPartId>& RenderScene::GetInstanceIdAllocator()
{
	return m_instanceIdAllocator;
}

GuidAllocator<ST_MeshSourceDesc>& RenderScene::GetMeshAssetIdAllocator()
{
	return m_meshAssetIdAllocator;
}

GuidAllocator<ST_MaterialSourceDesc>& RenderScene::GetMaterialAssetIdAllocator()
{
	return m_materialAssetIdAllocator;
}

void RenderScene::AddInstanceIdToMap(uint32_t instanceId, GObjectID goId)
{
	m_meshObjectIdMap[instanceId] = goId;
}

GObjectID RenderScene::GetGObjectIDByMeshID(uint32_t meshId) const
{
	auto findIter = m_meshObjectIdMap.find(meshId);
	if (findIter != m_meshObjectIdMap.end())
	{
		return findIter->second;
	}
	return GObjectID();
}

void RenderScene::DeleteEntityByGObjectID(GObjectID goId)
{
	std::set<uint32_t> deletePartGUIDs;	// 需要删除的GUID
	for (auto it = m_meshObjectIdMap.begin(); it != m_meshObjectIdMap.end(); it++)
	{
		if (it->second == goId)
		{
			m_meshObjectIdMap.erase(it);

			deletePartGUIDs.insert(it->first);
		}
	}

	// 删除GameObject包含的所有RenderEntity
	// 删除游戏对象的渲染实体
	for (auto it = m_renderEntities.begin(); it != m_renderEntities.end(); it++)
	{
		if (deletePartGUIDs.find(it->m_instanceId) != deletePartGUIDs.end())
		{
			m_renderEntities.erase(it);
		}
	}
}

void RenderScene::ClearForLevelReloading()
{
	// 清除场景数据
	m_instanceIdAllocator.clear();
	m_meshObjectIdMap.clear();
	m_renderEntities.clear();
}

void RenderScene::UpdateVisibleObjectsDirectionalLight(std::shared_ptr<RenderResource> pRenderResource, std::shared_ptr<RenderCamera> pCamera)
{
	Matrix4x4 directionalLightProjView = CalculateDirectionalLightCamera(*this, *pCamera);

	pRenderResource->m_meshPerframeStorageBufferObject.m_directionalLightProjView = directionalLightProjView;
	pRenderResource->m_meshDirectionalLightShadowPerframeStorageBufferObject.m_lightProjView = directionalLightProjView;

	m_directionalLightVisibleMeshNodes.clear();

	ST_ClusterFrustum frustum = CreateClusterFrustumFromMatrix(directionalLightProjView, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);

	for (const RenderEntity& entity : m_renderEntities)
	{
		ST_BoundingBox meshAssetBoundingBox{ entity.m_boundingBox.GetMinCorner(), entity.m_boundingBox.GetMaxCorner() };
		// 在视锥体可见范围内
		if (TiledFrustumIntersectBox(frustum, BoundingBoxTransform(meshAssetBoundingBox, entity.m_modelMatrix)))
		{
			m_directionalLightVisibleMeshNodes.emplace_back();
			ST_RenderMeshNode& refNode = m_directionalLightVisibleMeshNodes.back();

			// 模型矩阵
			refNode.m_modelMatrix = &entity.m_modelMatrix;

			// 顶点混合
			assert(entity.m_jointMatrices.size() < s_meshVertexBlendingMaxJointCount);
			if (!entity.m_jointMatrices.empty())
			{
				refNode.m_jointCount = entity.m_jointMatrices.size();
				refNode.m_jointMatrices = entity.m_jointMatrices.data();
			}

			// ID
			refNode.m_nodeId = entity.m_instanceId;

			// 网格
			ST_VulkanMesh& meshAsset = pRenderResource->GetEntityMesh(entity);
			refNode.m_refMesh = &meshAsset;
			refNode.m_enableVertexBlending = entity.m_enableVertexBlending;

			// 材质
			ST_VulkanPBRMaterial& materialAsset = pRenderResource->GetEntityMaterial(entity);
			refNode.m_refMaterial = &materialAsset;
		}
	}
}

void RenderScene::UpdateVisibleObjectsPointLight(std::shared_ptr<RenderResource> pRenderResource)
{
	// 清空点光源可见Node
	m_pointLightsVisibleMeshNodes.clear();

	std::vector<ST_BoundingSphere> pointLightBoundingSpheres;
	uint32_t pointLightNum = static_cast<uint32_t>(m_pointLightList.m_lights.size());
	pointLightBoundingSpheres.resize(pointLightNum);

	for (size_t i = 0; i < pointLightNum; i++)
	{
		pointLightBoundingSpheres[i].m_center = m_pointLightList.m_lights[i].m_position;
		pointLightBoundingSpheres[i].m_radius = m_pointLightList.m_lights[i].CalculateRadius();
	}

	for (const RenderEntity& entity : m_renderEntities)
	{
		ST_BoundingBox meshAssetBoundingBox{ entity.m_boundingBox.GetMinCorner(), entity.m_boundingBox.GetMaxCorner() };

		bool intersectWithPointLights = true;
		for (size_t i = 0; i < pointLightNum; i++)
		{
			// 判断包围盒是否在点光源范围内
			if (!BoxIntersectsWithSphere(BoundingBoxTransform(meshAssetBoundingBox, entity.m_modelMatrix), pointLightBoundingSpheres[i]))
			{
				intersectWithPointLights = false;
				break;
			}
		}

		if (intersectWithPointLights)
		{
			m_directionalLightVisibleMeshNodes.emplace_back();
			ST_RenderMeshNode& refNode = m_directionalLightVisibleMeshNodes.back();

			// 模型矩阵
			refNode.m_modelMatrix = &entity.m_modelMatrix;

			// 顶点混合
			assert(entity.m_jointMatrices.size() < s_meshVertexBlendingMaxJointCount);
			if (!entity.m_jointMatrices.empty())
			{
				refNode.m_jointCount = entity.m_jointMatrices.size();
				refNode.m_jointMatrices = entity.m_jointMatrices.data();
			}

			// ID
			refNode.m_nodeId = entity.m_instanceId;

			// 网格
			ST_VulkanMesh& meshAsset = pRenderResource->GetEntityMesh(entity);
			refNode.m_refMesh = &meshAsset;
			refNode.m_enableVertexBlending = entity.m_enableVertexBlending;

			// 材质
			ST_VulkanPBRMaterial& materialAsset = pRenderResource->GetEntityMaterial(entity);
			refNode.m_refMaterial = &materialAsset;
		}
	}
}

void RenderScene::UpdateVisibleObjectsMainCamera(std::shared_ptr<RenderResource> pRenderResource, std::shared_ptr<RenderCamera> pCamera)
{
	m_mainCameraVisibleMeshNodes.clear();

	Matrix4x4 viewMatrix = pCamera->GetViewMatrix();
	Matrix4x4 projMatrix = pCamera->GetPersProjMatrix();
	Matrix4x4 projViewMatrix = projMatrix * viewMatrix;

	ST_ClusterFrustum frustum = CreateClusterFrustumFromMatrix(projViewMatrix, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);

	for (const RenderEntity& entity : m_renderEntities)
	{
		ST_BoundingBox meshAssetBoundingBox{ entity.m_boundingBox.GetMinCorner(), entity.m_boundingBox.GetMaxCorner() };

		if (TiledFrustumIntersectBox(frustum, BoundingBoxTransform(meshAssetBoundingBox, entity.m_modelMatrix)))
		{
			m_directionalLightVisibleMeshNodes.emplace_back();
			ST_RenderMeshNode& refNode = m_directionalLightVisibleMeshNodes.back();

			// 模型矩阵
			refNode.m_modelMatrix = &entity.m_modelMatrix;

			// 顶点混合
			assert(entity.m_jointMatrices.size() < s_meshVertexBlendingMaxJointCount);
			if (!entity.m_jointMatrices.empty())
			{
				refNode.m_jointCount = entity.m_jointMatrices.size();
				refNode.m_jointMatrices = entity.m_jointMatrices.data();
			}

			// ID
			refNode.m_nodeId = entity.m_instanceId;

			// 网格
			ST_VulkanMesh& meshAsset = pRenderResource->GetEntityMesh(entity);
			refNode.m_refMesh = &meshAsset;
			refNode.m_enableVertexBlending = entity.m_enableVertexBlending;

			// 材质
			ST_VulkanPBRMaterial& materialAsset = pRenderResource->GetEntityMaterial(entity);
			refNode.m_refMaterial = &materialAsset;
		}
	}
}

void RenderScene::UpdateVisibleObjectsAxis(std::shared_ptr<RenderResource> pRenderResource)
{
	if (m_renderAxis.has_value())
	{
		RenderEntity& axis = *m_renderAxis;

		// 模型矩阵
		m_axisNode.m_modelMatrix = axis.m_modelMatrix;

		// ID
		m_axisNode.m_nodeId = axis.m_instanceId;

		// 网格
		ST_VulkanMesh& meshAsset = pRenderResource->GetEntityMesh(axis);
		m_axisNode.m_refMesh = &meshAsset;

		// 顶点混合
		m_axisNode.m_enableVertexBlending = axis.m_enableVertexBlending;
	}
}

void RenderScene::UpdateVisibleObjectsParticle(std::shared_ptr<RenderResource> pRenderResource)
{
	// TODO
}

NAMESPACE_XYH_END
