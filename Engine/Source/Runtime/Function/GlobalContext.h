#pragma once
#include <memory>
#include "../../Common/Common.h"

NAMESPACE_XYH_BEGIN

class RuntimeGlobalContext
{
public:
	// �����ͳ�ʼ������ϵͳ
	void InitSystems();
	//��������ϵͳ
	void ShutdownSystems();

public:
	std::shared_ptr<class LogSystem> m_pLogSystem;


};

extern RuntimeGlobalContext g_runtimeGlobalContext;

NAMESPACE_XYH_END