#include "OpenGLUtil.h"

NAMESPACE_XYH_BEGIN

static const char* attributeNames[] = { "position","texcoord" };

const VertexFactory* GetVertexFactory()
{
	static VertexFactory* sVertexFactory = nullptr;
	if (sVertexFactory == nullptr) {
		sVertexFactory = new VertexFactory(attributeNames, 2, sizeof(float) * 4 * 2);
	}
	return sVertexFactory;
}

void FullScreenQuad::Init()
{
	SetVertexCount(4);
	m_primitiveType = GL_TRIANGLE_STRIP;
	SetPosition(0, -1.0f, -1.0f, 0.0f);
	SetPosition(1, 1.0f, -1.0f, 0.0f);
	SetPosition(2, -1.0f, 1.0f, 0.0f);
	SetPosition(3, 1.0f, 1.0f, 0.0f);
	SetTexcoord(0, 0.0f, 0.0f);
	SetTexcoord(1, 1.0f, 0.0f);
	SetTexcoord(2, 0.0f, 1.0f);
	SetTexcoord(3, 1.0f, 1.0f);
	m_VBO = OpenGLUtil::CreateBufferObject(GL_ARRAY_BUFFER, sizeof(StaticMeshVertexData) * m_vertexCount, GL_DYNAMIC_DRAW, m_vertices);
	m_pVertexFactory = GetVertexFactory();
	m_VAO = OpenGLUtil::BuildVAO(m_VBO, m_pVertexFactory->mAttributeCount, m_pVertexFactory->mAttributes, m_pVertexFactory->mVertexSizeInBytes);
}

void FullScreenQuad::SetVertexCount(int vertex_count)
{
	m_vertexCount = vertex_count;
	m_vertices = new StaticMeshVertexData[vertex_count];
}

void FullScreenQuad::SetPosition(int index, float x, float y, float z, float w)
{
	m_vertices[index].mPosition[0] = x;
	m_vertices[index].mPosition[1] = y;
	m_vertices[index].mPosition[2] = z;
	m_vertices[index].mPosition[3] = w;
}

void FullScreenQuad::SetTexcoord(int index, float x, float y, float z, float w)
{
	m_vertices[index].mTexcoord[0] = x;
	m_vertices[index].mTexcoord[1] = y;
	m_vertices[index].mTexcoord[2] = z;
	m_vertices[index].mTexcoord[3] = w;
}

GLenum OpenGLUtil::TranslateBufferType(RHIBufferUsageFlags type)
{
	return GLenum();
}

GLuint OpenGLUtil::CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data)
{
	GLuint object;
	OGL_CALL(glGenBuffers(1, &object));
	OGL_CALL(glBindBuffer(bufferType, object));
	OGL_CALL(glBufferData(bufferType, size, data, usage));
	OGL_CALL(glBindBuffer(bufferType, 0));
	return object;
}

void OpenGLUtil::UpdataBufferObject(GLuint object, GLenum type, void* data, int size, int offset)
{
	OGL_CALL(glBindBuffer(type, object));
	OGL_CALL(glBufferSubData(type, offset, size, data));//cpu -> gpu
	OGL_CALL(glBindBuffer(type, 0));
}

GLuint OpenGLUtil::BuildVAO(GLuint inVBO, int inAttributeCount, const char** inAttributeNames, int inVertexSizeInBytes)
{
	int attributeComponentCount = 4;
	GLuint vao = 0;
	OGL_CALL(glGenVertexArrays(1, &vao));
	OGL_CALL(glBindVertexArray(vao));
	OGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, inVBO));
	for (int i = 0; i < inAttributeCount; i++) {
		const char* attributeName = inAttributeNames[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, attributeComponentCount, GL_FLOAT, false, inVertexSizeInBytes, (void*)(sizeof(float) * attributeComponentCount * i));
	}
	OGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	OGL_CALL(glBindVertexArray(0));
	return vao;
}

GLuint OpenGLUtil::CompileShader(GLenum shaderType, const char* shaderCode)
{
	GLuint shader = 0;
	OGL_CALL(shader = glCreateShader(shaderType));
	OGL_CALL(glShaderSource(shader, 1, &shaderCode, nullptr));
	OGL_CALL(glCompileShader(shader));
	GLint compileResult = GL_TRUE;
	OGL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult));
	if (compileResult == GL_FALSE)
	{
		char szLog[10240] = { 0 };
		GLsizei logLen = 0;
		OGL_CALL(glGetShaderInfoLog(shader, 10240, &logLen, szLog));
		printf("Compile Shader fail error log : %s \nshader code :\n%s\n", szLog, shaderCode);
		OGL_CALL(glDeleteShader(shader));
		shader = 0;
	}
	return shader;
}

GLuint OpenGLUtil::CreateProgram(GLuint vsShader, GLuint fsShader)
{
	GLuint program = 0;
	OGL_CALL(program = glCreateProgram());
	OGL_CALL(glAttachShader(program, vsShader));
	OGL_CALL(glAttachShader(program, fsShader));
	OGL_CALL(glLinkProgram(program));
	OGL_CALL(glDetachShader(program, vsShader));
	OGL_CALL(glDetachShader(program, fsShader));
	GLint nResult;
	OGL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &nResult));
	if (nResult == GL_FALSE) {
		char log[10240] = { 0 };
		GLsizei writed = 0;
		OGL_CALL(glGetProgramInfoLog(program, 10240, &writed, log));
		printf("create gpu program fail,link error : %s\n", log);
		OGL_CALL(glDeleteProgram(program));
		program = 0;
	}
	return program;
}

GLuint OpenGLUtil::CreateProgram(GLuint csShader)
{
	GLuint program = 0;
	OGL_CALL(program = glCreateProgram());
	OGL_CALL(glAttachShader(program, csShader));
	OGL_CALL(glLinkProgram(program));
	OGL_CALL(glDetachShader(program, csShader));
	GLint nResult;
	OGL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &nResult));
	if (nResult == GL_FALSE)
	{
		char log[10240] = { 0 };
		GLsizei writed = 0;
		OGL_CALL(glGetProgramInfoLog(program, 10240, &writed, log));
		printf("create gpu program fail,link error : %s\n", log);
		OGL_CALL(glDeleteProgram(program));
		program = 0;
	}
	return program;
}

OpenGLImage* OpenGLUtil::CreateTexture2D(
	unsigned char* pixelData,
	int width,
	int height,
	GLenum gpu_format,
	GLenum cpu_format,
	GLenum wrapMode,
	GLenum minFilter,
	GLenum magFilter)
{
	OpenGLImage* texture = new OpenGLImage;
	GLenum basicType = GL_UNSIGNED_BYTE;
	if (gpu_format == GL_RGBA32F) {
		basicType = GL_FLOAT;
	}
	OGL_CALL(glGenTextures(1, &texture->m_image));
	OGL_CALL(glBindTexture(GL_TEXTURE_2D, texture->m_image));
	OGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode));
	OGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode));
	OGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
	OGL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));
	OGL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, gpu_format, width, height, 0, cpu_format, basicType, pixelData));
	OGL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
	texture->m_format = gpu_format;
	texture->m_width = width;
	texture->m_height = height;
	return texture;
}

void OpenGLUtil::SetObjectName(GLenum inType, GLuint inObject, const char* inName)
{
	glObjectLabel(inType, inObject, -1, inName);
}

NAMESPACE_XYH_END

