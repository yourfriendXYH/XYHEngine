#pragma once
#include <windows.h>
#include "GL/glew.h"
#ifdef _WIN32
#include "GL/wglew.h"
#endif
namespace Alice {
	void InitExtensions();
	bool SupportExtension(const char* inExtensionName);
	const char* LikelyToSupportExtension(const char* inExtension);
}
struct GlobalConstants {
	union {
		struct {
			float mProjectionMatrix[16];
			float mViewMatrix[16];
			float mModelMatrix[16];
			unsigned int mMisc0[4];
			float mCameraPositionWS[4];
			float mViewDirectionWS[4];
		};
		float mData[1024];
	};
	void SetProjectionMatrix(const float* inMatrix);
	void SetViewMatrix(float* inMatrix);
	void SetModelMatrix(const float* inMatrix);
	void SetMisc0(unsigned int x, unsigned int y, unsigned int z, unsigned int w);
	void SetCameraPositionWS(float inX, float inY, float inZ, float inW = 0.0f);
	void SetCameraViewDirectionWS(float inX, float inY, float inZ, float inW = 0.0f);
};
struct Texture2D {
	GLuint mTexture;
	GLenum mFormat;
	int mWidth;
	int mHeight;
};
GLuint CompileShader(GLenum shaderType, const char* shaderCode);
GLuint CreateProgram(GLuint vsShader, GLuint fsShader);
GLuint CreateProgram(GLuint csShader);
GLuint CreateBufferObject(GLenum bufferType, GLsizeiptr size, GLenum usage, void* data = nullptr);
void UpdateBufferObject(GLuint object, GLenum type, void* data, int size, int offset);
Texture2D* CreateTexture2D(unsigned char* pixelData, int width, int height, GLenum gpu_format = GL_RGB, GLenum cpu_format = GL_RGB,
	GLenum wrapMode = GL_CLAMP_TO_EDGE, GLenum minFilter = GL_LINEAR, GLenum magFilter = GL_LINEAR);
void SetObjectName(GLenum inType,GLuint inObject, const char* inName);
struct ScopedEvent {
	ScopedEvent(LPCSTR inName){
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, inName);
	}
	~ScopedEvent() {
		glPopDebugGroup();
	}
};
#define EVENT_VAR_INNER(inName, line) _scopedEvent_##line(inName)
#define EventVar(inName,n) EVENT_VAR_INNER(inName, n)
#define SCOPED_EVENT(inName) \
        ScopedEvent EventVar(inName,__LINE__)

void CheckLastOpenGLError(const char* prefix, const char* file, long line, const char* operation);
#define GLAssert(x) 	{ CheckLastOpenGLError (NULL,__FILE__, __LINE__,#x); }
#define OGL_CALL(x) do { x; GLAssert(x); } while(0)