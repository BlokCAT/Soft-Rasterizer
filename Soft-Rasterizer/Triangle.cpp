#pragma once
#include"Triangle.hpp"


void Triangle::setVertex( array<Vector3f, 3>& vertexs)
{
	vertex[0] = vertexs[0];
	vertex[1] = vertexs[1];
	vertex[2] = vertexs[2];
	for (int i = 0; i < 3; i++)
		vertex[i].w() = 1; // point
}

void Triangle::setNormal( array<Vector3f, 3>& normals)
{
	normal[0] = normals[0];
	normal[1] = normals[1];
	normal[2] = normals[2];
}

void Triangle::setTexUV( array<Vector2f, 3>& texUVs)
{
	texUV[0] = texUVs[0];
	texUV[1] = texUVs[1];
	texUV[2] = texUVs[2];
}

bool Triangle::insideTriangle(float x, float y)
{
	Vector3f v[3];
	for (int i = 0; i < 3; i++)
		v[i] = { this->vertex[i].x(), this->vertex[i].y(), 1.0 };
	Vector3f f0, f1, f2;
	f0 = v[1].cross(v[0]);
	f1 = v[2].cross(v[1]);
	f2 = v[0].cross(v[2]);
	Vector3f p(x, y, 1.);
	if ((p.dot(f0) * f0.dot(v[2]) > 0) && (p.dot(f1) * f1.dot(v[0]) > 0) && (p.dot(f2) * f2.dot(v[1]) > 0))
		return true;
	return false;



	return false;
}
