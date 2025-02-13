#pragma once

#include <Eigen/Dense>
#include <vector>
#include "Mesh.hpp"
#include "Camera.hpp"
using namespace Eigen;
struct light
{
	Eigen::Vector3f position;
	Eigen::Vector3f intensity;
};
class Scene
{
public:
	Camera camera;
	vector<Mesh> objs;
	vector<light> lights;

	Matrix4f get_model_matrix(Mesh &obj);
	Matrix4f get_view_matrix();
	Matrix4f get_projection_matrix();

	void add_obj(Mesh& obj);
	void add_light(light& obj);

};