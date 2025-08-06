#include "RenderResource.h"

NAMESPACE_XYH_BEGIN

void RenderResource::Clear()
{
}

void RenderResource::UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc)
{
}

void RenderResource::UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity renderEntity, ST_RenderMeshData mesh_data, ST_RenderMaterialData material_data)
{
}

void RenderResource::UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMeshData mesh_data)
{
}

void RenderResource::UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMaterialData material_data)
{
}

void RenderResource::UpdatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera)
{
}


NAMESPACE_XYH_END

