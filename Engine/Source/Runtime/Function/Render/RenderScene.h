#pragma once
#include <optional>
#include <Common.h>
#include "Light.h"
#include "RenderEntity.h"
#include "RenderCommon.h"
#include "RenderGuidAllocator.h"
#include "RenderObject.h"
#include "Runtime/Function/Framework/Object/ObjectIdAllocator.h"

NAMESPACE_XYH_BEGIN

class RenderResource;
class RenderCamera;

class RenderScene
{
public:
	void Clear();	// 清除场景数据

	void UpdateVisibleObjects(std::shared_ptr<RenderResource> renderResource, std::shared_ptr<RenderCamera> camera);

	// 在渲染通道中设置可见节点的指针
	void SetVisibleNodesReference();

	GuidAllocator<ST_GameObjectPartId>& GetInstanceIdAllocator();
	GuidAllocator<ST_MeshSourceDesc>& GetMeshAssetIdAllocator();	// 获取网格资源ID分配器

	GuidAllocator<ST_MaterialSourceDesc>& GetMaterialAssetIdAllocator();	// 获取材质资源ID分配器

	void AddInstanceIdToMap(uint32_t instanceId, GObjectID goId);	// 将实例ID添加到映射中
	GObjectID GetGObjectIDByMeshID(uint32_t meshId) const;	// 根据网格ID获取游戏对象ID
	void DeleteEntityByGObjectID(GObjectID goId);	// 根据游戏对象ID删除实体

	void ClearForLevelReloading();	// 清理用于关卡重新加载的资源

public:
	ST_AmbientLight m_ambientLight;	// 环境光
	ST_DirectionalLight m_directionalLight;	// 直射光
	PointLightList m_pointLightList;	// 点光源列表

	std::vector<RenderEntity> m_renderEntities;	// 渲染实体列表

	std::optional<RenderEntity> m_renderAxis;	// 渲染坐标轴

	std::vector<ST_RenderMeshNode> m_directionalLightVisibleMeshNodes;	// 直射光可见网格节点列表
	std::vector<ST_RenderMeshNode> m_pointLightsVisibleMeshNodes;	// 点光源可见网格节点列表
	std::vector<ST_RenderMeshNode> m_mainCameraVisibleMeshNodes;	// 主摄像机可见网格节点列表
	ST_RenderAxisNode m_axisNode;	// 坐标轴节点



private:
	void UpdateVisibleObjectsDirectionalLight(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);	// 更新直射光可见对象
	void UpdateVisibleObjectsPointLight(std::shared_ptr<RenderResource> render_resource);	// 更新点光源可见对象
	void UpdateVisibleObjectsMainCamera(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);	// 更新主摄像机可见对象
	void UpdateVisibleObjectsAxis(std::shared_ptr<RenderResource> render_resource);	// 更新坐标轴可见对象
	void UpdateVisibleObjectsParticle(std::shared_ptr<RenderResource> render_resource);	// 更新粒子可见对象

private:

	GuidAllocator<ST_GameObjectPartId> m_instanceIdAllocator;
	GuidAllocator<ST_MeshSourceDesc> m_meshAssetIdAllocator;
	GuidAllocator<ST_MaterialSourceDesc> m_materialAssetIdAllocator;

	std::unordered_map<uint32_t, GObjectID> m_meshObjectIdMap;	// 网格对象ID映射

};

NAMESPACE_XYH_END