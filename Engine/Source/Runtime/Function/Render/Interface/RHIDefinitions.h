#include <Common.h>

NAMESPACE_XYH_BEGIN

enum class EBufferUsageFlags : uint32_t
{
	None = 0,

	Static = 1 << 0,	// 缓冲区将被写入一次。

	Dynamic = 1 << 1,	// 缓冲区将偶尔被写入，仅GPU可读，CPU可写。数据的有效期持续到下次更新或缓冲区被销毁为止。

	Volatile = 1 << 2,	// 缓冲区的数据生命周期为一个帧。必须在每个帧中写入数据，或者在每个帧时创建一个新的缓冲区。

	UnorderedAccess = 1 << 3,	// 允许为缓冲区创建无序访问视图
};

NAMESPACE_XYH_END