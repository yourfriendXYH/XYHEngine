#pragma once
#include <Common.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <functional>
#include <sstream>

#include <Runtime/Core/Macro.h>

NAMESPACE_XYH_BEGIN

class AssetManager
{
public:
	// 加载资源
	template<typename AssetType>
	bool LoadAsset(const std::string& assetURL, AssetType& outAsset)
	{
        // 读取json文件到字符串
        std::filesystem::path assetPath = GetFullPath(assetURL);
        std::ifstream assetJsonFile(assetPath);
		if (!assetJsonFile) // 文件打开失败
        {
            LOG_ERROR("open file: {} failed!", assetPath.generic_string());
            return false;
        }

		// josn文件内容转为字符串
        std::stringstream buffer;
        buffer << assetJsonFile.rdbuf();
        std::string assetJsonText(buffer.str());

        // parse to json object and read to runtime res object
        std::string error;
        //auto&& assetJson = Json::parse(assetJsonText, error);
        //if (!error.empty())
        //{
        //    LOG_ERROR("parse json file {} failed!", assetURL);
        //    return false;
        //}

        //Serializer::read(assetJson, outAsset);
        return true;
	}

	std::filesystem::path GetFullPath(const std::string& relativePath);
};

NAMESPACE_XYH_END