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
	void Clear();	// �����������

	void UpdateVisibleObjects(std::shared_ptr<RenderResource> renderResource, std::shared_ptr<RenderCamera> camera);

	// ����Ⱦͨ�������ÿɼ��ڵ��ָ��
	void SetVisibleNodesReference();

	GuidAllocator<ST_GameObjectPartId>& GetInstanceIdAllocator();
	GuidAllocator<ST_MeshSourceDesc>& GetMeshAssetIdAllocator();	// ��ȡ������ԴID������

	GuidAllocator<ST_MaterialSourceDesc>& GetMaterialAssetIdAllocator();	// ��ȡ������ԴID������

	void AddInstanceIdToMap(uint32_t instanceId, GObjectID goId);	// ��ʵ��ID��ӵ�ӳ����
	GObjectID GetGObjectIDByMeshID(uint32_t meshId) const;	// ��������ID��ȡ��Ϸ����ID
	void DeleteEntityByGObjectID(GObjectID goId);	// ������Ϸ����IDɾ��ʵ��

	void ClearForLevelReloading();	// �������ڹؿ����¼��ص���Դ

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
	void UpdateVisibleObjectsDirectionalLight(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);	// ����ֱ���ɼ�����
	void UpdateVisibleObjectsPointLight(std::shared_ptr<RenderResource> render_resource);	// ���µ��Դ�ɼ�����
	void UpdateVisibleObjectsMainCamera(std::shared_ptr<RenderResource> render_resource, std::shared_ptr<RenderCamera> camera);	// ������������ɼ�����
	void UpdateVisibleObjectsAxis(std::shared_ptr<RenderResource> render_resource);	// ����������ɼ�����
	void UpdateVisibleObjectsParticle(std::shared_ptr<RenderResource> render_resource);	// �������ӿɼ�����

private:

	GuidAllocator<ST_GameObjectPartId> m_instanceIdAllocator;
	GuidAllocator<ST_MeshSourceDesc> m_meshAssetIdAllocator;
	GuidAllocator<ST_MaterialSourceDesc> m_materialAssetIdAllocator;

	std::unordered_map<uint32_t, GObjectID> m_meshObjectIdMap;	// �������IDӳ��

};

NAMESPACE_XYH_END