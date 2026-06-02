#pragma once
#include "oglcontext.h"
#include <string>
#include <vector>
enum class ERenderPassType {
	ERPT_GRAPHICS,
	ERPT_COMPUTE
};
struct ShaderBufferResource {
	GLuint mBuffer;
	int mBinding;
};
struct ShaderImageResource {
	Texture2D* mTexture;
	int mBinding;
};
class RenderPass {
public:
	ERenderPassType mRenderPassType;
	union {
		GLuint mVSPShader;
		GLuint mComputeShader;
	};
	std::vector<ShaderBufferResource*> mBuffers, mOutputBuffers, mUniformBuffers;
	std::vector<ShaderImageResource*> mTextures, mOutputTextures;
	int mDispatchX, mDispatchY, mDispatchZ;
	uint32_t mViewportWidth, mViewportHeight;
	std::string mName;
	RenderPass(ERenderPassType inRPT,const char *inName):mRenderPassType(inRPT),mName(inName), mDispatchX(1), mDispatchY(1), mDispatchZ(1), mViewportWidth(0u),mViewportHeight(0u){}
	void SetVSPS(const char* inVSPath, const char* inFSPath);
	void SetUniformBufferObject(int inBindingPoint, GLuint inUBO);
	void SetCS(const char* inCSPath);
	void SetComputeImage(int inBindingPoint, Texture2D* inImage, bool inIsOutputResource=false);
	void SetSSBO(int inBindingPoint, GLuint inBuffer, bool inIsOutputResource = false);
	void SetComputeDispatchArgs(int inX,int inY,int inZ);
	void Build(uint32_t inCanvasWidth = 0u,uint32_t inCanvasHeight=0u);
	void Execute();
	void ExecuteIndirect(GLuint inIndirectBuffer);
};