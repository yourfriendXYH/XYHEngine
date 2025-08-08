#include "RenderCamera.h"

NAMESPACE_XYH_BEGIN

void RenderCamera::SetCurrentCameraType(ERenderCameraType type)
{
	std::lock_guard<std::mutex> lockGuard(m_viewMatrixMutex);	// ������ͼ���󻥳���

	m_currentCameraType = type;
}

void RenderCamera::SetMainViewMatrix(const Matrix4x4& viewMatrix, ERenderCameraType type)
{
	std::lock_guard<std::mutex> lockGuard(m_viewMatrixMutex);

	m_currentCameraType = type;

	m_viewMatrices[MAIN_VIEW_MATRIX_INDEX] = viewMatrix;	// �������ӿھ���

	// �������λ��
	Vector3 s = Vector3(viewMatrix[0][0], viewMatrix[0][1], viewMatrix[0][2]);
	Vector3 u = Vector3(viewMatrix[1][0], viewMatrix[1][1], viewMatrix[1][2]);
	Vector3 f = Vector3(-viewMatrix[2][0], -viewMatrix[2][1], -viewMatrix[2][2]);
	m_position = s * (-viewMatrix[0][3]) + u * (-viewMatrix[1][3]) + f * viewMatrix[2][3];
}

void RenderCamera::Move(Vector3 delta)
{
	m_position += delta;	// �������λ��
}

void RenderCamera::Rotate(Vector2 delta)
{
	//��x�� y����ת
	delta = Vector2(Radian(Degree(delta.x)).valueRadians(), Radian(Degree(delta.y)).valueRadians());

	// ����pitch
	float dot = m_upAxis.dotProduct(Forward());
	if ((dot < -0.99f && delta.x > 0.0f) || (dot > 0.99f && delta.x < 0.0f))
	{
		delta.x = 0.0f;	// ������x����ת
	}

	//��������ڵ�ǰ�ĺ�����ת
	//ƫ����������
	//��ֹ����
	Quaternion pitch, yaw;
	pitch.fromAngleAxis(Radian(delta.x), s_X);	// ��������ת
	yaw.fromAngleAxis(Radian(delta.y), s_Z);	// ��������ת

	m_rotation = pitch * m_rotation * yaw;	// ���������ת

	m_inverseRotation = m_rotation.conjugate();	// �����������ת
}

void RenderCamera::Zoom(float offset)
{
	// �Ŵ���С��Ұ
	m_fovX = Math::clamp(m_fovX - offset, MIN_FOV, MAX_FOV);
}

void RenderCamera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up)
{
	m_position = position;	// �������λ��
	
	// ģ����ת
	// ������ӳ�䵽����ռ䣨x, y, z��
	Vector3 forward = (target - position).normalisedCopy();	// ����ǰ����, ���ǰ����
	m_rotation = forward.getRotationTo(s_Y);	// ���������ת

	Vector3 right = forward.crossProduct(up.normalisedCopy()).normalisedCopy();	// �����ҷ���
	Vector3 orthogonalUp = right.crossProduct(forward);	// ���������Ϸ���

	Quaternion upRotation = (m_rotation * orthogonalUp).getRotationTo(s_Z);	// �����Ϸ�����ת

	m_rotation = Quaternion(upRotation) * m_rotation;

	//ģ����ת����
	//������ռ�ʸ��ӳ�䵽ģ��ʸ��
	m_inverseRotation = m_rotation.conjugate();
}

void RenderCamera::SetAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;

	// 1 / tan(fovy * 0.5) / aspect = 1 / tan(fovx * 0.5)
	// 1 / tan(fovy * 0.5) = aspect / tan(fovx * 0.5)
	// tan(fovy * 0.5) = tan(fovx * 0.5) / aspect

	// ����fovY
	m_fovY = Radian(Math::atan(Math::tan(Radian(Degree(m_fovX) * 0.5f)) / m_aspectRatio) * 2.0f).valueDegrees();
}

void RenderCamera::SetFovX(float fovX)
{
	m_fovX = fovX;
}

Matrix4x4 RenderCamera::GetViewMatrix()
{
	// ��ȡ��ͼ����
	std::lock_guard<std::mutex> lockGuard(m_viewMatrixMutex);
	auto viewMatrix = Matrix4x4::IDENTITY;
	switch (m_currentCameraType)
	{
	case XYH::ERenderCameraType::Editor:
	{
		viewMatrix = Math::makeLookAtMatrix(Position(), Position() + Forward(), Up());	// ��ȡ�༭����ͼ����
	}
	break;
	case XYH::ERenderCameraType::Motor:
	{
		viewMatrix = m_viewMatrices[MAIN_VIEW_MATRIX_INDEX];	// ��ȡ���ӿھ���
	}
	break;
	default:
		break;
	}
	return viewMatrix;
}

Matrix4x4 RenderCamera::GetPersProjMatrix() const
{
	// ����͸��ͶӰ����
	Matrix4x4 fixMat(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	Matrix4x4 projMat = fixMat * Math::makePerspectiveMatrix(Radian(Degree(m_fovY)), m_aspectRatio, m_zNear, m_zFar);

	return projMat;
}

Matrix4x4 RenderCamera::GetLookAtMatrix() const
{
	return Math::makeLookAtMatrix(Position(), Position() + Forward(), Up());
}

NAMESPACE_XYH_END

