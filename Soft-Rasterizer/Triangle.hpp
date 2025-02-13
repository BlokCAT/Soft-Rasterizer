#pragma once 
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <array>
using namespace Eigen;
using namespace std;

class Triangle
{
public:
	Vector4f vertex[3];
	Vector3f normal[3];
	Vector3f color[3];
	Vector2f  texUV[3];
	bool isSmoothShading;

	Triangle() {}
	void setVertex( array<Vector3f , 3> & vertexs);
	void setNormal( array<Vector3f , 3> & normals);
	void setTexUV( array<Vector2f, 3>  & texUVs);
	bool insideTriangle(float x, float y);

};