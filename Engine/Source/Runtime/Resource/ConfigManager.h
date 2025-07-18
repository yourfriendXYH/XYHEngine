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
	std::filesystem::path m_rootFolder; // ��Ŀ¼
	std::filesystem::path m_assetFolder; // �ʲ�Ŀ¼
	std::filesystem::path m_schemaFolder; // ģʽĿ¼
	std::filesystem::path m_editorBigIconPath; // �༭����ͼ��·��
	std::filesystem::path m_editorSmallIconPath; // �༭��Сͼ��·��
	std::filesystem::path m_editorFontPath; // �༭������·��

	std::string m_defaultWorldURL; // Ĭ������URL
	std::string m_globalRenderingResourceURL; // ȫ����Ⱦ��ԴURL
	std::string m_globalParticleResourceURL; // ȫ��������ԴURL
};

NAMESPACE_XYH_END
