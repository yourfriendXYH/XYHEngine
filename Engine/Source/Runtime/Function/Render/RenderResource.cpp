#include "RenderResource.h"
#include "RenderScene.h"

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

void RenderResource::UpdatePerFrameBuffer(std::shared_ptr<RenderScene> pRenderScene, std::shared_ptr<RenderCamera> pCamera)
{
	// 相机属性
	Matrix4x4 viewMatrix = pCamera->GetViewMatrix();
	Matrix4x4 projMatrix = pCamera->GetPersProjMatrix();
	Vector3 cameraPosition = pCamera->Position();
	Matrix4x4 projViewMatrix = projMatrix * viewMatrix;

	// 环境光
	Vector3 ambientLight = pRenderScene->m_ambientLight.m_irradiance;
	uint32_t pointLightNum = static_cast<uint32_t>(pRenderScene->m_pointLightList.m_lights.size());

	// 
	m_particleCollisionPerframeStorageBufferObject.m_viewMatrix = viewMatrix;
	m_particleCollisionPerframeStorageBufferObject.m_projViewMatrix = projViewMatrix;
	m_particleCollisionPerframeStorageBufferObject.m_projInvMatrix = projMatrix.inverse();

	m_meshPerframeStorageBufferObject.m_projViewMatrix = projViewMatrix;
	m_meshPerframeStorageBufferObject.m_cameraPosition = cameraPosition;
	m_meshPerframeStorageBufferObject.m_ambientLight = ambientLight;
	m_meshPerframeStorageBufferObject.m_pointLightNum = pointLightNum;

	// 目前并没有点光源
	m_meshPointLightShadowPerframeStorageBufferObject.m_pointLightNum = pointLightNum;
	for (uint32_t i = 0; i < pointLightNum; i++)
	{
		Vector3 pointLightPosition = pRenderScene->m_pointLightList.m_lights[i].m_position;	// 位置
		Vector3 pointLightIntensity = pRenderScene->m_pointLightList.m_lights[i].m_flux / (4.0f * Math_PI);	// 光强
		float radius = pRenderScene->m_pointLightList.m_lights[i].CalculateRadius();	// 点光源半径

		m_meshPerframeStorageBufferObject.m_scenePointLights[i].m_position = pointLightPosition;
		m_meshPerframeStorageBufferObject.m_scenePointLights[i].m_intensity = pointLightIntensity;
		m_meshPerframeStorageBufferObject.m_scenePointLights[i].m_radius = radius;

		m_meshPointLightShadowPerframeStorageBufferObject.m_pointLightsPositionAndRadius[i] = Vector4(pointLightPosition, radius);
	}
	// 直射光
	m_meshPerframeStorageBufferObject.m_sceneDirectionalLight.m_color = pRenderScene->m_directionalLight.m_color;
	m_meshPerframeStorageBufferObject.m_sceneDirectionalLight.m_direction = pRenderScene->m_directionalLight.m_direction;

	// Pick Pass
	m_meshInefficientPickPerframeStorageBufferObject.m_projViewMatrix = projViewMatrix;

	// 广告牌粒子
	m_particleBillboardPerframeStorageBufferObject.m_projViewMatrix = projViewMatrix;
	m_particleBillboardPerframeStorageBufferObject.m_rightDirection = pCamera->Right();
	m_particleBillboardPerframeStorageBufferObject.m_fowardDirection = pCamera->Forward();
	m_particleBillboardPerframeStorageBufferObject.m_upDirection = pCamera->Up();
}

void RenderResource::ResetRingBufferOffset(uint8_t currentFrameIndex)
{
	if (!m_globalRenderResource.m_storageBuffer.m_globalUploadRingbuffersEnd.empty() && !m_globalRenderResource.m_storageBuffer.m_globalUploadRingbuffersBegin.empty())
	{
		m_globalRenderResource.m_storageBuffer.m_globalUploadRingbuffersEnd[currentFrameIndex] =
			m_globalRenderResource.m_storageBuffer.m_globalUploadRingbuffersBegin[currentFrameIndex];
	}
}

ST_VulkanMesh& RenderResource::GetEntityMesh(RenderEntity entity)
{
	auto it = m_vulkanMeshes.find(entity.m_meshAssetId);
	if (it != m_vulkanMeshes.end())
	{
		return it->second;
	}
	else
	{
		throw std::runtime_error("failed to get entity mesh");
	}
}

ST_VulkanPBRMaterial& RenderResource::GetEntityMaterial(RenderEntity entity)
{
	auto it = m_vulkanPBRMaterials.find(entity.m_materialAssetId);
	if (it != m_vulkanPBRMaterials.end())
	{
		return it->second;
	}
	else
	{
		throw std::runtime_error("failed to get entity material");
	}
}


NAMESPACE_XYH_END

