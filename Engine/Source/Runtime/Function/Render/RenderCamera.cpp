#include "RenderCamera.h"

NAMESPACE_XYH_BEGIN

void RenderCamera::SetCurrentCameraType(ERenderCameraType type)
{
	std::lock_guard<std::mutex> lockGuard(m_viewMatrixMutex);	// 锁定视图矩阵互斥量

	m_currentCameraType = type;
}

void RenderCamera::SetMainViewMatrix(const Matrix4x4& viewMatrix, ERenderCameraType type)
{
	std::lock_guard<std::mutex> lockGuard(m_viewMatrixMutex);

	m_currentCameraType = type;

	m_viewMatrices[MAIN_VIEW_MATRIX_INDEX] = viewMatrix;	// 设置主视口矩阵

	// 计算相机位置
	Vector3 s = Vector3(viewMatrix[0][0], viewMatrix[0][1], viewMatrix[0][2]);
	Vector3 u = Vector3(viewMatrix[1][0], viewMatrix[1][1], viewMatrix[1][2]);
	Vector3 f = Vector3(-viewMatrix[2][0], -viewMatrix[2][1], -viewMatrix[2][2]);
	m_position = s * (-viewMatrix[0][3]) + u * (-viewMatrix[1][3]) + f * viewMatrix[2][3];
}

void RenderCamera::Move(Vector3 delta)
{
	m_position += delta;	// 更新相机位置
}

void RenderCamera::Rotate(Vector2 delta)
{
	//绕x， y轴旋转
	delta = Vector2(Radian(Degree(delta.x)).valueRadians(), Radian(Degree(delta.y)).valueRadians());

	// 限制pitch
	float dot = m_upAxis.dotProduct(Forward());
	if ((dot < -0.99f && delta.x > 0.0f) || (dot > 0.99f && delta.x < 0.0f))
	{
		delta.x = 0.0f;	// 限制绕x轴旋转
	}

	//俯仰相对于当前的横向旋转
	//偏航独立发生
	//防止滚动
	Quaternion pitch, yaw;
	pitch.fromAngleAxis(Radian(delta.x), s_X);	// 绕右轴旋转
	yaw.fromAngleAxis(Radian(delta.y), s_Z);	// 绕上轴旋转

	m_rotation = pitch * m_rotation * yaw;	// 更新相机旋转

	m_inverseRotation = m_rotation.conjugate();	// 更新相机逆旋转
}

void RenderCamera::Zoom(float offset)
{
	// 放大缩小视野
	m_fovX = Math::clamp(m_fovX - offset, MIN_FOV, MAX_FOV);
}

void RenderCamera::LookAt(const Vector3& position, const Vector3& target, const Vector3& up)
{
	m_position = position;	// 设置相机位置
	
	// 模型旋转
	// 将向量映射到相机空间（x, y, z）
	Vector3 forward = (target - position).normalisedCopy();	// 计算前方向, 相机前方向
	m_rotation = forward.getRotationTo(s_Y);	// 计算相机旋转

	Vector3 right = forward.crossProduct(up.normalisedCopy()).normalisedCopy();	// 计算右方向
	Vector3 orthogonalUp = right.crossProduct(forward);	// 计算正交上方向

	Quaternion upRotation = (m_rotation * orthogonalUp).getRotationTo(s_Z);	// 计算上方向旋转

	m_rotation = Quaternion(upRotation) * m_rotation;

	//模型旋转的逆
	//将相机空间矢量映射到模型矢量
	m_inverseRotation = m_rotation.conjugate();
}

void RenderCamera::SetAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;

	// 1 / tan(fovy * 0.5) / aspect = 1 / tan(fovx * 0.5)
	// 1 / tan(fovy * 0.5) = aspect / tan(fovx * 0.5)
	// tan(fovy * 0.5) = tan(fovx * 0.5) / aspect

	// 计算fovY
	m_fovY = Radian(Math::atan(Math::tan(Radian(Degree(m_fovX) * 0.5f)) / m_aspectRatio) * 2.0f).valueDegrees();
}

void RenderCamera::SetFovX(float fovX)
{
	m_fovX = fovX;
}

Matrix4x4 RenderCamera::GetViewMatrix()
{
	// 获取视图矩阵
	std::lock_guard<std::mutex> lockGuard(m_viewMatrixMutex);
	auto viewMatrix = Matrix4x4::IDENTITY;
	switch (m_currentCameraType)
	{
	case XYH::ERenderCameraType::Editor:
	{
		viewMatrix = Math::makeLookAtMatrix(Position(), Position() + Forward(), Up());	// 获取编辑器视图矩阵
	}
	break;
	case XYH::ERenderCameraType::Motor:
	{
		viewMatrix = m_viewMatrices[MAIN_VIEW_MATRIX_INDEX];	// 获取主视口矩阵
	}
	break;
	default:
		break;
	}
	return viewMatrix;
}

Matrix4x4 RenderCamera::GetPersProjMatrix() const
{
	// 计算透视投影矩阵
	Matrix4x4 fixMat(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	Matrix4x4 projMat = fixMat * Math::makePerspectiveMatrix(Radian(Degree(m_fovY)), m_aspectRatio, m_zNear, m_zFar);

	return projMat;
}

Matrix4x4 RenderCamera::GetLookAtMatrix() const
{
	return Math::makeLookAtMatrix(Position(), Position() + Forward(), Up());
}

NAMESPACE_XYH_END

