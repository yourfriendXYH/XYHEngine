#include "RenderScene.h"
#include "RenderResource.h"
#include "RenderCamera.h"

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
	for (auto it = m_meshObjectIdMap.begin(); it != m_meshObjectIdMap.end(); it++)
	{
		if (it->second == goId)
		{
			m_meshObjectIdMap.erase(it);
			break;	// ??? 只删除一个吗，是否存在一对多
		}
	}

	// 删除游戏对象的渲染实体
	ST_GameObjectPartId partId = { goId, 0 };
	size_t findGUID;
	if (m_instanceIdAllocator.GetElementGuid(partId, findGUID))
	{
		for (auto it = m_renderEntities.begin(); it != m_renderEntities.end(); it++)
		{
			//if (it->m_instanceId == findGUID)
			//{
			//	m_renderEntities.erase(it);
			//	break;
			//}
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

void RenderScene::UpdateVisibleObjectsDirectionalLight(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera)
{
}

void RenderScene::UpdateVisibleObjectsPointLight(std::shared_ptr<RenderResource> render_resource)
{
}

void RenderScene::UpdateVisibleObjectsMainCamera(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera)
{
}

void RenderScene::UpdateVisibleObjectsAxis(std::shared_ptr<RenderResource> render_resource)
{
}

void RenderScene::UpdateVisibleObjectsParticle(std::shared_ptr<RenderResource> render_resource)
{
}

NAMESPACE_XYH_END
