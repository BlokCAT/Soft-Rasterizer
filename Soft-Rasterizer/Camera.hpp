#pragma once
#include <Eigen/Dense>
using namespace Eigen;
class Camera
{
public:
	Vector3f eye_pos;
	Vector3f up_vector;
	Vector3f look_vector;
	float fov; //
	float zNear;
	float zFar;
	float width , height;
};