#pragma once 
#ifndef PathTracing_MESH_H
#define PathTracing_MESH_H

#include "Triangle.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "OBJ_Loader.hpp"
#include <array>
using namespace Eigen;
enum Pivot
{
	x, y, z
};

struct Transformation
{
	Vector3f T;
	Vector3f S;
	float angle;
	Pivot p;
};
 
class Mesh
{
public:
	Material material;
	Texture texture;
	vector<Triangle> triangles;
	Transformation transformation;

	//second ways with OBJ MTL
	Mesh(const string& path )
	{

		objl::Loader Loader;

		bool loadout = Loader.LoadFile(path);
		if (loadout)
		{
			objl::Mesh curMesh = Loader.LoadedMeshes[0];
			cout << "MeshTriangl::正在判断是否有纹理" << endl;
			if (curMesh.MeshMaterial.map_Kd != "")
			{
				cout << "OBJ有纹理" << endl;

				mat.ior = curMesh.MeshMaterial.Ni; //反射用不到ior
				mat.Ka = Vector3f(curMesh.MeshMaterial.Ka.X, curMesh.MeshMaterial.Ka.Y, curMesh.MeshMaterial.Ka.Z);
				tempMat->Ks = Vector3f(curMesh.MeshMaterial.Ks.X, curMesh.MeshMaterial.Ks.Y, curMesh.MeshMaterial.Ks.Z);
				tempMat->Illum = curMesh.MeshMaterial.illum;
				tempMat->Ns = curMesh.MeshMaterial.Ns;
				tempMat->texture = Texture(curMesh.MeshMaterial.map_Kd);
				m = tempMat;
				cout << "mtl材质全部读取出来了" << endl;
				cout << "正在显示读取出来的w纹理位置：" << m->texture.texture_path << endl;
			}
			else
			{
				cout << "OBJ没有纹理" << endl;
				m->mtype = BLENDER;
				m->ior = curMesh.MeshMaterial.Ni; //反射用不到ior
				m->Ka = Vector3f(curMesh.MeshMaterial.Ka.X, curMesh.MeshMaterial.Ka.Y, curMesh.MeshMaterial.Ka.Z);
				m->Kd = Vector3f(curMesh.MeshMaterial.Kd.X, curMesh.MeshMaterial.Kd.Y, curMesh.MeshMaterial.Kd.Z);
				m->Ks = Vector3f(curMesh.MeshMaterial.Ks.X, curMesh.MeshMaterial.Ks.Y, curMesh.MeshMaterial.Ks.Z);
				m->Illum = curMesh.MeshMaterial.illum;
				m->Ns = curMesh.MeshMaterial.Ns;
				cout << "mtl材质全部读取出来了" << endl;
				cout << "正在显示读取出来的Kd的值：" << m->Kd.x << endl;
			}

			array<Vector3f, 3> tempVertices;
			array<Vector3f, 3> tempVerticesNormal;
			array<Vector2f, 3> tempVerticesTexture;

			//遍历所有索引,设置三角形的顶点，顶点的法线，顶点的UV坐标
			for (int i = 0; i < curMesh.Indices.size(); i += 3)
			{
				for (int k = 0; k < 3; k++)
				{
					tempVertices[k] = Vector3f(curMesh.Vertices[curMesh.Indices[i + k]].Position.X,
						curMesh.Vertices[curMesh.Indices[i + k]].Position.Z,
						curMesh.Vertices[curMesh.Indices[i + k]].Position.Y);
					tempVerticesNormal[k] = Vector3f(curMesh.Vertices[curMesh.Indices[i + k]].Normal.X,
						curMesh.Vertices[curMesh.Indices[i + k]].Normal.Z,
						curMesh.Vertices[curMesh.Indices[i + k]].Normal.Y);
					tempVerticesTexture[k] = Vector2f(curMesh.Vertices[curMesh.Indices[i + k]].TextureCoordinate.X,
						curMesh.Vertices[curMesh.Indices[i + k]].TextureCoordinate.Y);
				}
				Triangle temp;
				temp.setVertex(tempVertices);
				temp.setNormal(tempVerticesNormal);
				temp.setTexUV(tempVerticesTexture);		
				triangles.push_back(temp);
			}
			cout << "这个OBJ模型有的面数量：" << triangles.size() << endl;
		}
		else
		{
			cout << "obj模型加载失败";
		}
	}

	void move_obj(Vector3f &_T)
	{
		transformation.T = _T;
	}
	void scale_obj(Vector3f& _S)
	{
		transformation.S = _S;
	}
	void rotate_obj(float _angle, Pivot _p)
	{
		transformation.angle = _angle;
		transformation.p = _p;
	}
};


#endif // PathTracing_MESH_H
