#pragma once
#include <optional>
#include <Common.h>
#include "Light.h"
#include "RenderEntity.h"
#include "RenderCommon.h"
#include "RenderGuidAllocator.h"

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

	//GuidAllocator<GameObjectPartId>& GetInstanceIdAllocator();
	GuidAllocator<ST_MeshSourceDesc>& GetMeshAssetIdAllocator();	// 获取网格资源ID分配器

	GuidAllocator<ST_MaterialSourceDesc>& GetMaterialAssetIdAllocator();	// 获取材质资源ID分配器

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


private:

	//GuidAllocator<GameObjectPartId>   m_instance_id_allocator;
	GuidAllocator<ST_MeshSourceDesc> m_meshAssetIdAllocator;
	GuidAllocator<ST_MaterialSourceDesc> m_materialAssetIdAllocator;

};

NAMESPACE_XYH_END