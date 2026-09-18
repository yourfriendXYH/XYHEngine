#pragma once
#include <Common.h>
#include <Runtime/Core/Misc/EnumClassFlags.h>

NAMESPACE_XYH_BEGIN

#define RHI_RAW_VIEW_ALIGNMENT 16

enum class EBufferUsageFlags : uint32_t
{
	None = 0,

	Static = 1 << 0,	// 缓冲区将被写入一次。

	Dynamic = 1 << 1,	// 缓冲区将偶尔被写入，仅GPU可读，CPU可写。数据的有效期持续到下次更新或缓冲区被销毁为止。

	Volatile = 1 << 2,	// 缓冲区的数据生命周期为一个帧。必须在每个帧中写入数据，或者在每个帧时创建一个新的缓冲区。

	UnorderedAccess = 1 << 3,	// 允许为缓冲区创建无序访问视图

	ByteAddressBuffer = 1 << 4,	// 创建一个字节地址缓冲区，本质上是一个带有uint32类型的结构化缓冲区。

	DrawIndirect = 1 << 7,	// 创建一个缓冲区，其中包含 DispatchIndirect 或 DrawIndirect 使用的参数。

	/**
	* 创建一个可绑定为着色器资源的缓冲区。
	* 仅适用于通常不会用作着色器资源的缓冲区类型，例如顶点缓冲区。（顶点缓冲区也需要当作SRV）
	*/
	ShaderResource = 1 << 8,

	Shared = 1 << 12,	// 创建一个可以与外部RHI或进程共享的缓冲区。

	/**
	* 缓冲区包含不透明的光线追踪加速结构数据。
	* 带有此标志的资源无法直接绑定到任何着色器阶段，只能与光线追踪 API 一起使用。
	* 此标志与其他所有缓冲区标志（除静态和保留资源外）互斥。
	*/
	AccelerationStructure = 1 << 13,

	VertexBuffer = 1 << 14,
	IndexBuffer = 1 << 15,
	StructuredBuffer = 1 << 16,

	/**
	* 实验性：允许缓冲区在内部以预留资源（即平铺/稀疏/虚拟）形式创建，无需物理内存支持。
	* 不能与动态缓冲区及其他防止资源在本地GPU内存中分配的缓冲标志同时使用。
	*/
	ReservedResource = 1 << 22,

	// Helper bit-masks
	AnyDynamic = (Dynamic | Volatile),
};
ENUM_CLASS_FLAGS(EBufferUsageFlags);

NAMESPACE_XYH_END