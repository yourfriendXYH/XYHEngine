#include "RenderSystem.h"
#include "../GlobalContext.h"
#include "../../Resource/ConfigManager.h"
#include "Interface/RHI.h"
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"
#include "RenderCamera.h"
#include "RenderScene.h"
#include "RenderPipeline.h"

NAMESPACE_XYH_BEGIN

RenderSystem::~RenderSystem()
{
	Clear();
}

void RenderSystem::Initialize(ST_RenderSystemInitInfo initInfo)
{
	std::shared_ptr<ConfigManager> pConfigManager = g_runtimeGlobalContext.m_pConfigManager;

	ST_RHIInitInfo rhiInitInfo;
	rhiInitInfo.m_pWindowSystem = initInfo.m_pWindowSystem;
	m_pRHI = std::make_shared<VulkanRHI>();
	m_pRHI->Initialize(rhiInitInfo);

	// 全局渲染资源
	// GlobalRenderingRes globalRenderingRes;
	const std::string& globalRenderingResURL = pConfigManager->GetGlobalRenderingResURL();
	// asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

	// 上传ibl，颜色分级纹理
	ST_LevelResourceDesc levelResourceDesc;	// 关卡资源描述
	//level_resource_desc.m_ibl_resource_desc.m_skybox_irradiance_map = global_rendering_res.m_skybox_irradiance_map;
	//level_resource_desc.m_ibl_resource_desc.m_skybox_specular_map = global_rendering_res.m_skybox_specular_map;
	//level_resource_desc.m_ibl_resource_desc.m_brdf_map = global_rendering_res.m_brdf_map;
	//level_resource_desc.m_color_grading_resource_desc.m_color_grading_map = global_rendering_res.m_color_grading_map;

	m_pRenderResource = std::make_shared<RenderResource>();
	m_pRenderResource->UploadGlobalRenderResource(m_pRHI, levelResourceDesc);	// 上传全局渲染资源

	// 初始化相机参数
	m_pRenderCamera = std::make_shared<RenderCamera>();	// 创建渲染相机
	m_pRenderCamera->LookAt(Vector3(-5.0f, 0.0f, 3.0f), Vector3(-4.0f, 0.0f, 3.0f), Vector3(0.0f, 0.0f, 1.0f));	// 设置相机初始位置和方向
	m_pRenderCamera->m_zFar = 1000.0f;	// 设置远裁剪面
	m_pRenderCamera->m_zNear = 0.1f;		// 设置近裁剪面
	m_pRenderCamera->SetAspectRatio(1280.0f / 768.0f);	// 设置宽高比

	m_pRenderScene = std::make_shared<RenderScene>();	// 创建渲染场景
	m_pRenderScene->m_ambientLight = { Vector3(0.1f, 0.1f, 0.1f) };	// 设置环境光颜色
	m_pRenderScene->m_directionalLight.m_direction = Vector3(-1.0f, -1.0f, -1.0f).normalisedCopy();	// 设置直射光方向
	m_pRenderScene->m_directionalLight.m_color = Vector3(1.0f, 1.0f, 1.0f);	// 设置直射光颜色
	m_pRenderScene->SetVisibleNodesReference();

}

void RenderSystem::Tick(float deltaTime)
{
	// 在逻辑上下文和渲染上下文中交换数据
	ProcessSwapData();

	// 切换命令缓冲区
	m_pRHI->PrepareContext();

	// 准备渲染用到的资源和数据
	m_pRenderPipeline->PreparePassData(m_pRenderResource);

	if (m_renderPipelineType == ERENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)
	{
		m_pRenderPipeline->DeferredRender(m_pRHI, m_pRenderResource);
	}
}

void RenderSystem::Clear()
{
}

void RenderSystem::SwapLogicRenderData()
{
	// 重置渲染数据，并将逻辑数据和渲染数据的索引交换
	m_swapContext.SwapLogicRenderData();
}

std::shared_ptr<RenderCamera> RenderSystem::GetRenderCamera() const
{
	return m_pRenderCamera;
}

void RenderSystem::ProcessSwapData()
{
	ST_RenderSwapData& swapData = m_swapContext.GetRenderSwapData();

	if (swapData.m_levelResourceDesc.has_value())
	{
		m_pRenderResource->UploadGlobalRenderResource(m_pRHI, *swapData.m_levelResourceDesc);

		m_swapContext.ResetLevelRsourceSwapData();
	}

	// 游戏对象加载
	if (swapData.m_gameObjectResourceDesc.has_value())
	{
		while (!swapData.m_gameObjectResourceDesc->IsEmpty())
		{
			// 遍历GameObject的Part
			GameObjectDesc gObject = swapData.m_gameObjectResourceDesc->GetNextProcessObject();
			for (size_t partIndex = 0; partIndex < gObject.GetObjectParts().size(); ++partIndex)
			{
				const ST_GameObjectPartDesc& gameObjectPart = gObject.GetObjectParts()[partIndex];
				ST_GameObjectPartId partId = { gObject.GetId(), partIndex };

				// RenderScene中是否有该Entity
				bool isEntityInScene = m_pRenderScene->GetInstanceIdAllocator().HasElement(partId);

				// 将Part的数据 转为 Entity的数据
				RenderEntity renderEntity;
				renderEntity.m_instanceId = static_cast<uint32_t>(m_pRenderScene->GetInstanceIdAllocator().AllocGuid(partId));
				renderEntity.m_modelMatrix = gameObjectPart.m_transformDesc.m_transformMatrix;

				// Entity的Id 关联 GameObject的Id
				m_pRenderScene->AddInstanceIdToMap(renderEntity.m_instanceId, gObject.GetId());

				// 模型资源是否已加载
				ST_MeshSourceDesc meshSource = { gameObjectPart.m_meshDesc.m_meshFile };
				bool isMeshLoaded = m_pRenderScene->GetMeshAssetIdAllocator().HasElement(meshSource);

				ST_RenderMeshData meshData;
				if (!isMeshLoaded)	// 未加载
				{
					// 加载网格数据，并获取boundingBox
					meshData = m_pRenderResource->LoadMeshData(meshSource, renderEntity.m_boundingBox);
				}
				else //已加载
				{
					// 只获取boundingBox
					renderEntity.m_boundingBox = m_pRenderResource->GetCachedBoudingBox(meshSource);
				}

				// 获取关节的变换矩阵
				renderEntity.m_meshAssetId = m_pRenderScene->GetMeshAssetIdAllocator().AllocGuid(meshSource);
				renderEntity.m_enableVertexBlending = gameObjectPart.m_skeletonAnimationResult.m_transforms.size() > 1u;
				renderEntity.m_jointMatrices.resize(gameObjectPart.m_skeletonAnimationResult.m_transforms.size());
				for (size_t i = 0; i < gameObjectPart.m_skeletonAnimationResult.m_transforms.size(); ++i)
				{
					renderEntity.m_jointMatrices[i] = gameObjectPart.m_skeletonAnimationResult.m_transforms[i].m_matrix;
				}

				// 材质数据
				ST_MaterialSourceDesc materialSource;
				if (gameObjectPart.m_materialDesc.m_withTexture)
				{
					materialSource = {
						gameObjectPart.m_materialDesc.m_baseColorTextureFile,	// 基础颜色贴图
						gameObjectPart.m_materialDesc.m_metallicRoughnessTextureFile,	// 金属度-粗糙度贴图
						gameObjectPart.m_materialDesc.m_normalTextureFile,	// 法线贴图
						gameObjectPart.m_materialDesc.m_occlusionTextureFile,	// 遮挡贴图
						gameObjectPart.m_materialDesc.m_emissiveTextureFile	// 自发光贴图
					};
				}
				else
				{
					// 使用默认贴图
					materialSource = {};
				}

				bool isMaterialLoaded = m_pRenderScene->GetMaterialAssetIdAllocator().HasElement(materialSource);
				ST_RenderMaterialData materialData;
				if (!isMaterialLoaded)
				{
					// 加载材质数据
					materialData = m_pRenderResource->LoadMaterialData(materialSource);
				}
				renderEntity.m_materialAssetId = m_pRenderScene->GetMaterialAssetIdAllocator().AllocGuid(materialSource);

				// 将渲染数据缓存至RenderResource
				if (!isMeshLoaded)	// 网格数据
				{
					m_pRenderResource->UploadGameObjectRenderResource(m_pRHI, renderEntity, meshData);
				}
				if (!isMaterialLoaded)	// 材质数据
				{
					m_pRenderResource->UploadGameObjectRenderResource(m_pRHI, renderEntity, materialData);
				}

				if (!isEntityInScene)
				{
					m_pRenderScene->m_renderEntities.push_back(renderEntity);
				}
				else
				{
					for (RenderEntity& entity : m_pRenderScene->m_renderEntities)
					{
						if (entity.m_instanceId == renderEntity.m_instanceId)
						{
							entity = renderEntity;
							break;
						}
					}
				}
			}

			swapData.m_gameObjectResourceDesc->Pop();	// 弹出第一个
		}

		m_swapContext.ResetGameObjectResourceSwapData();	// 重置数据
	}

	// 游戏对象删除
	if (swapData.m_gameObjectToDelete.has_value())
	{
		while (!swapData.m_gameObjectToDelete->IsEmpty())
		{
			const GameObjectDesc& gObject = swapData.m_gameObjectToDelete->GetNextProcessObject();
			m_pRenderScene->DeleteEntityByGObjectID(gObject.GetId());

			swapData.m_gameObjectToDelete->Pop();
		}
	}

	// 相机数据
	if (swapData.m_cameraSwapData.has_value())
	{

	}
}

NAMESPACE_XYH_END

