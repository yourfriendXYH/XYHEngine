#version 450
#extension GL_NV_shader_atomic_int64 : enable
#extension GL_ARB_gpu_shader_int64 : enable


layout(binding = 3, std430)buffer VisBuffer64
{
	uint64_t mData[];
}VisBuffer;

layout(location = 0)flat in uvec4 v_cluasterIndex;

void main()
{
	ivec2 texcoord = ivec2(gl_FragCoord.xy);

	int pixelIndex = 1280 * texcoord.y + texcoord.x;	// 横向遍历

	float depthValue = gl_FragCoord.z;
	uint64_t uintDepthValue = floatBitsToUint(depthValue);
	uint64_t pixelValue64 = uintDepthValue << 32 | uint64_t(v_cluasterIndex.x);

	atomicMin(VisBuffer.mData[pixelIndex], pixelValue64);	// 深度测试
	//VisBuffer.mData[pixelIndex] = 100ul;
};