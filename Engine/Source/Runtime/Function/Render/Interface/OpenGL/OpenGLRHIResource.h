#pragma once

#include <Runtime/Function/Render/Interface/RHIStruct.h>
#include <Runtime/Function/Render/Interface/OpenGL/OpenGLRHI.h>

NAMESPACE_XYH_BEGIN

class OpenGLBuffer : public RHIBuffer
{
public:
	void SetResource(GLuint res)
	{
		m_buffer = res;
	}
	GLuint GetResource() const
	{
		return m_buffer;
	}
private:
	GLuint m_buffer;
};

class OpenGLImage : public RHIImage
{
public:
	void SetResource(GLuint res)
	{
		m_image = res;
	}
	GLuint GetResource() const
	{
		return m_image;
	}
public:
	GLenum m_format;
	GLuint m_image;
};

class OpenGLShader : public RHIShader
{
public:
	void SetResource(GLuint res)
	{
		m_shaderProgram = res;
	}
	GLuint GetResource() const
	{
		return m_shaderProgram;
	}
private:
	GLuint m_shaderProgram;
};

NAMESPACE_XYH_END