#pragma once
#include <Common.h>
#include <string>
#include <filesystem>

NAMESPACE_XYH_BEGIN

class ConfigManager
{
public:
	void Initialize(const std::filesystem::path& configFilePath);

	const std::filesystem::path& GetRootFolder() const;
	const std::filesystem::path& GetAssetFolder() const;
	const std::filesystem::path& GetSchemaFolder() const;
	const std::filesystem::path& GetEditorBigIconPath() const;
	const std::filesystem::path& GetEditorSmallIconPath() const;
	const std::filesystem::path& GetEditorFontPath() const;

	const std::string& GetDefaultWorldURL() const;
	const std::string& GetGlobalRenderingResURL() const;
	const std::string& GetGlobalParticleResURL() const;

private:
	std::filesystem::path m_rootFolder; // 根目录
	std::filesystem::path m_assetFolder; // 资产目录
	std::filesystem::path m_schemaFolder; // 模式目录
	std::filesystem::path m_editorBigIconPath; // 编辑器大图标路径
	std::filesystem::path m_editorSmallIconPath; // 编辑器小图标路径
	std::filesystem::path m_editorFontPath; // 编辑器字体路径

	std::string m_defaultWorldURL; // 默认世界URL
	std::string m_globalRenderingResourceURL; // 全局渲染资源URL
	std::string m_globalParticleResourceURL; // 全局粒子资源URL
};

NAMESPACE_XYH_END
