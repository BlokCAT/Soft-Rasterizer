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
			cout << "MeshTriangl::�����ж��Ƿ�������" << endl;
			if (curMesh.MeshMaterial.map_Kd != "")
			{
				cout << "OBJ������" << endl;

				mat.ior = curMesh.MeshMaterial.Ni; //�����ò���ior
				mat.Ka = Vector3f(curMesh.MeshMaterial.Ka.X, curMesh.MeshMaterial.Ka.Y, curMesh.MeshMaterial.Ka.Z);
				tempMat->Ks = Vector3f(curMesh.MeshMaterial.Ks.X, curMesh.MeshMaterial.Ks.Y, curMesh.MeshMaterial.Ks.Z);
				tempMat->Illum = curMesh.MeshMaterial.illum;
				tempMat->Ns = curMesh.MeshMaterial.Ns;
				tempMat->texture = Texture(curMesh.MeshMaterial.map_Kd);
				m = tempMat;
				cout << "mtl����ȫ����ȡ������" << endl;
				cout << "������ʾ��ȡ������w����λ�ã�" << m->texture.texture_path << endl;
			}
			else
			{
				cout << "OBJû������" << endl;
				m->mtype = BLENDER;
				m->ior = curMesh.MeshMaterial.Ni; //�����ò���ior
				m->Ka = Vector3f(curMesh.MeshMaterial.Ka.X, curMesh.MeshMaterial.Ka.Y, curMesh.MeshMaterial.Ka.Z);
				m->Kd = Vector3f(curMesh.MeshMaterial.Kd.X, curMesh.MeshMaterial.Kd.Y, curMesh.MeshMaterial.Kd.Z);
				m->Ks = Vector3f(curMesh.MeshMaterial.Ks.X, curMesh.MeshMaterial.Ks.Y, curMesh.MeshMaterial.Ks.Z);
				m->Illum = curMesh.MeshMaterial.illum;
				m->Ns = curMesh.MeshMaterial.Ns;
				cout << "mtl����ȫ����ȡ������" << endl;
				cout << "������ʾ��ȡ������Kd��ֵ��" << m->Kd.x << endl;
			}

			array<Vector3f, 3> tempVertices;
			array<Vector3f, 3> tempVerticesNormal;
			array<Vector2f, 3> tempVerticesTexture;

			//������������,���������εĶ��㣬����ķ��ߣ������UV����
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
			cout << "���OBJģ���е���������" << triangles.size() << endl;
		}
		else
		{
			cout << "objģ�ͼ���ʧ��";
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
