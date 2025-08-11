#pragma once
#include <Common.h>
#include <unordered_map>

NAMESPACE_XYH_BEGIN

static const size_t s_invalidGuid = 0;	// 无效的GUID

// GUID分配器类
template<typename T>
class GuidAllocator
{
public:

	// 检查GUID是否有效
    static bool IsValidGuid(size_t guid) { return guid != s_invalidGuid; }

	// 分配GUID
    size_t AllocGuid(const T& t)
    {
        auto findIt = m_elementsGuidMap.find(t);
        if (findIt != m_elementsGuidMap.end())
        {
            return findIt->second;
        }

        for (size_t i = 0; i < m_guidElementsMap.size() + 1; i++)
        {
            size_t guid = i + 1;
            if (m_guidElementsMap.find(guid) == m_guidElementsMap.end())
            {
                m_guidElementsMap.insert(std::make_pair(guid, t));
                m_elementsGuidMap.insert(std::make_pair(t, guid));
                return guid;
            }
        }

        return s_invalidGuid;
    }

    bool GetGuidRelatedElement(size_t guid, T& t)
    {
        auto findIt = m_guidElementsMap.find(guid);
        if (findIt != m_guidElementsMap.end())
        {
            t = findIt->second;
            return true;
        }
        return false;
    }

    bool GetElementGuid(const T& t, size_t& guid)
    {
        auto findIt = m_elementsGuidMap.find(t);
        if (findIt != m_elementsGuidMap.end())
        {
            guid = findIt->second;
            return true;
        }
        return false;
    }

    bool HasElement(const T& t) { return m_elementsGuidMap.find(t) != m_elementsGuidMap.end(); }

    void FreeGuid(size_t guid)
    {
        auto findIt = m_guidElementsMap.find(guid);
        if (findIt != m_guidElementsMap.end())
        {
            const auto& ele = findIt->second;
            m_elementsGuidMap.erase(ele);
            m_guidElementsMap.erase(guid);
        }
    }

    void FreeElement(const T& t)
    {
        auto findIt = m_elementsGuidMap.find(t);
        if (findIt != m_elementsGuidMap.end())
        {
            const auto& guid = findIt->second;
            m_elementsGuidMap.erase(t);
            m_guidElementsMap.erase(guid);
        }
    }

    std::vector<size_t> GetAllocatedGuids() const
    {
        std::vector<size_t> allocatedGuids;
        for (const auto& ele : m_guidElementsMap)
        {
            allocatedGuids.push_back(ele.first);
        }
        return allocatedGuids;
    }

    void clear()
    {
        m_elementsGuidMap.clear();
        m_guidElementsMap.clear();
    }

private:
    std::unordered_map<T, size_t> m_elementsGuidMap;
    std::unordered_map<size_t, T> m_guidElementsMap;
};

NAMESPACE_XYH_END
