#pragma once
#include <string>
#include <Common.h>
#include "RHIDefinitions.h"
#include "RHIAccess.h"
#include "RHIUtilities.h"

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

enum class ERHIBufferInitAction
{
	// Default for the RHI, data can be "undefined"
	Default,

	// Zero all buffer data
	Zeroed,

	// Upload data from a provided FResourceArrayUploadInterface. This data will be discarded after it's used.
	ResourceArray,

	// Caller will use FRHIBufferInitializer to set the initial buffer contents.
	Initializer,
};

struct RHIBufferCreateDesc : public RHIBufferDesc
{
	static RHIBufferCreateDesc Create(const char* InDebugName, EBufferUsageFlags InUsage)
	{
		return RHIBufferCreateDesc(InDebugName, InUsage);
	}

	static RHIBufferCreateDesc Create(const char* InDebugName, uint32_t InSize, uint32_t InStride, EBufferUsageFlags InUsage)
	{
		return RHIBufferCreateDesc(InDebugName, InSize, InStride, InUsage);
	}

	static RHIBufferCreateDesc Create(const char* InDebugName, const RHIBufferDesc& InDesc)
	{
		return RHIBufferCreateDesc(InDebugName, InDesc);
	}

	static RHIBufferCreateDesc CreateNull(const char* InDebugName)
	{
		return Create(InDebugName, 0, 0, EBufferUsageFlags::NullResource);
	}

	static RHIBufferCreateDesc CreateVertex(const char* InDebugName)
	{
		return Create(InDebugName, EBufferUsageFlags::VertexBuffer);
	}

	static RHIBufferCreateDesc CreateVertex(const char* InDebugName, uint32_t InSize)
	{
		return Create(InDebugName, InSize, 0, EBufferUsageFlags::VertexBuffer);
	}

	template<typename TVertexType>
	static RHIBufferCreateDesc CreateVertex(const char* InDebugName, uint32_t InCount)
	{
		return Create(InDebugName, InCount * sizeof(TVertexType), 0, EBufferUsageFlags::VertexBuffer);
	}

	static RHIBufferCreateDesc CreateIndex(const char* InDebugName)
	{
		return Create(InDebugName, EBufferUsageFlags::IndexBuffer);
	}

	static RHIBufferCreateDesc CreateIndex(const char* InDebugName, uint32_t InSize, uint32_t InStride)
	{
		return Create(InDebugName, InSize, InStride, EBufferUsageFlags::IndexBuffer);
	}

	template<typename TIndexType>
	static RHIBufferCreateDesc CreateIndex(const char* InDebugName, uint32_t InCount)
	{
		return Create(InDebugName, InCount * sizeof(TIndexType), sizeof(TIndexType), EBufferUsageFlags::IndexBuffer);
	}

	static RHIBufferCreateDesc CreateStructured(const char* InDebugName)
	{
		return Create(InDebugName, EBufferUsageFlags::StructuredBuffer);
	}

	static RHIBufferCreateDesc CreateStructured(const char* InDebugName, uint32_t InSize, uint32_t InStride)
	{
		return Create(InDebugName, InSize, InStride, EBufferUsageFlags::StructuredBuffer);
	}

	template<typename TStructureType>
	static RHIBufferCreateDesc CreateStructured(const char* InDebugName, uint32_t InCount)
	{
		return Create(InDebugName, InCount * sizeof(TStructureType), sizeof(TStructureType), EBufferUsageFlags::StructuredBuffer);
	}

	static RHIBufferCreateDesc CreateByteAddress(const char* InDebugName)
	{
		return Create(InDebugName, EBufferUsageFlags::ByteAddressBuffer);
	}

	static RHIBufferCreateDesc CreateByteAddress(const char* InDebugName, uint32_t InSize, uint32_t InStride)
	{
		return Create(InDebugName, InSize, InStride, EBufferUsageFlags::ByteAddressBuffer);
	}

	static RHIBufferCreateDesc CreateUniform(const char* InDebugName)
	{
		return Create(InDebugName, EBufferUsageFlags::UniformBuffer);
	}

	RHIBufferCreateDesc() = default;

	RHIBufferCreateDesc(const char* InDebugName, EBufferUsageFlags InUsage)
		: m_debugName(InDebugName)
	{
		m_usage = InUsage;
	}

	RHIBufferCreateDesc(const char* InDebugName, uint32_t InSize, uint32_t InStride, EBufferUsageFlags InUsage)
		: RHIBufferDesc(InSize, InStride, InUsage)
		, m_debugName(InDebugName)
	{
	}

	RHIBufferCreateDesc(const char* InDebugName, const RHIBufferDesc& InOtherDesc)
		: RHIBufferDesc(InOtherDesc)
		, m_debugName(InDebugName)
	{
	}

	RHIBufferCreateDesc& SetSize(uint32_t InSize) { m_size = InSize;     return *this; }
	RHIBufferCreateDesc& SetStride(uint32_t InStride) { m_stride = InStride;   return *this; }
	RHIBufferCreateDesc& SetUsage(EBufferUsageFlags InUsage) { m_usage = InUsage;    return *this; }
	RHIBufferCreateDesc& AddUsage(EBufferUsageFlags InUsage) { m_usage |= InUsage;    return *this; }
	//RHIBufferCreateDesc& SetGPUMask(FRHIGPUMask InGPUMask) { GPUMask = InGPUMask; return *this; }

	RHIBufferCreateDesc& SetDebugName(const char* InDebugName) { m_debugName = InDebugName;       return *this; }
	RHIBufferCreateDesc& SetInitialState(ERHIAccess InInitialState) { m_initialState = InInitialState; return *this; }
	RHIBufferCreateDesc& DetermineInitialState() { if (m_initialState == ERHIAccess::Unknown) m_initialState = RHIGetDefaultResourceState(m_usage, false); return *this; }
	RHIBufferCreateDesc& SetInitAction(ERHIBufferInitAction InInitAction) { m_initAction = InInitAction;     return *this; }
	RHIBufferCreateDesc& SetClassName(const std::string& InClassName) { m_className = InClassName;       return *this; }
	RHIBufferCreateDesc& SetOwnerName(const std::string& InOwnerName) { m_ownerName = InOwnerName;       return *this; }

	RHIBufferCreateDesc& SetInitActionNone()
	{
		return SetInitAction(ERHIBufferInitAction::Default);
	}
	RHIBufferCreateDesc& SetInitActionZeroData()
	{
		return SetInitAction(ERHIBufferInitAction::Zeroed);
	}
	RHIBufferCreateDesc& SetInitActionInitializer()
	{
		return SetInitAction(ERHIBufferInitAction::Initializer);
	}
	//RHIBufferCreateDesc& SetInitActionResourceArray(FResourceArrayUploadInterface* InInitialData)
	//{
	//	InitialData = InInitialData;
	//	return SetInitAction(ERHIBufferInitAction::ResourceArray);
	//}

	std::string GetTraceClassName() const
	{
		return m_className;
	}

	/* A friendly name for the resource. */
	const char* m_debugName = nullptr;

	/* Provider of initial data for the buffer. InitialData->Discard() will be called after the data is read. */
	//FResourceArrayUploadInterface* InitialData = nullptr;

	/* The RHI access state that the resource will be created in. */
	ERHIAccess m_initialState = ERHIAccess::Unknown;

	/* Tells how to initialize (or not) the buffer's data. */
	ERHIBufferInitAction m_initAction = ERHIBufferInitAction::Default;

	/* The owner class of FRHIBuffer used for Insight asset metadata tracing */
	std::string m_className = "";

	/* The owner name used for Insight asset metadata tracing */
	std::string m_ownerName = "";
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