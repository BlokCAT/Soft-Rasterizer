#pragma once
#include "Scene.hpp"
float MY_PI = 3.141592658;

Matrix4f Scene::get_model_matrix(Mesh& obj)
{
	//rotation
	Eigen::Matrix4f rotation;
	float angle = obj.transformation.angle;
	angle = angle * MY_PI / 180.f;
	switch (obj.transformation.p)
	{   
	case x:
	{
		rotation << 1 , 0, sin(angle), 0,
				   0, cos(angle), -sin(angle), 0,
					0 ,sin(angle), cos(angle), 0,
					 0, 0, 0, 1;
		 break;
	}
	case y:
	{
		rotation << cos(angle), 0, sin(angle), 0,
			0, 1, 0, 0,
			-sin(angle), 0, cos(angle), 0,
			0, 0, 0, 1;
		break;
	}
	case z:
	{
		rotation << cos(angle), -sin(angle), 0,0,
			sin(angle), cos(angle), 0,0,
			0, 0, 1, 0,        
			0, 0, 0, 1;
		break;
	}
	default:
		break;
	}
   
	//scale
	Eigen::Matrix4f scale;
	scale << obj.transformation.S.x(), 0, 0, 0,
			  0, obj.transformation.S.y(), 0, 0,
			  0, 0, obj.transformation.S.z(), 0,
			  0, 0, 0, 1;

	//translate
	Eigen::Matrix4f translate;
	translate << 1, 0, 0, obj.transformation.T.x(),
			0, 1, 0, obj.transformation.T.y(),
			0, 0, 1, obj.transformation.T.z(),
			0, 0, 0, 1;

	return translate * rotation * scale;
}
Matrix4f Scene::get_view_matrix()
{
	Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

	Vector3f eye_pos = camera.eye_pos;
	Vector3f up = camera.up_vector;
	Vector3f lookAt = camera.look_vector;
	Vector3f e = lookAt.cross(up);

	Eigen::Matrix4f translate;
	Eigen::Matrix4f rotate;

	translate << 1, 0, 0, -eye_pos[0],
		0, 1, 0, -eye_pos[1],
		0, 0, 1, -eye_pos[2],
		0, 0, 0, 1;

	lookAt = lookAt * -1;
	rotate << e.x(), e.y(), e.z(), 0,
		up.x(), up.y(), up.z(), 0,
		lookAt.x(), lookAt.y(), lookAt.z(), 0,
		0, 0, 0, 1;

	view = rotate * translate * view;

	return view;
}
Matrix4f Scene::get_projection_matrix()
{
	Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

	Eigen::Matrix4f pto = Eigen::Matrix4f::Identity();
	pto << camera.zNear, 0, 0, 0,
		0, camera.zNear, 0, 0,
		0, 0, (camera.zNear + camera.zFar), (-1 * camera.zFar * camera.zNear),
		0, 0, 1, 0;

	/*              Y
					  ↑
					  │
					  └──→ X
					╱
				Z ↙
	*/
	//摄像机永远看向-z
	float halfAngle =(camera.fov * (MY_PI / 180.0f)) / 2;
	float top = camera.zNear * tan(halfAngle);  //y 轴
	float right = top * 1; //对应x轴
	float left = -right;
	float bottom = -top;

	Eigen::Matrix4f m_s = Eigen::Matrix4f::Identity();
	m_s << 2 / (right - left), 0, 0, 0,
		0, 2 / (top - bottom), 0, 0,
		0, 0, 2 / (camera.zNear - camera.zFar), 0,
		0, 0, 0, 1;

	Eigen::Matrix4f m_t = Eigen::Matrix4f::Identity();
	m_t << 1, 0, 0, -(right + left) / 2,
		0, 1, 0, -(top + bottom) / 2,
		0, 0, 1, -(camera.zFar + camera.zNear) / 2,
		0, 0, 0, 1;

	projection = m_s * m_t * pto * projection;//先挤压成方阵，用的pto，然后在正投影，用 m_s * m_t
	return projection;
}

void Scene::add_obj(Mesh& obj)
{
	objs.push_back(obj);
}
void Scene::add_light(light& light)
{
	lights.push_back(light);
}