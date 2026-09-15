#pragma once
#include <string>
#include <Common.h>
#include "RHIDefinitions.h"

NAMESPACE_XYH_BEGIN

struct RHIBufferDesc
{
	uint32_t m_size = 0;

	uint32_t m_stride = 0;

	EBufferUsageFlags m_usage = EBufferUsageFlags::None;

	RHIBufferDesc() = default;

	RHIBufferDesc(uint32_t inSize, uint32_t inStride, EBufferUsageFlags inUsage)
		:
		m_size(inSize),
		m_stride(inStride),
		m_usage(inUsage)
	{

	}

	bool operator==(const RHIBufferDesc& other) const
	{
		return m_size == other.m_size
			&& m_stride == other.m_stride
			&& m_usage == other.m_usage;
	}

	bool operator!=(const RHIBufferDesc& other) const
	{
		return !(*this == other);
	}

	RHIBufferDesc& operator=(const RHIBufferDesc& other)
	{
		m_size = other.m_size;
		m_stride = other.m_stride;
		m_usage = other.m_usage;

		return *this;
	}
};

struct RHIBufferCreateDesc : public RHIBufferDesc
{

};

class RHIBuffer
{
public:
	RHIBuffer() {};
	RHIBuffer(const RHIBufferDesc& createDesc);

public:

	const RHIBufferDesc& GetDesc() const
	{
		return m_desc;
	}

	uint32_t GetSize() const
	{
		return m_desc.m_size;
	}

	uint32_t GetStride() const
	{
		return m_desc.m_stride;
	}

	EBufferUsageFlags GetUsage() const
	{
		return m_desc.m_usage;
	}

	void SetName(const std::string name)
	{
		m_name = name;
	}

private:

	RHIBufferDesc m_desc;
	std::string m_name;
};

NAMESPACE_XYH_END