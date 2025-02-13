#pragma once
#include "Render.hpp"

void Render::render(Scene& scene)
{
	vector<Mesh>objs = scene.objs;
	if (objs.empty())
	{
		std::cerr << "����û��ģ�ͣ��޷���Ⱦ��" ;
		return;
	}

	float f1 = (scene.camera.zFar - scene.camera.zNear) / 2.0;
	float f2 = (scene.camera.zFar + scene.camera.zNear) / 2.0;

	view = scene.get_view_matrix();
	projection = scene.get_projection_matrix();

	//����ÿһ��Mesh����
	for (int i = 0; i < objs.size(); i++)
	{
		model = scene.get_model_matrix(objs[i]);

		//������������MVP����
		Matrix4f mvp = projection * view * model;
		
		//�������������������
		for (auto t : objs[i].triangles)
		{
			Triangle newtri = t;

			/*�õ����������ϵ����������εĵ�����vertex_on_camera��ֻʹ��MV�任��û��ͶӰ�任��
			������ʱ��ռ�����������λ�ò��䣬���������ɫʹ�ã�Ϊʲô������
			��Ϊ��ɫ�Ļ�������֮������λ�ò����Է����仯����Ȼ��ɫ��ʱ����߲���*/
			array<Eigen::Vector4f, 3> vertex_on_camera
			{
				(view * model * t.vertex[0]),
				(view * model * t.vertex[1]),
				(view * model * t.vertex[2])
			};
			array<Eigen::Vector3f, 3> camera_space_pos; //������ɫʹ�ã��ȴ��ȥ�����汸��
			transform(vertex_on_camera.begin(), vertex_on_camera.end(), camera_space_pos.begin(), [](auto& v) 
			{
				return v.template head<3>();  //ֻ����ǰ������xyz
			});
		

			//����ͶӰ����ϵ�µ�3�����xyz����,����v����
			Vector4f v[] = {
					mvp * t.vertex[0],
					mvp * t.vertex[1],
					mvp * t.vertex[2]
			};
			

			// ������껯��ע���ʱ��δ
			// ������������wά����Ϊ������MVP�任��w����
			// ��¼��ԭ����zֵ������ں������в�ֵʮ������
			for (auto& vec : v) {
				vec.x() /= vec.w();
				vec.y() /= vec.w();
				vec.z() /= vec.w();
			}

			//�������������ϵ�µķ�����,���㷽ʽ����ֱ�ӳ���view * model ��ô��
			Matrix4f inv_trans = (view * model).inverse().transpose();
			Vector4f n[] = 
			{
				inv_trans * to_vec4(t.normal[0], 0.0f),
				inv_trans * to_vec4(t.normal[1], 0.0f),
				inv_trans * to_vec4(t.normal[2], 0.0f)
			};
			
			for (auto& vert : v)
			{
				vert.x() = 0.5 * scene.camera.width * (vert.x() + 1.0);
				vert.y() = 0.5 * scene.camera.height * (vert.y() + 1.0);

				//��z��ԭ��ͶӰ�任֮ǰ��,�������������ϵ�µ�
				vert.z() = vert.z() * f1 + f2;
			}

			for (int i = 0; i < 3; i++)
			{
				newtri.vertex[i] = v[i];
				newtri.normal[i] = n[i].head<3>();
				newtri.color[i] = Vector3f(100, 0, 100);
			}
			rasterize_triangle(newtri, camera_space_pos);
		}
	}
}

void Render::rasterize_triangle(Triangle& tri, array<Eigen::Vector3f, 3> &viewspace_pos)
{
	//ȡ�������ε������������� ������ά�ȵ�xyzw
	array<Vector4f, 3> v = { tri.vertex[0] , tri.vertex[1] , tri.vertex[2] };
	int min_x = std::min(v[0].x(), std::min(v[1].x(), v[2].x()));
	int min_y = std::min(v[0].y(), std::min(v[1].y(), v[2].y()));
	int max_x = std::max(v[0].x(), std::max(v[1].x(), v[2].x()));
	int max_y = std::max(v[0].y(), std::max(v[1].y(), v[2].y()));

	/* ��դ����
			   y
				��
				��
				�������� x
	*/
	for (int x = min_x; x <= max_x; x++)
		for (int y = min_y; y <= max_y; y++)
			if (tri.insideTriangle(x + 0.5, y + 0.5))
			{
				//�����������Ļ�ռ�����������棬�Ͳ�ֵ������������������Ȩ��
				std:: tuple<float, float, float> tmp = computeBarycentric2D(x + 0.5, y + 0.5, tri.vertex);
				float a, b, c;
				std::tie(a, b, c) = tmp;


				/* �����ά��Ļ�ռ���С������������������ֵ�ֱ�Ϊ Z'1, Z'2, Z'3��
				���ڲ�һ�����������Ϊ ��', ��', ��'����Ӧ���ֵΪ Z'��
				����ά�ռ�������������ֵ�ֱ�Ϊ Z1, Z2, Z3����������Ϊ ��, ��, �ã���Ӧ���ֵΪ Z��
				��  Z = 1 / ( (a' / Z1) + (b' / Z2) + (c' / Z3) )
				 ����Ŀǰw�����¼��ԭ����zֵ�� ���Դ���ʹ�õ���Z = 1.0 / (a / v[0].w() + b / v[1].w() + c / v[2].w());
				��������� I = ( (��' / Z1) * I1 + (��' / Z1) * I2 + (��' / Z3) * I3 ) * Z ��ʽ
				���zp*=Z�Ǣ�ʽ��I1  I2  I3 ������Ҫ������Z���ԵĽ��
				zp�ļ���ԭ�������https://blog.csdn.net/Q_pril/article/details/123598746 ��1.3.2����*/
				float Z = 1.0 / (a / v[0].w() + b / v[1].w() + c / v[2].w());  
				float zp = a * v[0].z() / v[0].w() + b * v[1].z() / v[1].w() + c * v[2].z() / v[2].w();
				zp *= Z;
				if (depth_buf[get_index(x, y)] > zp)
				{
					depth_buf[get_index(x, y)] = zp;
					Vector3f shader_viewspace_pos[] ={viewspace_pos[0], viewspace_pos[1] , viewspace_pos[2]};
					Vector3f interpolated_color = interpolate(a, b, c, tri.color);
					Vector3f interpolated_normal = interpolate(a, b, c, tri.normal);
					Vector2f interpolated_uv = interpolate(a, b, c, tri.texUV);
					Vector3f interpolated_shadingcoords = interpolate(a, b, c, shader_viewspace_pos);


				}
			}



}

auto Render::to_vec4(const Eigen::Vector3f& v3, float w )
{
	return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

int Render::get_index(int x, int y)
{
	return (h - y) * w + x;
}


tuple<float, float, float>Render::computeBarycentric2D(float x, float y,  Vector4f *v) {
	 float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
	 float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
	 float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) / (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
	 return { c1,c2,c3 };
 }

Vector2f Render::interpolate(float a, float b, float c, Vector2f *vert)
{
	return (a * vert[0] + b * vert[1] + c * vert[2]);
}
Vector3f Render::interpolate(float a, float b, float c, Vector3f *vert)
{
	return (a * vert[0] + b * vert[1] + c * vert[2]);
}