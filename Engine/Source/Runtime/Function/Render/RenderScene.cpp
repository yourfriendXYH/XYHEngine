#include "RenderScene.h"
#include "RenderResource.h"
#include "RenderCamera.h"

NAMESPACE_XYH_BEGIN

void RenderScene::UpdateVisibleObjects(std::shared_ptr<RenderResource> renderResource, std::shared_ptr<RenderCamera> camera)
{
}

void RenderScene::SetVisibleNodesReference()
{
}

GuidAllocator<ST_MeshSourceDesc>& RenderScene::GetMeshAssetIdAllocator()
{
	return m_meshAssetIdAllocator;
}

GuidAllocator<ST_MaterialSourceDesc>& RenderScene::GetMaterialAssetIdAllocator()
{
	return m_materialAssetIdAllocator;
}

NAMESPACE_XYH_END


