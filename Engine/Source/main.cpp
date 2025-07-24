#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include "Runtime/Engine.h"
#include "Editor/Editor.h"
#include "Runtime/Core/Macro.h"


int main(int argc, char** argv)
{
	std::cout << "Hello XYH Engine." << std::endl;

	std::filesystem::path executablePath(argv[0]);
	std::filesystem::path configFilePath = executablePath.parent_path() / "XYHEngineConfig.ini";

	XYH::XYHEngine* pEngine = new XYH::XYHEngine();

	pEngine->StartEngine(configFilePath.generic_string());
	pEngine->Inititalize();

	XYH::XYHEditor* pEditor = new XYH::XYHEditor();
	pEditor->Initialize(pEngine);
	pEditor->Run();
	pEditor->Clear();

	LOG_DEBUG("XYH");
	LOG_INFO("XYH");
	LOG_WARN("XYH");
	LOG_ERROR("XYH");
	//LOG_FATAL("XYH");

	THREAD_SLEEP(1000);

	pEngine->Clear();
	pEngine->ShutdownEngine();

	return 0;
}