#include "ConfigManager.h"
#include <fstream>

NAMESPACE_XYH_BEGIN

void ConfigManager::Initialize(const std::filesystem::path& configFilePath)
{
    // 读取配置文件的路径
    std::ifstream configFile(configFilePath);
    std::string configLine;
    while (std::getline(configFile, configLine))
    {
        size_t seperate_pos = configLine.find_first_of('=');
        if (seperate_pos > 0 && seperate_pos < (configLine.length() - 1))
        {
            std::string name = configLine.substr(0, seperate_pos);
            std::string value = configLine.substr(seperate_pos + 1, configLine.length() - seperate_pos - 1);
            if (name == "BinaryRootFolder")
            {
                m_rootFolder = configFilePath.parent_path() / value;
            }
            else if (name == "AssetFolder")
            {
                m_assetFolder = m_rootFolder / value;
            }
            else if (name == "SchemaFolder")
            {
                m_schemaFolder = m_rootFolder / value;
            }
            else if (name == "DefaultWorld")
            {
                m_defaultWorldURL = value;
            }
            else if (name == "BigIconFile")
            {
                m_editorBigIconPath = m_rootFolder / value;
            }
            else if (name == "SmallIconFile")
            {
                m_editorSmallIconPath = m_rootFolder / value;
            }
            else if (name == "FontFile")
            {
                m_editorFontPath = m_rootFolder / value;
            }
            else if (name == "GlobalRenderingRes")
            {
                m_globalRenderingResourceURL = value;
            }
            else if (name == "GlobalParticleRes")
            {
                m_globalParticleResourceURL = value;
            }
        }
    }
}

const std::filesystem::path& ConfigManager::GetRootFolder() const
{
	return m_rootFolder;
}

const std::filesystem::path& ConfigManager::GetAssetFolder() const
{
	return m_assetFolder;
}

const std::filesystem::path& ConfigManager::GetSchemaFolder() const
{
	return m_schemaFolder;
}

const std::filesystem::path& ConfigManager::GetEditorBigIconPath() const
{
	return m_editorBigIconPath;
}

const std::filesystem::path& ConfigManager::GetEditorSmallIconPath() const
{
	return m_editorSmallIconPath;
}

const std::filesystem::path& ConfigManager::GetEditorFontPath() const
{
	return m_editorFontPath;
}

const std::string& ConfigManager::GetDefaultWorldURL() const
{
	return m_defaultWorldURL;
}

const std::string& ConfigManager::GetGlobalRenderingResURL() const
{
	return m_globalRenderingResourceURL;
}

const std::string& ConfigManager::GetGlobalParticleResURL() const
{
	return m_globalParticleResourceURL;
}

NAMESPACE_XYH_END
