#pragma once
#include <Common.h>
#include <memory>
#include <unordered_map>
#include "RenderEntity.h"
#include "RenderSwapContext.h"
#include "RenderType.h"
#include "../../Core/Math/AxisAligned.h"

NAMESPACE_XYH_BEGIN

class RHI;
class RenderScene;
class RenderCamera;

class RenderResourceBase
{
public:
	virtual ~RenderResourceBase() {};

	virtual void Clear() = 0;	// ������Ⱦ��Դ

	virtual void UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc) = 0;	// �ϴ�ȫ����Ⱦ��Դ

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity renderEntity, ST_RenderMeshData mesh_data, ST_RenderMaterialData material_data) = 0;	// �ϴ���Ϸ������Ⱦ��Դ

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMeshData mesh_data) = 0;	// �ϴ���Ϸ������Ⱦ��Դ�����������ݣ�

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMaterialData material_data) = 0; // �ϴ���Ϸ������Ⱦ��Դ�����������ݣ�

	virtual void UpdatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) = 0; // ����ÿ֡������

    // TODO: data caching
    std::shared_ptr<TextureData> LoadTextureHDR(std::string file, int desiredChannels = 4);
    std::shared_ptr<TextureData> LoadTexture(std::string file, bool isSrgb = false);
    ST_RenderMeshData LoadMeshData(const ST_MeshSourceDesc& source, AxisAlignedBox& boundingBox);
    ST_RenderMaterialData LoadMaterialData(const ST_MaterialSourceDesc& source);
    AxisAlignedBox GetCachedBoudingBox(const ST_MeshSourceDesc& source) const;

private:
    ST_StaticMeshData LoadStaticMesh(std::string meshFile, AxisAlignedBox& boundingBox);

private:
	std::unordered_map<ST_MeshSourceDesc, AxisAlignedBox> m_boundingBoxCacheMap;	// ����߽�л���ӳ��
};

NAMESPACE_XYH_END