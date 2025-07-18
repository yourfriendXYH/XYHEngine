#pragma once
#include <string>
#include "..\Common\Common.h"

NAMESPACE_XYH_BEGIN

class XYHEngine
{
public:
	void StartEngine(const std::string& configFilePath);

	void ShutdownEngine();

	void Inititalize();

	void Clear();

	bool IsQuit() const;

	void Run();

	void TickOneFrame();

	int GetFPS();


protected:


private:

	bool m_isQuit = false;
	int m_fps = 0;
};

NAMESPACE_XYH_END