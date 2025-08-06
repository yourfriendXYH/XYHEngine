#include "RenderResourceBase.h"

NAMESPACE_XYH_BEGIN

std::shared_ptr<TextureData> RenderResourceBase::LoadTextureHDR(std::string file, int desiredChannels)
{
	return std::shared_ptr<TextureData>();
}

std::shared_ptr<TextureData> RenderResourceBase::LoadTexture(std::string file, bool isSrgb)
{
	return std::shared_ptr<TextureData>();
}

ST_RenderMeshData RenderResourceBase::LoadMeshData(const ST_MeshSourceDesc& source, AxisAlignedBox& boundingBox)
{
	return ST_RenderMeshData();
}

ST_RenderMaterialData RenderResourceBase::LoadMaterialData(const ST_MaterialSourceDesc& source)
{
	return ST_RenderMaterialData();
}

AxisAlignedBox RenderResourceBase::GetCachedBoudingBox(const ST_MeshSourceDesc& source) const
{
	return AxisAlignedBox();
}

ST_StaticMeshData RenderResourceBase::LoadStaticMesh(std::string meshFile, AxisAlignedBox& boundingBox)
{
	return ST_StaticMeshData();
}

NAMESPACE_XYH_END
