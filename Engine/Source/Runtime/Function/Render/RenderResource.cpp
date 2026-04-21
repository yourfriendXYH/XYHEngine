#include "RenderResource.h"
#include "RenderScene.h"
#include "RenderMesh.h"
#include <Runtime/Function/Render/Interface/Vulkan/VulkanRHI.h>

NAMESPACE_XYH_BEGIN

void RenderResource::Clear()
{
}

void RenderResource::UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc)
{
	// 环形缓冲区 和 轴线 的内存分配
	CreateAndMapStorageBuffer(pRHI);

	// test
	return;

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

	// 天空盒相关数据 内存分配
	CreateIBLTextures(pRHI, irradianceMaps, specularMaps);

	// create brdf lut texture
	pRHI->CreateGlobalImage(
		m_globalRenderResource.m_iblResource.m_pBrdfLUTTextureImage,
		m_globalRenderResource.m_iblResource.m_pBrdfLUTTextureImageView,
		m_globalRenderResource.m_iblResource.m_brdfLUTTextureImageAllocation,
		pBRDFMap->m_width,
		pBRDFMap->m_height,
		pBRDFMap->m_pixels,
		pBRDFMap->m_format
	);

	// color grading
	std::shared_ptr<TextureData> pColorGradingMap = LoadTexture(levelResourceDesc.m_colorGradingResourceDesc.m_colorGradingMap);

	// create color grading texture
	pRHI->CreateGlobalImage(
		m_globalRenderResource.m_colorGradingResource.m_pColorGradingLUTTextureImage,
		m_globalRenderResource.m_colorGradingResource.m_pColorGradingLUTTextureImageView,
		m_globalRenderResource.m_colorGradingResource.m_pColorGradingLUTTextureImageAllocation,
		pColorGradingMap->m_width,
		pColorGradingMap->m_height,
		pColorGradingMap->m_pixels,
		pColorGradingMap->m_format
	);
}

void RenderResource::UploadGameObjectRenderResource(std::shared_ptr<RHI> pRHI, RenderEntity renderEntity, ST_RenderMeshData meshData, ST_RenderMaterialData materialData)
{
	GetOrCreateVulkanMesh(pRHI, renderEntity, meshData);
	GetOrCreateVulkanMaterial(pRHI, renderEntity, materialData);
}

void RenderResource::UploadGameObjectRenderResource(std::shared_ptr<RHI> pRHI, RenderEntity renderEntity, ST_RenderMeshData meshData)
{
	GetOrCreateVulkanMesh(pRHI, renderEntity, meshData);
}

void RenderResource::UploadGameObjectRenderResource(std::shared_ptr<RHI> pRHI, RenderEntity renderEntity, ST_RenderMaterialData materialData)
{
	GetOrCreateVulkanMaterial(pRHI, renderEntity, materialData);
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
	// 假设所有纹理具有相同的宽度、高度和格式
	uint32_t irradianceCubemapMiplevels = static_cast<uint32_t>(std::floor(log2(std::max(irradianceMaps[0]->m_width, irradianceMaps[0]->m_height)))) + 1u;
	pRHI->CreateCubeMap(
		m_globalRenderResource.m_iblResource.m_pIrradianceTextureImage,
		m_globalRenderResource.m_iblResource.m_pIrradianceTextureImageView,
		m_globalRenderResource.m_iblResource.m_irradianceTextureImageAllocation,
		irradianceMaps[0]->m_width,
		irradianceMaps[0]->m_height,
		{ irradianceMaps[0]->m_pixels, irradianceMaps[1]->m_pixels, irradianceMaps[2]->m_pixels, irradianceMaps[3]->m_pixels, irradianceMaps[4]->m_pixels, irradianceMaps[5]->m_pixels },
		irradianceMaps[0]->m_format,
		irradianceCubemapMiplevels
	);

	uint32_t specularCubemapMiplevels = static_cast<uint32_t>(std::floor(log2(std::max(specularMaps[0]->m_width, specularMaps[0]->m_height)))) + 1u;
	pRHI->CreateCubeMap(
		m_globalRenderResource.m_iblResource.m_pSpecularTextureImage,
		m_globalRenderResource.m_iblResource.m_pSpecularTextureImageView,
		m_globalRenderResource.m_iblResource.m_specularTextureImageAllocation,
		specularMaps[0]->m_width,
		specularMaps[0]->m_height,
		{ specularMaps[0]->m_pixels, specularMaps[1]->m_pixels, specularMaps[2]->m_pixels, specularMaps[3]->m_pixels, specularMaps[4]->m_pixels, specularMaps[5]->m_pixels },
		specularMaps[0]->m_format,
		specularCubemapMiplevels
	);
}

ST_VulkanMesh& RenderResource::GetOrCreateVulkanMesh(std::shared_ptr<RHI> pRHI, RenderEntity entity, ST_RenderMeshData meshData)
{
	size_t assetId = entity.m_meshAssetId;
	auto it = m_vulkanMeshes.find(assetId);
	if (it != m_vulkanMeshes.end())
	{
		return it->second;
	}
	else
	{
		ST_VulkanMesh tempVkMesh;
		auto res = m_vulkanMeshes.insert(std::make_pair(assetId, std::move(tempVkMesh)));
		assert(res.second);

		// 索引
		uint32_t indexBufferSize = static_cast<uint32_t>(meshData.m_staticMeshData.m_indexBuffer->m_size);
		void* pIndexBufferData = meshData.m_staticMeshData.m_indexBuffer->m_data;
		// 顶点
		uint32_t vertexBufferSize = static_cast<uint32_t>(meshData.m_staticMeshData.m_vertexBuffer->m_size);
		ST_MeshVertexDataDefinition* pVertexBufferData = reinterpret_cast<ST_MeshVertexDataDefinition*>(meshData.m_staticMeshData.m_vertexBuffer->m_data);

		ST_VulkanMesh& nowMesh = res.first->second;

		if (meshData.m_skeletonBindingBuffer)
		{
			uint32_t jointBindingBufferSize = static_cast<uint32_t>(meshData.m_skeletonBindingBuffer->m_size);
			ST_MeshVertexBindingDataDefinition* pJointBindingBufferData = reinterpret_cast<ST_MeshVertexBindingDataDefinition*>(meshData.m_skeletonBindingBuffer->m_data);
			// 更新网格数据
			UpdateMeshData(
				pRHI,
				true,
				indexBufferSize,
				pIndexBufferData,
				vertexBufferSize,
				pVertexBufferData,
				jointBindingBufferSize,
				pJointBindingBufferData,
				nowMesh
			);
		}
		else
		{
			// 更新网格数据
			UpdateMeshData(
				pRHI,
				false,
				indexBufferSize,
				pIndexBufferData,
				vertexBufferSize,
				pVertexBufferData,
				0,
				nullptr,
				nowMesh
			);
		}

		return nowMesh;
	}
}

ST_VulkanPBRMaterial& RenderResource::GetOrCreateVulkanMaterial(std::shared_ptr<RHI> pRHI, RenderEntity entity, ST_RenderMaterialData materialData)
{
	// TODO: 在此处插入 return 语句
	ST_VulkanPBRMaterial test;
	return test;
}

void RenderResource::UpdateMeshData(
	std::shared_ptr<RHI> pRHI,
	bool enableVertexBlending,
	uint32_t indexBufferSize,
	void* pIndexBufferData,
	uint32_t vertexBufferSize,
	ST_MeshVertexDataDefinition const* pVertexBufferData,
	uint32_t jointBindingBufferSize,
	ST_MeshVertexBindingDataDefinition const* pJointBindingBufferData,
	ST_VulkanMesh& outNowMesh)
{
	outNowMesh.m_enableVertexBlending = enableVertexBlending;
	// 顶点
	assert(0 == (vertexBufferSize % sizeof(ST_MeshVertexDataDefinition)));
	outNowMesh.m_meshVertexCount = vertexBufferSize / sizeof(ST_MeshVertexDataDefinition);
	UpdateVertexBuffer(
		pRHI,
		enableVertexBlending,
		vertexBufferSize,
		pVertexBufferData,
		jointBindingBufferSize,
		pJointBindingBufferData,
		indexBufferSize,
		reinterpret_cast<uint16_t*>(pIndexBufferData),
		outNowMesh
	);
	// 索引
	assert(0 == (indexBufferSize % sizeof(uint16_t)));
	outNowMesh.m_meshIndexCount = indexBufferSize / sizeof(uint16_t);
	UpdateIndexBuffer(
		pRHI,
		indexBufferSize,
		pIndexBufferData,
		outNowMesh
	);
}

void RenderResource::UpdateVertexBuffer(
	std::shared_ptr<RHI> pRHI,
	bool enableVertexBlending,
	uint32_t vertexBufferSize,
	ST_MeshVertexDataDefinition const* pVertexBufferData,
	uint32_t jointBindingBufferSize,
	ST_MeshVertexBindingDataDefinition const* pJointBindingBufferData,
	uint32_t indexBufferSize,
	uint16_t* pIndexBufferData,
	ST_VulkanMesh& outNowMesh)
{
	if (enableVertexBlending)
	{
		assert(0 == (vertexBufferSize % sizeof(ST_MeshVertexDataDefinition)));
		uint32_t vertexCount = vertexBufferSize / sizeof(ST_MeshVertexDataDefinition);
		assert(0 == (indexBufferSize % sizeof(uint16_t)));
		uint32_t indexCount = indexBufferSize / sizeof(uint16_t);

		// size
		RHIDeviceSize vertexPositionBufferSize = sizeof(ST_MeshVertex::ST_VulkanMeshVertexPostition) * vertexCount;
		RHIDeviceSize vertexVaryingEnableBlendingBufferSize = sizeof(ST_MeshVertex::ST_VulkanMeshVertexVaryingEnableBlending) * vertexCount;
		RHIDeviceSize vertexVaryingBufferSize = sizeof(ST_MeshVertex::ST_VulkanMeshVertexVarying) * vertexCount;
		RHIDeviceSize vertexJointBindingBufferSize = sizeof(ST_MeshVertex::ST_VulkanMeshVertexJointBinding) * indexCount;

		// offset
		RHIDeviceSize vertexPositionBufferOffset = 0u;
		RHIDeviceSize vertexVaryingEnableBlendingBufferOffset = vertexPositionBufferOffset + vertexPositionBufferSize;
		RHIDeviceSize vertexVaryingBufferOffset = vertexVaryingEnableBlendingBufferOffset + vertexVaryingEnableBlendingBufferSize;
		RHIDeviceSize vertexJointBindingBufferOffset = vertexVaryingBufferOffset + vertexVaryingBufferSize;

		// 临时顶点数据缓冲区创建（用于数据拷贝）
		RHIDeviceSize inefficientStagingBufferSize = vertexPositionBufferSize + vertexVaryingEnableBlendingBufferSize + vertexVaryingBufferSize + vertexJointBindingBufferSize;
		RHIBuffer* pInefficientStagingBuffer = RHI_NULL_HANDLE;
		RHIDeviceMemory* pInefficientStagingBufferMemory = RHI_NULL_HANDLE;
		pRHI->CreateBuffer(
			inefficientStagingBufferSize,
			RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			pInefficientStagingBuffer,
			pInefficientStagingBufferMemory
		);

		// 给内存赋值
		void* pInefficientStagingBufferData;
		pRHI->MapMemory(
			pInefficientStagingBufferMemory,
			0,
			RHI_WHOLE_SIZE,
			0,
			&pInefficientStagingBufferData
		);

		// 指向Position数据块的指针
		ST_MeshVertex::ST_VulkanMeshVertexPostition* pMeshVertexPositions = 
			reinterpret_cast<ST_MeshVertex::ST_VulkanMeshVertexPostition*>(reinterpret_cast<uintptr_t>(pInefficientStagingBufferData) + vertexPositionBufferOffset);
		// normal,tangent ptr
		ST_MeshVertex::ST_VulkanMeshVertexVaryingEnableBlending* pMeshVertexBlendingVaryings =
			reinterpret_cast<ST_MeshVertex::ST_VulkanMeshVertexVaryingEnableBlending*>(reinterpret_cast<uintptr_t>(pInefficientStagingBufferData) + vertexVaryingEnableBlendingBufferOffset);
		// texcoord ptr
		ST_MeshVertex::ST_VulkanMeshVertexVarying* pMeshVertexVaryings =
			reinterpret_cast<ST_MeshVertex::ST_VulkanMeshVertexVarying*>(reinterpret_cast<uintptr_t>(pInefficientStagingBufferData) + vertexVaryingBufferOffset);
		// 
		ST_MeshVertex::ST_VulkanMeshVertexJointBinding* pMeshVertexJointBinding =
			reinterpret_cast<ST_MeshVertex::ST_VulkanMeshVertexJointBinding*>(reinterpret_cast<uintptr_t>(pInefficientStagingBufferData) + vertexJointBindingBufferOffset);

		// 
		for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			Vector3 normal = Vector3(pVertexBufferData[vertexIndex].m_nx, pVertexBufferData[vertexIndex].m_ny, pVertexBufferData[vertexIndex].m_nz);
			Vector3 tangent = Vector3(pVertexBufferData[vertexIndex].m_tx, pVertexBufferData[vertexIndex].m_ty, pVertexBufferData[vertexIndex].m_tz);

			pMeshVertexPositions[vertexIndex].m_position = Vector3(pVertexBufferData[vertexIndex].m_x, pVertexBufferData[vertexIndex].m_y, pVertexBufferData[vertexIndex].m_z);
			pMeshVertexBlendingVaryings[vertexIndex].m_normal = normal;
			pMeshVertexBlendingVaryings[vertexIndex].m_tangent = tangent;
			pMeshVertexVaryings[vertexIndex].m_texcoord = Vector2(pVertexBufferData[vertexIndex].m_u, pVertexBufferData[vertexIndex].m_v);
		}

		// 
		for (uint32_t i = 0; i < indexCount; ++i)
		{
			uint32_t vertexBufferIndex = pIndexBufferData[i];

			pMeshVertexJointBinding[i].m_indices[0] = pJointBindingBufferData[vertexBufferIndex].m_index0;
			pMeshVertexJointBinding[i].m_indices[1] = pJointBindingBufferData[vertexBufferIndex].m_index1;
			pMeshVertexJointBinding[i].m_indices[2] = pJointBindingBufferData[vertexBufferIndex].m_index2;
			pMeshVertexJointBinding[i].m_indices[3] = pJointBindingBufferData[vertexBufferIndex].m_index3;

			float invTotalWeight = pJointBindingBufferData[vertexBufferIndex].m_weight0 + pJointBindingBufferData[vertexBufferIndex].m_weight1 +
								   pJointBindingBufferData[vertexBufferIndex].m_weight2 + pJointBindingBufferData[vertexBufferIndex].m_weight3;
			// 加权平均
			invTotalWeight = (invTotalWeight != 0.0) ? 1 / invTotalWeight : 1.0;
			pMeshVertexJointBinding[i].m_weights = Vector4(
				pJointBindingBufferData[vertexBufferIndex].m_weight0 * invTotalWeight,
				pJointBindingBufferData[vertexBufferIndex].m_weight1 * invTotalWeight,
				pJointBindingBufferData[vertexBufferIndex].m_weight2 * invTotalWeight,
				pJointBindingBufferData[vertexBufferIndex].m_weight3 * invTotalWeight
			);
		}

		pRHI->UnmapMemory(pInefficientStagingBufferMemory);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		ST_RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.m_usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

		bufferInfo.m_size = vertexPositionBufferSize;
		pRHI->CreateBufferVMA(
			static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
			&bufferInfo, 
			&allocInfo, 
			outNowMesh.m_meshVertexPositionBuffer, 
			&outNowMesh.m_meshVertexPositionBufferAllocation, 
			nullptr
		);
		bufferInfo.m_size = vertexVaryingEnableBlendingBufferSize;
		pRHI->CreateBufferVMA(
			static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
			&bufferInfo,
			&allocInfo,
			outNowMesh.m_meshVertexVaryingEnableBlendingBuffer,
			&outNowMesh.m_meshVertexVaryingEnableBlendingBufferAllocation,
			nullptr
		);
		bufferInfo.m_size = vertexVaryingBufferSize;
		pRHI->CreateBufferVMA(
			static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
			&bufferInfo,
			&allocInfo,
			outNowMesh.m_meshVertexVaryingBuffer,
			&outNowMesh.m_meshVertexVaryingBufferAllocation,
			nullptr
		);
		bufferInfo.m_usage = RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferInfo.m_size = vertexJointBindingBufferSize;
		pRHI->CreateBufferVMA(
			static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
			&bufferInfo,
			&allocInfo,
			outNowMesh.m_meshVertexJointBindingBuffer,
			&outNowMesh.m_meshVertexJointBindingBufferAllocation,
			nullptr
		);

		// 拷贝
		pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshVertexPositionBuffer, vertexPositionBufferOffset, 0, vertexPositionBufferSize);
		pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshVertexPositionBuffer, vertexVaryingEnableBlendingBufferOffset, 0, vertexVaryingEnableBlendingBufferSize);
		pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshVertexPositionBuffer, vertexVaryingBufferOffset, 0, vertexVaryingBufferSize);
		pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshVertexPositionBuffer, vertexJointBindingBufferOffset, 0, vertexJointBindingBufferSize);

		// 销毁用于拷贝的临时缓冲区
		pRHI->DestroyBuffer(pInefficientStagingBuffer);
		pRHI->FreeMemory(pInefficientStagingBufferMemory);

		// 更新顶点混合的描述符集
		ST_RHIDescriptorSetAllocateInfo meshVertexBlendingPerMeshDescriptorSetAllocInfo;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_pNext = nullptr;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_pDescriptorPool = static_cast<VulkanRHI*>(pRHI.get())->m_pRHIDescriptorPool;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_descriptorSetCount = 1;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_pSetLayouts = m_pMeshDescriptorSetLayout;
		if (RHI_SUCCESS != pRHI->AllocateDescriptorSets(&meshVertexBlendingPerMeshDescriptorSetAllocInfo, outNowMesh.m_meshVertexBlendingDescriptorSet))
		{
			throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
		}
		ST_RHIDescriptorBufferInfo meshVertexJointBindingStorageBufferInfo = {};
		meshVertexJointBindingStorageBufferInfo.m_offset = 0;
		meshVertexJointBindingStorageBufferInfo.m_range = vertexJointBindingBufferSize;
		meshVertexJointBindingStorageBufferInfo.m_pBuffer = outNowMesh.m_meshVertexJointBindingBuffer;
		assert(meshVertexJointBindingStorageBufferInfo.m_range < m_globalRenderResource.m_storageBuffer.m_maxStorageBufferRange);

		RHIDescriptorSet* pDescriptorSetToWrite = outNowMesh.m_meshVertexBlendingDescriptorSet;
		ST_RHIWriteDescriptorSet descriptorWrites[1];
		ST_RHIWriteDescriptorSet& meshVertexBlendingVertexJointBindingStorageBufferWriteInfo = descriptorWrites[0];
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_pNext = nullptr;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_pDstSet = pDescriptorSetToWrite;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_dstBinding = 0;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_dstArrayElement = 0;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_descriptorCount = 1;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_pBufferInfo = &meshVertexJointBindingStorageBufferInfo;
		// 更新
		pRHI->UpdateDescriptorSets((sizeof(descriptorWrites) / sizeof(descriptorWrites[0])), descriptorWrites, 0, nullptr);
	}
	else // 只更新顶点缓冲区数据
	{
		assert(0 == (vertexBufferSize % sizeof(ST_MeshVertexDataDefinition)));
		uint32_t vertexCount = vertexBufferSize / sizeof(ST_MeshVertexDataDefinition);

		// size
		RHIDeviceSize vertexPositionBufferSize = sizeof(ST_MeshVertex::ST_VulkanMeshVertexPostition) * vertexCount;
		RHIDeviceSize vertexVaryingEnableBlendingBufferSize = sizeof(ST_MeshVertex::ST_VulkanMeshVertexVaryingEnableBlending) * vertexCount;
		RHIDeviceSize vertexVaryingBufferSize = sizeof(ST_MeshVertex::ST_VulkanMeshVertexVarying) * vertexCount;

		// offset
		RHIDeviceSize vertexPositionBufferOffset = 0u;
		RHIDeviceSize vertexVaryingEnableBlendingBufferOffset = vertexPositionBufferOffset + vertexPositionBufferSize;
		RHIDeviceSize vertexVaryingBufferOffset = vertexVaryingEnableBlendingBufferOffset + vertexVaryingEnableBlendingBufferSize;

		// 临时顶点数据缓冲区创建（用于数据拷贝）
		RHIDeviceSize inefficientStagingBufferSize = vertexPositionBufferSize + vertexVaryingEnableBlendingBufferSize + vertexVaryingBufferSize;
		RHIBuffer* pInefficientStagingBuffer = RHI_NULL_HANDLE;
		RHIDeviceMemory* pInefficientStagingBufferMemory = RHI_NULL_HANDLE;
		pRHI->CreateBuffer(
			inefficientStagingBufferSize,
			RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
			RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			pInefficientStagingBuffer,
			pInefficientStagingBufferMemory
		);

		// 给内存赋值
		void* pInefficientStagingBufferData;
		pRHI->MapMemory(
			pInefficientStagingBufferMemory,
			0,
			RHI_WHOLE_SIZE,
			0,
			&pInefficientStagingBufferData
		);

		// 指向Position数据块的指针
		ST_MeshVertex::ST_VulkanMeshVertexPostition* pMeshVertexPositions =
			reinterpret_cast<ST_MeshVertex::ST_VulkanMeshVertexPostition*>(reinterpret_cast<uintptr_t>(pInefficientStagingBufferData) + vertexPositionBufferOffset);
		// normal,tangent ptr
		ST_MeshVertex::ST_VulkanMeshVertexVaryingEnableBlending* pMeshVertexBlendingVaryings =
			reinterpret_cast<ST_MeshVertex::ST_VulkanMeshVertexVaryingEnableBlending*>(reinterpret_cast<uintptr_t>(pInefficientStagingBufferData) + vertexVaryingEnableBlendingBufferOffset);
		// texcoord ptr
		ST_MeshVertex::ST_VulkanMeshVertexVarying* pMeshVertexVaryings =
			reinterpret_cast<ST_MeshVertex::ST_VulkanMeshVertexVarying*>(reinterpret_cast<uintptr_t>(pInefficientStagingBufferData) + vertexVaryingBufferOffset);

		// 
		for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			Vector3 normal = Vector3(pVertexBufferData[vertexIndex].m_nx, pVertexBufferData[vertexIndex].m_ny, pVertexBufferData[vertexIndex].m_nz);
			Vector3 tangent = Vector3(pVertexBufferData[vertexIndex].m_tx, pVertexBufferData[vertexIndex].m_ty, pVertexBufferData[vertexIndex].m_tz);

			pMeshVertexPositions[vertexIndex].m_position = Vector3(pVertexBufferData[vertexIndex].m_x, pVertexBufferData[vertexIndex].m_y, pVertexBufferData[vertexIndex].m_z);
			pMeshVertexBlendingVaryings[vertexIndex].m_normal = normal;
			pMeshVertexBlendingVaryings[vertexIndex].m_tangent = tangent;
			pMeshVertexVaryings[vertexIndex].m_texcoord = Vector2(pVertexBufferData[vertexIndex].m_u, pVertexBufferData[vertexIndex].m_v);
		}

		pRHI->UnmapMemory(pInefficientStagingBufferMemory);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		ST_RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.m_usage = RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

		bufferInfo.m_size = vertexPositionBufferSize;
		pRHI->CreateBufferVMA(
			static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
			&bufferInfo,
			&allocInfo,
			outNowMesh.m_meshVertexPositionBuffer,
			&outNowMesh.m_meshVertexPositionBufferAllocation,
			nullptr
		);
		bufferInfo.m_size = vertexVaryingEnableBlendingBufferSize;
		pRHI->CreateBufferVMA(
			static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
			&bufferInfo,
			&allocInfo,
			outNowMesh.m_meshVertexVaryingEnableBlendingBuffer,
			&outNowMesh.m_meshVertexVaryingEnableBlendingBufferAllocation,
			nullptr
		);
		bufferInfo.m_size = vertexVaryingBufferSize;
		pRHI->CreateBufferVMA(
			static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
			&bufferInfo,
			&allocInfo,
			outNowMesh.m_meshVertexVaryingBuffer,
			&outNowMesh.m_meshVertexVaryingBufferAllocation,
			nullptr
		);

		// 拷贝
		pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshVertexPositionBuffer, vertexPositionBufferOffset, 0, vertexPositionBufferSize);
		pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshVertexPositionBuffer, vertexVaryingEnableBlendingBufferOffset, 0, vertexVaryingEnableBlendingBufferSize);
		pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshVertexPositionBuffer, vertexVaryingBufferOffset, 0, vertexVaryingBufferSize);

		// 销毁用于拷贝的临时缓冲区
		pRHI->DestroyBuffer(pInefficientStagingBuffer);
		pRHI->FreeMemory(pInefficientStagingBufferMemory);

		// 更新顶点混合的描述符集
		ST_RHIDescriptorSetAllocateInfo meshVertexBlendingPerMeshDescriptorSetAllocInfo;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_sType = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_pNext = nullptr;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_pDescriptorPool = static_cast<VulkanRHI*>(pRHI.get())->m_pRHIDescriptorPool;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_descriptorSetCount = 1;
		meshVertexBlendingPerMeshDescriptorSetAllocInfo.m_pSetLayouts = m_pMeshDescriptorSetLayout;
		if (RHI_SUCCESS != pRHI->AllocateDescriptorSets(&meshVertexBlendingPerMeshDescriptorSetAllocInfo, outNowMesh.m_meshVertexBlendingDescriptorSet))
		{
			throw std::runtime_error("allocate mesh vertex blending per mesh descriptor set");
		}
		ST_RHIDescriptorBufferInfo meshVertexJointBindingStorageBufferInfo = {};
		meshVertexJointBindingStorageBufferInfo.m_offset = 0;
		meshVertexJointBindingStorageBufferInfo.m_range = 1;
		meshVertexJointBindingStorageBufferInfo.m_pBuffer = m_globalRenderResource.m_storageBuffer.m_pGlobalNullDescriptorStorageBuffer;
		assert(meshVertexJointBindingStorageBufferInfo.m_range < m_globalRenderResource.m_storageBuffer.m_maxStorageBufferRange);

		RHIDescriptorSet* pDescriptorSetToWrite = outNowMesh.m_meshVertexBlendingDescriptorSet;
		ST_RHIWriteDescriptorSet descriptorWrites[1];
		ST_RHIWriteDescriptorSet& meshVertexBlendingVertexJointBindingStorageBufferWriteInfo = descriptorWrites[0];
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_sType = RHI_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_pNext = nullptr;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_pDstSet = pDescriptorSetToWrite;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_dstBinding = 0;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_dstArrayElement = 0;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_descriptorType = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_descriptorCount = 1;
		meshVertexBlendingVertexJointBindingStorageBufferWriteInfo.m_pBufferInfo = &meshVertexJointBindingStorageBufferInfo;
		// 更新
		pRHI->UpdateDescriptorSets((sizeof(descriptorWrites) / sizeof(descriptorWrites[0])), descriptorWrites, 0, nullptr);
	}
}

void RenderResource::UpdateIndexBuffer(std::shared_ptr<RHI> pRHI, uint32_t indexBufferSize, void* pIndexBufferData, ST_VulkanMesh& outNowMesh)
{
	RHIDeviceSize bufferSize = indexBufferSize;
	RHIBuffer* pInefficientStagingBuffer = RHI_NULL_HANDLE;
	RHIDeviceMemory* pInefficientStagingBufferMemory = RHI_NULL_HANDLE;
	pRHI->CreateBuffer(
		bufferSize,
		RHI_BUFFER_USAGE_TRANSFER_SRC_BIT,
		RHI_MEMORY_PROPERTY_HOST_VISIBLE_BIT | RHI_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		pInefficientStagingBuffer,
		pInefficientStagingBufferMemory
	);
	// 数据拷贝
	void* stagingBufferData;
	pRHI->MapMemory(pInefficientStagingBufferMemory, 0, bufferSize, 0, &stagingBufferData);
	memcpy(stagingBufferData, pIndexBufferData, (size_t)bufferSize);
	pRHI->UnmapMemory(pInefficientStagingBufferMemory);

	ST_RHIBufferCreateInfo bufferInfo = { RHI_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.m_size = bufferSize;
	bufferInfo.m_usage = RHI_BUFFER_USAGE_INDEX_BUFFER_BIT | RHI_BUFFER_USAGE_TRANSFER_DST_BIT;
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	pRHI->CreateBufferVMA(
		static_cast<VulkanRHI*>(pRHI.get())->m_assetsAllocator,
		&bufferInfo,
		&allocInfo,
		outNowMesh.m_meshIndexBuffer,
		&outNowMesh.m_meshIndexBufferAllocation,
		nullptr
	);

	// buffer拷贝
	pRHI->CopyBuffer(pInefficientStagingBuffer, outNowMesh.m_meshIndexBuffer, 0, 0, bufferSize);

	// 销毁临时缓冲区 和 释放内存
	pRHI->DestroyBuffer(pInefficientStagingBuffer);
	pRHI->FreeMemory(pInefficientStagingBufferMemory);
}


NAMESPACE_XYH_END

