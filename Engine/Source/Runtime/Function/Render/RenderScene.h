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
	void Clear();	// �����������

	void UpdateVisibleObjects(std::shared_ptr<RenderResource> renderResource, std::shared_ptr<RenderCamera> camera);

	// ����Ⱦͨ�������ÿɼ��ڵ��ָ��
	void SetVisibleNodesReference();

	//GuidAllocator<GameObjectPartId>& GetInstanceIdAllocator();
	GuidAllocator<ST_MeshSourceDesc>& GetMeshAssetIdAllocator();	// ��ȡ������ԴID������

	GuidAllocator<ST_MaterialSourceDesc>& GetMaterialAssetIdAllocator();	// ��ȡ������ԴID������

public:
	ST_AmbientLight m_ambientLight;	// ������
	ST_DirectionalLight m_directionalLight;	// ֱ���
	PointLightList m_pointLightList;	// ���Դ�б�

	std::vector<RenderEntity> m_renderEntities;	// ��Ⱦʵ���б�

	std::optional<RenderEntity> m_renderAxis;	// ��Ⱦ������

	std::vector<ST_RenderMeshNode> m_directionalLightVisibleMeshNodes;	// ֱ���ɼ�����ڵ��б�
	std::vector<ST_RenderMeshNode> m_pointLightsVisibleMeshNodes;	// ���Դ�ɼ�����ڵ��б�
	std::vector<ST_RenderMeshNode> m_mainCameraVisibleMeshNodes;	// ��������ɼ�����ڵ��б�
	ST_RenderAxisNode m_axisNode;	// ������ڵ�



private:


private:

	//GuidAllocator<GameObjectPartId>   m_instance_id_allocator;
	GuidAllocator<ST_MeshSourceDesc> m_meshAssetIdAllocator;
	GuidAllocator<ST_MaterialSourceDesc> m_materialAssetIdAllocator;

};

NAMESPACE_XYH_END