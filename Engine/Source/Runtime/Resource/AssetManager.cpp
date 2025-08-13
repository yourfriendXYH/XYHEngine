#include "AssetManager.h"
#include <Runtime/Function/GlobalContext.h>
#include <Runtime/Resource/ConfigManager.h>

NAMESPACE_XYH_BEGIN

std::filesystem::path AssetManager::GetFullPath(const std::string& relativePath)
{
	return std::filesystem::absolute(g_runtimeGlobalContext.m_pConfigManager->GetRootFolder() / relativePath);
}

NAMESPACE_XYH_END

