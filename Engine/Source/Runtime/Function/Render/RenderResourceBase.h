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

	virtual void Clear() = 0;	// 清理渲染资源

	virtual void UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc) = 0;	// 上传全局渲染资源

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity renderEntity, ST_RenderMeshData mesh_data, ST_RenderMaterialData material_data) = 0;	// 上传游戏对象渲染资源

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMeshData mesh_data) = 0;	// 上传游戏对象渲染资源（仅网格数据）

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMaterialData material_data) = 0; // 上传游戏对象渲染资源（仅材质数据）

	virtual void UpdatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) = 0; // 更新每帧缓冲区

    // TODO: data caching
    std::shared_ptr<TextureData> LoadTextureHDR(std::string file, int desiredChannels = 4);
    std::shared_ptr<TextureData> LoadTexture(std::string file, bool isSrgb = false);
    ST_RenderMeshData LoadMeshData(const ST_MeshSourceDesc& source, AxisAlignedBox& boundingBox);
    ST_RenderMaterialData LoadMaterialData(const ST_MaterialSourceDesc& source);
    AxisAlignedBox GetCachedBoudingBox(const ST_MeshSourceDesc& source) const;

private:
    ST_StaticMeshData LoadStaticMesh(std::string meshFile, AxisAlignedBox& boundingBox);

private:
	std::unordered_map<ST_MeshSourceDesc, AxisAlignedBox> m_boundingBoxCacheMap;	// 网格边界盒缓存映射
};

NAMESPACE_XYH_END