#include "RenderPass.h"
#include "Interface/RHI.h"
#include <Runtime/Function/Render/Interface/OpenGL/OpenGLUtil.h>
#include "Interface/Vulkan/VulkanRHI.h"
#include "RenderResource.h"
#include <Runtime/Function/Render/Interface/OpenGL/Util.h>
#include <Runtime/Function/Render/Interface/OpenGL/OpenGLRHIResource.h>

XYH::ST_VisibleNodes XYH::RenderPass::s_visibleNodes;

NAMESPACE_XYH_BEGIN

RenderPass::RenderPass(ERenderPassType renderPassType, const char* name)
	:
	m_renderPassType(renderPassType),
	m_name(name)
{
}

void RenderPass::Initialize(const ST_RenderPassInitInfo* initInfo)
{
	m_pGlobalRenderResource = &(std::static_pointer_cast<RenderResource>(m_pRenderResource)->m_globalRenderResource);
}

void RenderPass::PostInitialize()
{
}

void RenderPass::Build(uint32_t width, uint32_t height)
{
	switch (m_renderPassType)
	{
	case XYH::ERenderPassType::ERPT_GRAPHICS:
	{
		m_viewportWidth = width;
		m_viewportHeight = height;
	}
	break;
	case XYH::ERenderPassType::ERPT_COMPUTE:
	{

	}
	break;
	default:
		break;
	}
}

void RenderPass::Execute(RHIBuffer* pIndirectBuffer)
{
	if (m_renderPassType == ERenderPassType::ERPT_GRAPHICS)	// 图形绘制
	{
		if (nullptr != pIndirectBuffer)
		{
#ifdef USE_OPENGL
			SCOPED_EVENT(m_name.c_str());
			OGL_CALL(glViewport(0, 0, m_viewportWidth, m_viewportHeight));
			OGL_CALL(glFrontFace(GL_CW));
			OGL_CALL(glUseProgram(((OpenGLShader*)m_pGraphicsShader)->GetResource()));
			// UBO
			int uboSlot = 0;
			for (auto pUniformBuffer : m_uniformBuffers)
			{
				OGL_CALL(glUniformBlockBinding(((OpenGLShader*)m_pGraphicsShader)->GetResource(), pUniformBuffer->m_binding, uboSlot));
				OGL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, uboSlot++, ((OpenGLBuffer*)pUniformBuffer->m_pBuffer)->GetResource()));
			}
			// SSBO
			for (auto pStorageBuffer : m_inputBuffers)
			{
				OGL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, pStorageBuffer->m_binding, ((OpenGLBuffer*)pStorageBuffer->m_pBuffer)->GetResource()));
			}
			// 绘制
			OGL_CALL(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ((OpenGLBuffer*)pIndirectBuffer)->GetResource()));
			OGL_CALL(glDrawArraysIndirect(GL_TRIANGLES, nullptr));
			// 解绑
			for (auto pStorageBuffer : m_inputBuffers)
			{
				OGL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, pStorageBuffer->m_binding, 0));
			}
			OGL_CALL(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
			OGL_CALL(glFlush());
#endif // USE_OPENGL
		}
		else
		{

		}
	}
	else if (m_renderPassType == ERenderPassType::ERPT_COMPUTE)	// 计算
	{
#ifdef USE_OPENGL
		SCOPED_EVENT(m_name.c_str());
		OGL_CALL(glUseProgram(((OpenGLShader*)m_pComputeShader)->GetResource()));
		int uboSlot = 0;
		// UBO
		for (auto pUniformBuffer : m_uniformBuffers)
		{
			OGL_CALL(glUniformBlockBinding(((OpenGLShader*)m_pComputeShader)->GetResource(), pUniformBuffer->m_binding, uboSlot));
			OGL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, uboSlot++, ((OpenGLBuffer*)pUniformBuffer->m_pBuffer)->GetResource()));
		}
		// SSBO
		for (auto pStorageBuffer : m_inputBuffers)
		{
			OGL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, pStorageBuffer->m_binding, ((OpenGLBuffer*)pStorageBuffer->m_pBuffer)->GetResource()));
		}
		// Texture
		for (auto pImageRes : m_inputTextures)
		{
			OGL_CALL(glBindImageTexture(pImageRes->m_binding, ((OpenGLImage*)pImageRes->m_pImage)->GetResource(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
		}
		// 计算
		OGL_CALL(glDispatchCompute(m_dispatchX, m_dispatchY, m_dispatchZ));
		// 解绑
		for (auto pStorageBuffer : m_inputBuffers)
		{
			OGL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, pStorageBuffer->m_binding, 0));
		}
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glFlush();
#endif // USE_OPENGL
	}
}

void RenderPass::Draw()
{
}

RHIRenderPass* RenderPass::GetRenderPass() const
{
	return m_framebuffer.m_pRenderPass;
}

std::vector<RHIImageView*> RenderPass::GetFramebufferImageViews() const
{
	std::vector<RHIImageView*> imageViews;
	for (const auto& attch : m_framebuffer.m_attachments)
	{
		imageViews.push_back(attch.m_pView);
	}
	return imageViews;
}

std::vector<RHIDescriptorSetLayout*> RenderPass::GetDescriptorSetLayouts() const
{
	std::vector<RHIDescriptorSetLayout*> layouts;
	for (const auto& descriptor : m_descriptorInfos)
	{
		layouts.push_back(descriptor.m_pDescriptorSetLayout);
	}
	return layouts;
}

void RenderPass::SetGraphicsShader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	size_t fileSize = 0;
	unsigned char* fileContent = LoadFileContent(vertexShaderPath, fileSize);
	char* shaderCode = new char[fileSize + 1];
	memcpy(shaderCode, fileContent, fileSize);
	shaderCode[fileSize] = 0;
#ifdef USE_OPENGL
	GLuint vsshader = OpenGLUtil::CompileShader(GL_VERTEX_SHADER, shaderCode);
#endif
	delete[]shaderCode;
	delete[]fileContent;

	fileContent = LoadFileContent(fragmentShaderPath, fileSize);
	shaderCode = new char[fileSize + 1];
	memcpy(shaderCode, fileContent, fileSize);
	shaderCode[fileSize] = 0;
#ifdef USE_OPENGL
	GLuint fsshader = OpenGLUtil::CompileShader(GL_FRAGMENT_SHADER, shaderCode);
#endif
	delete[]shaderCode;
#ifdef USE_OPENGL
	GLuint shaderProgram = OpenGLUtil::CreateProgram(vsshader, fsshader);
	if (nullptr == m_pGraphicsShader)
	{
		m_pGraphicsShader = new OpenGLShader;
	}
	((OpenGLShader*)m_pGraphicsShader)->SetResource(shaderProgram);
#endif
}

void RenderPass::SetComputeShader(const char* computeShaderPath)
{
	size_t fileSize = 0;
	unsigned char* fileContent = LoadFileContent(computeShaderPath, fileSize);
	char* shaderCode = new char[fileSize + 1];
	memcpy(shaderCode, fileContent, fileSize);
	shaderCode[fileSize] = 0;

#ifdef USE_OPENGL
	GLuint shader = OpenGLUtil::CompileShader(GL_COMPUTE_SHADER, shaderCode);
	GLuint shaderProgram = OpenGLUtil::CreateProgram(shader);
	if (nullptr == m_pComputeShader)
	{
		m_pComputeShader = new OpenGLShader;
	}
	((OpenGLShader*)m_pComputeShader)->SetResource(shaderProgram);
	delete[]shaderCode;
	delete[]fileContent;
#endif // USE_OPENGL
}

void RenderPass::SetComputeImage(int binding, RHIImage* pImage, bool isOutputResource)
{
	ST_ImageResource* pImageResource = new ST_ImageResource;
	pImageResource->m_binding = binding;
	pImageResource->m_pImage = pImage;
	m_inputTextures.push_back(pImageResource);
	if (isOutputResource)	// 输出的纹理
	{
		m_outputTextures.push_back(pImageResource);
	}
}

void RenderPass::SetStorageBuffer(int binding, RHIBuffer* pBuffer, bool isOutputResource)
{
	ST_BufferResource* pBufferResource = new ST_BufferResource;
	pBufferResource->m_binding = binding;
	pBufferResource->m_pBuffer = pBuffer;
	m_inputBuffers.push_back(pBufferResource);
	if (isOutputResource)
	{
		m_outputBuffers.push_back(pBufferResource);
	}
}

void RenderPass::SetUniformBuffer(int binding, RHIBuffer* pBuffer)
{
	ST_BufferResource* pBufferResource = new ST_BufferResource;
	pBufferResource->m_binding = binding;
	pBufferResource->m_pBuffer = pBuffer;
	m_uniformBuffers.push_back(pBufferResource);
}

void RenderPass::SetComputeDispatchArgs(int x, int y, int z)
{
	m_dispatchX = x;
	m_dispatchY = y;
	m_dispatchZ = z;
}

NAMESPACE_XYH_END

