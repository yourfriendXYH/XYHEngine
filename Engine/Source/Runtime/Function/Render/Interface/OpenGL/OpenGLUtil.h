#pragma once
#include "OpenGLRHI.h"
#include "OpenGLRHIResource.h"

NAMESPACE_XYH_BEGIN

struct StaticMeshVertexData
{
	float mPosition[4];
	float mTexcoord[4];
};
struct VertexFactory {
	const char** mAttributes;
	int mAttributeCount;
	int mVertexSizeInBytes;
	VertexFactory(const char** inAttributes, int inAttributeCount, int inVertexSize) {
		mAttributes = inAttributes;
		mAttributeCount = inAttributeCount;
		mVertexSizeInBytes = inVertexSize;
	}
};
const VertexFactory* GetVertexFactory();

class FullScreenQuad
{
public:
	void Init();

	void SetVertexCount(int vertex_count);
	void SetPosition(int index, float x, float y, float z, float w = 1.0f);
	void SetTexcoord(int index, float x, float y, float z = 1.0f, float w = 1.0f);

public:
	GLenum m_primitiveType;

	StaticMeshVertexData* m_vertices;
	int m_vertexCount;

	GLuint m_VBO;
	GLuint m_VAO;

	const VertexFactory* m_pVertexFactory;
};

struct Texture2D
{
	GLuint mTexture;
	GLenum mFormat;
	int mWidth;
	int mHeight;
};


class OpenGLUtil
{
public:
	static GLenum TranslateBufferType(RHIBufferUsageFlags type);

	static GLuint CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data = nullptr);

	static void UpdataBufferObject(GLuint object, GLenum type, void* data, int size, int offset);

	static GLuint BuildVAO(GLuint inVBO, int inAttributeCount, const char** inAttributeNames, int inVertexSizeInBytes);

	static GLuint CompileShader(GLenum shaderType, const char* shaderCode);

	static GLuint CreateProgram(GLuint vsShader, GLuint fsShader);
	// 创建计算着色器
	static GLuint CreateProgram(GLuint csShader);

	static OpenGLImage* CreateTexture2D(
		unsigned char* pixelData,
		int width,
		int height,
		GLenum gpu_format = GL_RGB,
		GLenum cpu_format = GL_RGB,
		GLenum wrapMode = GL_CLAMP_TO_EDGE,
		GLenum minFilter = GL_LINEAR,
		GLenum magFilter = GL_LINEAR
	);

	static void SetObjectName(GLenum inType, GLuint inObject, const char* inName);
};

NAMESPACE_XYH_END