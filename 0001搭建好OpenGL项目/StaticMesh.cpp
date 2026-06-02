#include "StaticMesh.h"
#include "utils.h"
static const char* attributeNames[] = { "position","texcoord" };
const VertexFactory* GetVertexFactory() {
	static VertexFactory* sVertexFactory = nullptr;
	if (sVertexFactory == nullptr) {
		sVertexFactory = new VertexFactory(attributeNames, 2, sizeof(float) * 4 * 2);
	}
	return sVertexFactory;
}
StaticMesh::StaticMesh() {
	mbDrawWidthIBO = false;
	mPrimitiveType = GL_TRIANGLES;
}
void StaticMesh::SetVertexCount(int vertex_count) {
	mVertexCount = vertex_count;
	mVertices = new StaticMeshVertexData[vertex_count];
}
void StaticMesh::SetPosition(int index, float x, float y, float z, float w) {
	mVertices[index].mPosition[0] = x;
	mVertices[index].mPosition[1] = y;
	mVertices[index].mPosition[2] = z;
	mVertices[index].mPosition[3] = w;
}
void StaticMesh::SetTexcoord(int index, float x, float y, float z, float w) {
	mVertices[index].mTexcoord[0] = x;
	mVertices[index].mTexcoord[1] = y;
	mVertices[index].mTexcoord[2] = z;
	mVertices[index].mTexcoord[3] = w;
}
void StaticMesh::Submit() {
	UpdateBufferObject(mVBO, GL_ARRAY_BUFFER, mVertices, sizeof(StaticMeshVertexData)*mVertexCount, 0);
}
void StaticMesh::Draw() {
	OGL_CALL(glDrawArrays(mPrimitiveType, 0, mVertexCount));
}