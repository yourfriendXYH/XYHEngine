#include "RenderResource.h"
#include "RenderScene.h"
#include "RenderMesh.h"

NAMESPACE_XYH_BEGIN

void RenderResource::Clear()
{
}

void RenderResource::UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc)
{
	CreateAndMapStorageBuffer(pRHI);

	// sky box irradiance 天空盒辐照度
	SkyBoxIrradianceMap skyBoxIrradianceMap = levelResourceDesc.m_iblResourceDesc.m_skyboxIrradianceMap;	// 天空盒资源路径
	// 加载纹理数据
	// sky box irradiance
	std::shared_ptr<TextureData> pIrradiancePosXMap = LoadTextureHDR(skyBoxIrradianceMap.m_positiveXMap);
	std::shared_ptr<TextureData> pIrradianceNegXMap = LoadTextureHDR(skyBoxIrradianceMap.m_negativeXMap);
	std::shared_ptr<TextureData> pIrradiancePosYMap = LoadTextureHDR(skyBoxIrradianceMap.m_positiveYMap);
	std::shared_ptr<TextureData> pIrradianceNegYMap = LoadTextureHDR(skyBoxIrradianceMap.m_negativeYMap);
	std::shared_ptr<TextureData> pIrradiancePosZMap = LoadTextureHDR(skyBoxIrradianceMap.m_positiveZMap);
	std::shared_ptr<TextureData> pIrradianceNegZMap = LoadTextureHDR(skyBoxIrradianceMap.m_negativeZMap);

	// sky box specular
	SkyBoxSpecularMap skyBoxSpecularMap = levelResourceDesc.m_iblResourceDesc.m_skyboxSpecularMap;
	std::shared_ptr<TextureData> pSpecularPosXMap = LoadTextureHDR(skyBoxSpecularMap.m_positiveXMap);
	std::shared_ptr<TextureData> pSpecularNegXMap = LoadTextureHDR(skyBoxSpecularMap.m_negativeXMap);
	std::shared_ptr<TextureData> pSpecularPosYMap = LoadTextureHDR(skyBoxSpecularMap.m_positiveYMap);
	std::shared_ptr<TextureData> pSpecularNegYMap = LoadTextureHDR(skyBoxSpecularMap.m_negativeYMap);
	std::shared_ptr<TextureData> pSpecularPosZMap = LoadTextureHDR(skyBoxSpecularMap.m_positiveZMap);
	std::shared_ptr<TextureData> pSpecularNegZMap = LoadTextureHDR(skyBoxSpecularMap.m_negativeZMap);

	// BRDF纹理 Bidirectional Reflectance Distribution Function Texture
	// 存储材质表面反射属性的数据纹理
	std::shared_ptr<TextureData> pBRDFMap = LoadTextureHDR(levelResourceDesc.m_iblResourceDesc.m_brdfMap);

	// IBL纹理 Image-Based Lighting纹理
	// 存储环境光照信息的特殊纹理
	CreateIBLSamplers(pRHI);

	std::array<std::shared_ptr<TextureData>, 6> irradianceMaps = {
		pIrradiancePosXMap,
		pIrradianceNegXMap,
		pIrradiancePosYMap,
		pIrradianceNegYMap,
		pIrradiancePosZMap,
		pIrradianceNegZMap
	};

	std::array<std::shared_ptr<TextureData>, 6> specularMaps = {
		pSpecularPosXMap,
		pSpecularNegXMap,
		pSpecularPosYMap,
		pSpecularNegYMap,
		pSpecularPosZMap,
		pSpecularNegZMap
	};

	CreateIBLTextures(pRHI, irradianceMaps, specularMaps);
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

void RenderResource::CreateAndMapStorageBuffer(std::shared_ptr<RHI> pRHI)
{
	ST_StorageBuffer& storageBuffer = m_globalRenderResource.m_storageBuffer;

	uint32_t framesInFlight = RHI::s_maxFramesInFlight;	// 帧缓冲数量

	ST_RHIPhysicalDeviceProperties properties;
	pRHI->GetPhysicalDeviceProperties(&properties);

	storageBuffer.m_minUniformBufferOffsetAlignment = static_cast<uint32_t>(properties.m_limits.m_minUniformBufferOffsetAlignment);
	storageBuffer.m_minStorageBufferOffsetAlignment = static_cast<uint32_t>(properties.m_limits.m_minStorageBufferOffsetAlignment);
	storageBuffer.m_maxStorageBufferRange = properties.m_limits.m_maxStorageBufferRange;
	storageBuffer.m_nonCoherentAtomSize = properties.m_limits.m_nonCoherentAtomSize;

	// 环形缓冲区
	uint32_t globalStorageBufferSize = 1024 * 1024 * 128;	// 3帧共用的缓冲区， 134217728
	pRHI->CreateBuffer(
		globalStorageBufferSize,
		RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		storageBuffer.m_pGlobalUploadRingbuffer,	// 缓冲区骨架
		storageBuffer.m_pGlobalUploadRingbufferMemory	// 缓冲区内存
	);

	storageBuffer.m_globalUploadRingbuffersBegin.resize(framesInFlight);
	storageBuffer.m_globalUploadRingbuffersEnd.resize(framesInFlight);
	storageBuffer.m_globalUploadRingbuffersSize.resize(framesInFlight);
	// 总大小： 134217728
	// frame 1
	// begin: 0				size: 44739242
	// frame 2
	// begin: 44739242		size: 44739242
	// frame 3
	// begin: 89478485		size: 44739242
	for (size_t i = 0; i < framesInFlight; ++i)
	{
		storageBuffer.m_globalUploadRingbuffersBegin[i] = (globalStorageBufferSize * i) / framesInFlight;
		storageBuffer.m_globalUploadRingbuffersSize[i] = (globalStorageBufferSize * (i + 1)) / framesInFlight - (globalStorageBufferSize * i) / framesInFlight;
	}

	// Axis
	pRHI->CreateBuffer(
		sizeof(ST_AxisStorageBufferObject),
		RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		storageBuffer.m_pAxisInefficientStorageBuffer,
		storageBuffer.m_pAxisInefficientStorageBufferMemory
	);

	// Null Descriptor
	pRHI->CreateBuffer(
		64,
		RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		0,
		storageBuffer.m_pGlobalNullDescriptorStorageBuffer,
		storageBuffer.m_pGlobalNullDescriptorStorageBufferMemory
	);

	// TODO: 程序终止时解除映射
	pRHI->MapMemory(
		storageBuffer.m_pGlobalUploadRingbufferMemory,
		0,
		RHI_WHOLE_SIZE,
		0,
		&storageBuffer.m_pGlobalUploadRingbufferMemoryPointer
	);

	pRHI->MapMemory(
		storageBuffer.m_pAxisInefficientStorageBufferMemory,
		0,
		RHI_WHOLE_SIZE,
		0,
		&storageBuffer.m_pAxisInefficientStorageBufferMemoryPointer
	);

	static_assert(64 >= sizeof(ST_MeshVertex::ST_VulkanMeshVertexJointBinding), "");
}

void RenderResource::CreateIBLSamplers(std::shared_ptr<RHI> pRHI)
{
	ST_RHIPhysicalDeviceProperties properties;
	pRHI->GetPhysicalDeviceProperties(&properties);

	// 采样器创建
	ST_RHISamplerCreateInfo samplerInfo{};
	samplerInfo.m_sType = RHI_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.m_magFilter = RHI_FILTER_LINEAR;
	samplerInfo.m_minFilter = RHI_FILTER_LINEAR;
	samplerInfo.m_addressModeU = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.m_addressModeV = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.m_addressModeW = RHI_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.m_anisotropyEnable = RHI_TRUE;                                                // close:false
	samplerInfo.m_maxAnisotropy = properties.m_limits.m_maxSamplerAnisotropy; // close :1.0f
	samplerInfo.m_borderColor = RHI_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.m_unnormalizedCoordinates = RHI_FALSE;
	samplerInfo.m_compareEnable = RHI_FALSE;
	samplerInfo.m_compareOp = RHI_COMPARE_OP_ALWAYS;
	samplerInfo.m_mipmapMode = RHI_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.m_maxLod = 0.0f;

	if (m_globalRenderResource.m_iblResource.m_pBrdfLUTTextureSampler != RHI_NULL_HANDLE)
	{
		pRHI->DestroySampler(m_globalRenderResource.m_iblResource.m_pBrdfLUTTextureSampler);
	}
	if (pRHI->CreateSampler(&samplerInfo, m_globalRenderResource.m_iblResource.m_pBrdfLUTTextureSampler) != RHI_SUCCESS)
	{
		throw std::runtime_error("vk create sampler");
	}

	samplerInfo.m_minLod = 0.0f;
	samplerInfo.m_maxLod = 8.0f; // TODO: irradiance_texture_miplevels
	samplerInfo.m_mipLodBias = 0.0f;

	if (m_globalRenderResource.m_iblResource.m_pIrradianceTextureSampler != RHI_NULL_HANDLE)
	{
		pRHI->DestroySampler(m_globalRenderResource.m_iblResource.m_pIrradianceTextureSampler);
	}
	if (pRHI->CreateSampler(&samplerInfo, m_globalRenderResource.m_iblResource.m_pIrradianceTextureSampler) != RHI_SUCCESS)
	{
		throw std::runtime_error("vk create sampler");
	}

	if (m_globalRenderResource.m_iblResource.m_pSpecularTextureSampler != RHI_NULL_HANDLE)
	{
		pRHI->DestroySampler(m_globalRenderResource.m_iblResource.m_pSpecularTextureSampler);
	}
	if (pRHI->CreateSampler(&samplerInfo, m_globalRenderResource.m_iblResource.m_pSpecularTextureSampler) != RHI_SUCCESS)
	{
		throw std::runtime_error("vk create sampler");
	}
}

void RenderResource::CreateIBLTextures(std::shared_ptr<RHI> pRHI, std::array<std::shared_ptr<TextureData>, 6> irradianceMaps, std::array<std::shared_ptr<TextureData>, 6> specularMaps)
{

}


NAMESPACE_XYH_END

