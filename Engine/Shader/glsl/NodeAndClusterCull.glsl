#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1)in;



// UE5源码位置：NaniteDefinitions.h
#define NANITE_MAX_BVH_NODE_FANOUT_BITS						2
#define NANITE_MAX_BVH_NODE_FANOUT_MASK						((1 << NANITE_MAX_BVH_NODE_FANOUT_BITS)-1)
#define NANITE_MAX_BVH_NODE_FANOUT							(1 << NANITE_MAX_BVH_NODE_FANOUT_BITS)
#define NANITE_BVH_NODE_ENABLE_MASK							((1 << NANITE_MAX_BVH_NODE_FANOUT)-1)

#define NANITE_MAX_CLUSTERS_PER_GROUP_BITS					9
#define NANITE_MAX_CLUSTERS_PER_GROUP_MASK					((1 << NANITE_MAX_CLUSTERS_PER_GROUP_BITS) - 1)
#define NANITE_MAX_CLUSTERS_PER_GROUP						((1 << NANITE_MAX_CLUSTERS_PER_GROUP_BITS) - 1)

#define NANITE_MAX_GROUP_PARTS_BITS							5
#define NANITE_MAX_GROUP_PARTS_MASK							((1 << NANITE_MAX_GROUP_PARTS_BITS) - 1)
#define NANITE_MAX_GROUP_PARTS								(1 << NANITE_MAX_GROUP_PARTS_BITS)

#define NANITE_MAX_RESOURCE_PAGES_BITS						16 // 2GB of 32kb root pages or 4GB of 64kb streaming pages
#define NANITE_MAX_RESOURCE_PAGES_MASK						((1 << NANITE_MAX_RESOURCE_PAGES_BITS) - 1)
#define NANITE_MAX_RESOURCE_PAGES							(1 << NANITE_MAX_RESOURCE_PAGES_BITS)

// 四叉树C++的数据结构
// UE5源码位置：NaniteResources.h
// 208 字节
//	struct FPackedHierarchyNode
//	{
//		FVector4f		LODBounds[NANITE_MAX_BVH_NODE_FANOUT];
//		
//		struct
//		{
//			FVector3f	BoxBoundsCenter;
//			uint32		MinLODError_MaxParentLODError;
//		} Misc0[NANITE_MAX_BVH_NODE_FANOUT];
//	
//		struct
//		{
//			FVector3f	BoxBoundsExtent;
//			uint32		ChildStartReference;
//		} Misc1[NANITE_MAX_BVH_NODE_FANOUT];
//		
//		struct
//		{
//			uint32		ResourcePageIndex_NumPages_GroupPartSize;
//		} Misc2[NANITE_MAX_BVH_NODE_FANOUT];
//	};

// 自定义结构体，用于解码四叉树
struct FPackedHierarchyNode
{
	uvec4 LODBounds[NANITE_MAX_BVH_NODE_FANOUT];
	uvec4 Misc0[NANITE_MAX_BVH_NODE_FANOUT];
	uvec4 Misc1[NANITE_MAX_BVH_NODE_FANOUT];
	uint Misc2[NANITE_MAX_BVH_NODE_FANOUT];
};

// UE5源码位置：NaniteDataDecode.ush
struct FHierarchyNodeSlice
{
//	float4	LODBounds;
//	float3	BoxBoundsCenter;
//	float3	BoxBoundsExtent;
	vec4	LODBounds;			
	vec3	BoxBoundsCenter;	
	vec3	BoxBoundsExtent;
	
	float	MinLODError;
	float	MaxParentLODError;
	uint	ChildStartReference;	// Can be node (index) or cluster (page:cluster)
	uint	NumChildren;
	uint	StartPageIndex;
	uint	NumPages;
	bool	bEnabled;
	bool	bLoaded;
	bool	bLeaf;
};

layout(binding = 0)uniform GlobalConstants
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ModelMatrix;
	uvec4 Misc0;
	vec4 CameraPositionWS;
	vec4 ViewDirectionWS;
};

layout(binding = 1, std430)buffer FBVH
{
	FPackedHierarchyNode m_data[];
}BVH;

layout(binding = 2, std430)buffer FCurrentIndirectWorkArgs
{
	uint m_data[];
}CurrentIndirectWorkArgs;

layout(binding = 3, std430)buffer FNextIndirectWorkArgs
{
	uint m_data[];
}NextIndirectWorkArgs;

layout(binding = 4, std430)buffer FMainAndPostNodeAndClusterBatches
{
	uint m_data[];
	//vec4 m_data[];
}MainAndPostNodeAndClusterBatches;


// UE5源码位置：Platform.ush
uint BitFieldExtractU32(uint Data, uint Size, uint Offset)
{
	// Shift amounts are implicitly &31 in HLSL, so they should be optimized away on most platforms
	// In GLSL shift amounts < 0 or >= word_size are undefined, so we better be explicit
	Size &= 31;
	Offset &= 31;
	return (Data >> Offset) & ((1u << Size) - 1u);
}

// UE5源码位置：NaniteDataDecode.ush
//FHierarchyNodeSlice UnpackHierarchyNodeSlice(uint4 RawData0, uint4 RawData1, uint4 RawData2, uint RawData3)
FHierarchyNodeSlice UnpackHierarchyNodeSlice(uvec4 RawData0, uvec4 RawData1, uvec4 RawData2, uint RawData3)
{
//	const uint4 Misc0 = RawData1;
//	const uint4 Misc1 = RawData2;
	const uvec4 Misc0 = RawData1;
	const uvec4 Misc1 = RawData2;
	const uint  Misc2 = RawData3;

	FHierarchyNodeSlice Node;
//	Node.LODBounds				= asfloat(RawData0);
	Node.LODBounds				= uintBitsToFloat(RawData0);

//	Node.BoxBoundsCenter		= asfloat(Misc0.xyz);
	Node.BoxBoundsCenter		= uintBitsToFloat(Misc0.xyz);
//	Node.BoxBoundsExtent		= asfloat(Misc1.xyz);
	Node.BoxBoundsExtent		= uintBitsToFloat(Misc1.xyz);

//	Node.MinLODError			= f16tof32(Misc0.w);
//	Node.MaxParentLODError		= f16tof32(Misc0.w >> 16);
	vec2 unpackedData = unpackHalf2x16(Misc0.w);

	Node.MinLODError			= unpackedData.x;
	Node.MaxParentLODError		= unpackedData.y;

	Node.ChildStartReference	= Misc1.w;						// When changing this, remember to also update StoreHierarchyNodeChildStartReference
	Node.bLoaded				= (Misc1.w != 0xFFFFFFFFu);

	Node.NumChildren			= BitFieldExtractU32(Misc2, NANITE_MAX_CLUSTERS_PER_GROUP_BITS, 0);
	Node.NumPages				= BitFieldExtractU32(Misc2, NANITE_MAX_GROUP_PARTS_BITS, NANITE_MAX_CLUSTERS_PER_GROUP_BITS);
	Node.StartPageIndex			= BitFieldExtractU32(Misc2, NANITE_MAX_RESOURCE_PAGES_BITS, NANITE_MAX_CLUSTERS_PER_GROUP_BITS + NANITE_MAX_GROUP_PARTS_BITS);
	Node.bEnabled				= Misc2 != 0u;
	Node.bLeaf					= Misc2 != 0xFFFFFFFFu;

	return Node;
}

// UE5源码位置：NaniteDataDecode.ush
#define HIERARCHY_NODE_SLICE_SIZE	((4 + 4 + 4 + 1) * 4 * NANITE_MAX_BVH_NODE_FANOUT)	// 208字节

//FHierarchyNodeSlice GetHierarchyNodeSlice(ByteAddressBuffer InputBuffer, uint NodeIndex, uint ChildIndex)
FHierarchyNodeSlice GetHierarchyNodeSlice(uint NodeIndex, uint ChildIndex)
{
//	const uint BaseAddress	= NodeIndex * HIERARCHY_NODE_SLICE_SIZE;

//	const uint4 RawData0	= InputBuffer.Load4(BaseAddress + 16 * ChildIndex);
//	const uint4 RawData1	= InputBuffer.Load4(BaseAddress + (NANITE_MAX_BVH_NODE_FANOUT * 16) + 16 * ChildIndex);
//	const uint4 RawData2	= InputBuffer.Load4(BaseAddress + (NANITE_MAX_BVH_NODE_FANOUT * 32) + 16 * ChildIndex);
//	const uint  RawData3	= InputBuffer.Load( BaseAddress + (NANITE_MAX_BVH_NODE_FANOUT * 48) +  4 * ChildIndex);

	FPackedHierarchyNode packedData = BVH.m_data[NodeIndex];

	const uvec4 RawData0	= packedData.LODBounds[ChildIndex];
	const uvec4 RawData1	= packedData.Misc0[ChildIndex];
	const uvec4 RawData2	= packedData.Misc1[ChildIndex];
	const uint  RawData3	= packedData.Misc2[ChildIndex];
	
	return UnpackHierarchyNodeSlice(RawData0, RawData1, RawData2, RawData3);
}

void main()
{
	uint bvhNodeCount = 21u;

	// 获取第一个节点
	FHierarchyNodeSlice slice = GetHierarchyNodeSlice(0, 1);

	// 打印测试
	//MainAndPostNodeAndClusterBatches.m_data[0] = vec4(slice.LODBounds.xyz, slice.MinLODError);
	//MainAndPostNodeAndClusterBatches.m_data[0] = vec4(1.0, 2.0, 3.0, 1.0);

	// 单线程遍历四叉树，分层遍历
	/*uint currentNodeOffset = 0;	// 总偏移（index）
	uint currentNodeCount = 1;	// 当前层的节点数
	uint nextHierarchyNodeIndexOffset = currentNodeOffset + currentNodeCount;	// 下一层的偏移（下一层遍历时从哪个索引开始）
	uint nextHierarchyNodeCount = 0;	// 用于记录下一层的节点数
	while (true)
	{
		uint currentNodeIndex = MainAndPostNodeAndClusterBatches.m_data[currentNodeOffset];
		for (int i = 0; i < NANITE_MAX_BVH_NODE_FANOUT; ++i)
		{
			FHierarchyNodeSlice slice = GetHierarchyNodeSlice(currentNodeIndex, i);	// 获取 Child Node
			if (slice.bEnabled)
			{
				if (slice.bLeaf)	// 叶子节点
				{
					//
				}
				else
				{
					// 输出分层遍历的顺序
					MainAndPostNodeAndClusterBatches.m_data[nextHierarchyNodeIndexOffset + nextHierarchyNodeCount] = slice.ChildStartReference;
					nextHierarchyNodeCount++;
				}
			}
		}

		currentNodeCount--;

		if (currentNodeCount == 0)
		{
			if (nextHierarchyNodeCount == 0)
			{
				break;
			}
			currentNodeOffset = nextHierarchyNodeIndexOffset;
			currentNodeCount = nextHierarchyNodeCount;
			nextHierarchyNodeIndexOffset = currentNodeOffset + currentNodeCount;
			nextHierarchyNodeCount = 0;
		}
		else
		{
			currentNodeOffset++;
		}
	}*/

	// 多Pass分层遍历
	uint currentNodeOffset = CurrentIndirectWorkArgs.m_data[0];	// 总偏移（index）
	uint currentNodeCount = CurrentIndirectWorkArgs.m_data[1];	// 当前层的节点数

	uint nextHierarchyNodeIndexOffset = currentNodeOffset + currentNodeCount;	// 下一层的偏移（下一层遍历时从哪个索引开始）
	uint nextHierarchyNodeCount = 0;	// 用于记录下一层的节点数
	for (uint idx = 0; idx < currentNodeCount; ++idx)
	{
		uint currentNodeIndex = MainAndPostNodeAndClusterBatches.m_data[currentNodeOffset + idx];
		for (int i = 0; i < NANITE_MAX_BVH_NODE_FANOUT; ++i)
		{
			FHierarchyNodeSlice slice = GetHierarchyNodeSlice(currentNodeIndex, i);	// 获取 Child Node
			if (slice.bEnabled)
			{
				if (slice.bLeaf)	// 叶子节点
				{
					//
				}
				else
				{
					// 输出分层遍历的顺序
					MainAndPostNodeAndClusterBatches.m_data[nextHierarchyNodeIndexOffset + nextHierarchyNodeCount] = slice.ChildStartReference;
					nextHierarchyNodeCount++;
				}
			}
		}
	}

	NextIndirectWorkArgs.m_data[0] = nextHierarchyNodeIndexOffset;
	NextIndirectWorkArgs.m_data[1] = nextHierarchyNodeCount;
}