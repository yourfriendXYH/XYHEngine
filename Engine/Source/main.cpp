#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include "Runtime/Engine.h"
#include "Editor/Editor.h"


int main(int argc, char** argv)
{
	std::cout << "Hello XYH Engine." << std::endl;

	std::filesystem::path executablePath(argv[0]);
	std::filesystem::path configFilePath = executablePath.parent_path() / "123";

	XYH::XYHEngine* pEngine = new XYH::XYHEngine();

	pEngine->StartEngine();
	pEngine->Inititalize();

	XYH::XYHEditor* pEditor = new XYH::XYHEditor();
	pEditor->Initialize(pEngine);
	pEditor->Run();
	pEditor->Clear();

	pEngine->Clear();
	pEngine->ShutdownEngine();

	return 0;
}