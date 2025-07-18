#include "GlobalContext.h"
#include "../Core/LogSystem.h"
#include "../Resource/ConfigManager.h"
#include "Render/RenderSystem.h"

NAMESPACE_XYH_BEGIN

RuntimeGlobalContext g_runtimeGlobalContext;

void RuntimeGlobalContext::InitSystems(const std::string& configFilePath)
{
	m_pLogSystem = std::make_shared<LogSystem>(); // ��־ϵͳ

	m_pConfigManager = std::make_shared<ConfigManager>(); // ���ù�����
	m_pConfigManager->Initialize(configFilePath); // ��ʼ�����ù�����

	m_pRenderSystem = std::make_shared<RenderSystem>(); // ��Ⱦϵͳ
	ST_RenderSystemInitInfo renderInitInfo; // ��Ⱦϵͳ��ʼ����Ϣ
	renderInitInfo;
	m_pRenderSystem->Initialize(renderInitInfo); // ��ʼ����Ⱦϵͳ
}

void RuntimeGlobalContext::ShutdownSystems()
{
	m_pLogSystem.reset(); // ������־ϵͳ

	m_pRenderSystem->Clear(); // ������Ⱦϵͳ
	m_pRenderSystem.reset(); // ������Ⱦϵͳָ��
}

NAMESPACE_XYH_END

