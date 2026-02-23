#pragma once
#include <string>
#include "..\Common\Common.h"

NAMESPACE_XYH_BEGIN

class XYHEngine
{
	friend class XYHEditor;

public:
	void StartEngine(const std::string& configFilePath);	// 启动引擎

	void ShutdownEngine();

	void Inititalize();

	void Clear();

	bool IsQuit() const;

	void Run();

	bool TickOneFrame(float deltaTime);

	int GetFPS();

protected:
	// 逻辑帧
	void LogicalTick(float deltaTime);

	// Frames Per Second: 每秒钟渲染的帧数
	// 计算每秒的帧率
	void CalculateFPS(float deltaTime);

	// 渲染帧
	bool RendererTick(float deltaTime);

	// 每帧只调用一次
	// 计算每帧的持续时间
	float CalculateDeltaTime();

public:

	static const float s_fpsAlpha;

private:

	bool m_isQuit = false;

	// 帧率计算变量
	float m_averageDuration = 0.0f;	// 平均每帧的持续时间
	int m_frameCount = 0;	// 当前帧数
	int m_fps = 0;	// 当前帧率
};

NAMESPACE_XYH_END