#include "RenderSystem.h"
#include "../GlobalContext.h"
#include "../../Resource/ConfigManager.h"
#include "Interface/RHI.h"
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"
#include "RenderCamera.h"
#include "RenderScene.h"
#include "RenderPipeline.h"
#include <Runtime/Function/Render/Interface/DX12/D3D12RHI.h>
#include <Runtime/Function/Render/Passes/ParticlePass.h>

NAMESPACE_XYH_BEGIN

#define USE_DX12
//#define USE_VK

RenderSystem::~RenderSystem()
{
	Clear();
}

void RenderSystem::Initialize(ST_RenderSystemInitInfo initInfo)
{
	std::shared_ptr<ConfigManager> pConfigManager = g_runtimeGlobalContext.m_pConfigManager;

	// RHI初始化
	ST_RHIInitInfo rhiInitInfo;
	rhiInitInfo.m_pWindowSystem = initInfo.m_pWindowSystem;
#ifdef USE_DX12	// 使用Direct3D12
	m_pRHI = std::make_shared<D3D12RHI>();
	m_pRHI->Initialize(rhiInitInfo);
#endif // USE_DX12
#ifdef USE_VK	// 使用Vulkan
	m_pRHI = std::make_shared<VulkanRHI>();
	m_pRHI->Initialize(rhiInitInfo);
#endif // USE_VK

	// 全局渲染资源
	// GlobalRenderingRes globalRenderingRes;
	const std::string& globalRenderingResURL = pConfigManager->GetGlobalRenderingResURL();
	// asset_manager->loadAsset(global_rendering_res_url, global_rendering_res);

	// 上传ibl，颜色分级纹理
	ST_LevelResourceDesc levelResourceDesc;	// 关卡资源描述
	SkyBoxIrradianceMap testSkyBoxIrradianceMap;
	SkyBoxSpecularMap testSkyBoxSpecularMap;
	// 路径赋值
	levelResourceDesc.m_iblResourceDesc.m_skyboxIrradianceMap = testSkyBoxIrradianceMap;
	levelResourceDesc.m_iblResourceDesc.m_skyboxSpecularMap = testSkyBoxSpecularMap;
	levelResourceDesc.m_iblResourceDesc.m_brdfMap = "";
	levelResourceDesc.m_colorGradingResourceDesc.m_colorGradingMap = "";

	// 渲染资源管理器
	m_pRenderResource = std::make_shared<RenderResource>();
	m_pRenderResource->UploadGlobalRenderResource(m_pRHI, levelResourceDesc);	// 上传全局渲染资源

	// 渲染相机
	// 初始化相机参数
	m_pRenderCamera = std::make_shared<RenderCamera>();	// 创建渲染相机
	m_pRenderCamera->LookAt(Vector3(2.0f, 2.0f, 2.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));	// 设置相机初始位置和方向
	m_pRenderCamera->m_zFar = 100.0f;	// 设置远裁剪面
	m_pRenderCamera->m_zNear = 0.1f;		// 设置近裁剪面
	m_pRenderCamera->SetAspectRatio(1280.0f / 720.0f);	// 设置宽高比

	// 渲染场景
	m_pRenderScene = std::make_shared<RenderScene>();	// 创建渲染场景
	m_pRenderScene->m_ambientLight = { Vector3(0.1f, 0.1f, 0.1f) };	// 设置环境光颜色
	m_pRenderScene->m_directionalLight.m_direction = Vector3(-1.0f, -1.0f, -1.0f).normalisedCopy();	// 设置直射光方向
	m_pRenderScene->m_directionalLight.m_color = Vector3(1.0f, 1.0f, 1.0f);	// 设置直射光颜色
	m_pRenderScene->SetVisibleNodesReference();

	// 渲染管线
	m_pRenderPipeline = std::make_shared<RenderPipeline>();
	m_pRenderPipeline->m_pRHI = m_pRHI;
	ST_RenderPipelineInitInfo pipelineInitInfo;
	pipelineInitInfo.m_enableFXAA = true;	// TODO
	pipelineInitInfo.m_pRenderResource = m_pRenderResource;
	m_pRenderPipeline->Initialize(pipelineInitInfo);
}

void RenderSystem::Tick(float deltaTime)
{
	// 分发渲染数据
	ProcessSwapData();

	// 切换至当前帧的命令缓冲区
	m_pRHI->PrepareContext();

	// 更新每帧的缓冲区
	m_pRenderResource->UpdatePerFrameBuffer(m_pRenderScene, m_pRenderCamera);

	// 更新每帧的对象
	// 可见对象更新
	m_pRenderScene->UpdateVisibleObjects(std::static_pointer_cast<RenderResource>(m_pRenderResource), m_pRenderCamera);

	// 准备渲染用到的资源和数据
#ifdef USE_VK	// 使用Vulkan
	m_pRenderPipeline->PreparePassData(m_pRenderResource);
#endif // USE_VK

	// 执行渲染流程
	if (m_renderPipelineType == ERENDER_PIPELINE_TYPE::FORWARD_PIPELINE)	// 前向渲染
	{
		// 暂不处理
	}
	else if (m_renderPipelineType == ERENDER_PIPELINE_TYPE::DEFERRED_PIPELINE)	// 延迟渲染
	{
		m_pRenderPipeline->DeferredRender(m_pRHI, m_pRenderResource);
	}
}

void RenderSystem::Clear()
{
	if (nullptr != m_pRHI)
	{
		m_pRHI->Clear();
	}
	m_pRHI.reset();

	if (nullptr != m_pRenderScene)
	{
		m_pRenderScene->Clear();
	}
	m_pRenderScene.reset();

	if (nullptr != m_pRenderResource)
	{
		m_pRenderResource->Clear();
	}
	m_pRenderResource.reset();

	if (nullptr != m_pRenderPipeline)
	{
		m_pRenderPipeline->Clear();
	}
	m_pRenderPipeline.reset();
}

void RenderSystem::SwapLogicRenderData()
{
	// 重置渲染数据，并将逻辑数据和渲染数据的索引交换
	m_swapContext.SwapLogicRenderData();
}

RenderSwapContext& RenderSystem::GetSwapContext()
{
	return m_swapContext;
}

std::shared_ptr<RHI> RenderSystem::GetRHI() const
{
	return m_pRHI;
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
				if (!isMeshLoaded) // 未加载
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

	// 相机数据更新
	if (swapData.m_cameraSwapData.has_value())
	{
		if (swapData.m_cameraSwapData->m_fovX.has_value())
		{
			// 水平方向视场角
			// fovX = 2 * atan(tan(fovY / 2) * aspect)
			// fovY = 2 * atan(tan(fovX / 2) / aspect)
			m_pRenderCamera->SetFovX(*swapData.m_cameraSwapData->m_fovX);
		}

		if (swapData.m_cameraSwapData->m_viewMatrix.has_value())
		{
			m_pRenderCamera->SetMainViewMatrix(*swapData.m_cameraSwapData->m_viewMatrix);
		}

		if (swapData.m_cameraSwapData->m_cameraType.has_value())
		{
			// 相机类型
			m_pRenderCamera->SetCurrentCameraType(*swapData.m_cameraSwapData->m_cameraType);
		}

		m_swapContext.ResetCameraSwapData();
	}

	// 粒子数据更新
	std::shared_ptr<ParticlePass> pParticlePass = std::static_pointer_cast<ParticlePass>(m_pRenderPipeline->m_pParticlePass);
	assert(pParticlePass);
	if (swapData.m_particleSubmitRequest.has_value())
	{
		int emitterCount = swapData.m_particleSubmitRequest->GetEmitterCount();
		pParticlePass->SetEmitterCount(emitterCount);

		for (int i = 0; i < emitterCount; i++)
		{
			const ST_ParticleEmitterDesc& desc = swapData.m_particleSubmitRequest->GetEmitterDesc(i);
			pParticlePass->CreateEmitter(i, desc);
		}
		pParticlePass->InitializeEmitters();

		m_swapContext.ResetPartilceBatchSwapData();
	}
	if (swapData.m_emitterTickRequest.has_value())
	{
		pParticlePass->SetTickIndices(swapData.m_emitterTickRequest->m_emitterIndices);

		m_swapContext.ResetEmitterTickSwapData();
	}
	if (swapData.m_emitterTransformRequest.has_value())
	{
		pParticlePass->SetTransformIndices(swapData.m_emitterTransformRequest->m_transformDescs);

		m_swapContext.ResetEmitterTransformSwapData();
	}
}

NAMESPACE_XYH_END

