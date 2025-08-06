#pragma once
#include <Common.h>
#include "RenderResourceBase.h"

NAMESPACE_XYH_BEGIN

class RenderResource : public RenderResourceBase
{
public:
	void Clear() override final;	// ������Ⱦ��Դ

	virtual void UploadGlobalRenderResource(std::shared_ptr<RHI> pRHI, const ST_LevelResourceDesc& levelResourceDesc) override final;	// �ϴ�ȫ����Ⱦ��Դ

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity renderEntity, ST_RenderMeshData mesh_data, ST_RenderMaterialData material_data) override final;	// �ϴ���Ϸ������Ⱦ��Դ

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMeshData mesh_data) override final;	// �ϴ���Ϸ������Ⱦ��Դ�����������ݣ�

	virtual void UploadGameObjectRenderResource(std::shared_ptr<RHI> rhi, RenderEntity render_entity, ST_RenderMaterialData material_data) override final; // �ϴ���Ϸ������Ⱦ��Դ�����������ݣ�

	virtual void UpdatePerFrameBuffer(std::shared_ptr<RenderScene> render_scene, std::shared_ptr<RenderCamera> camera) override final; // ����ÿ֡������

private:

};

NAMESPACE_XYH_END