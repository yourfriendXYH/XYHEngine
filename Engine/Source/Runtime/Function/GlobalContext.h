#pragma once
#include <memory>
#include <string>
#include <Common.h>

NAMESPACE_XYH_BEGIN

class RuntimeGlobalContext
{
public:
	// �����ͳ�ʼ������ϵͳ
	void InitSystems(const std::string& configFilePath);
	//��������ϵͳ
	void ShutdownSystems();

public:
	std::shared_ptr<class LogSystem> m_pLogSystem;	// ��־ϵͳ

	std::shared_ptr<class ConfigManager> m_pConfigManager; // ���ù�����

	std::shared_ptr<class RenderSystem> m_pRenderSystem; // ��Ⱦϵͳ
};

extern RuntimeGlobalContext g_runtimeGlobalContext;

NAMESPACE_XYH_END