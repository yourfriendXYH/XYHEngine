#pragma once
#include <Common.h>
#include "RenderResourceBase.h"

NAMESPACE_XYH_BEGIN

class RenderResource : public RenderResourceBase
{
public:
	void Clear() override final;	// 清理渲染资源

	virtual void UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc) override final;	// 上传全局渲染资源

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity renderEntity, ST_RenderMeshData mesh_data, ST_RenderMaterialData material_data) override final;	// 上传游戏对象渲染资源

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMeshData mesh_data) override final;	// 上传游戏对象渲染资源（仅网格数据）

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMaterialData material_data) override final; // 上传游戏对象渲染资源（仅材质数据）

	virtual void UpdatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) override final; // 更新每帧缓冲区

private:

};

NAMESPACE_XYH_END