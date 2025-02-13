#pragma once
#include "Render.hpp"

void Render::render(Scene& scene)
{
	vector<Mesh>objs = scene.objs;
	if (objs.empty())
	{
		std::cerr << "场景没有模型，无法渲染！" ;
		return;
	}

	float f1 = (scene.camera.zFar - scene.camera.zNear) / 2.0;
	float f2 = (scene.camera.zFar + scene.camera.zNear) / 2.0;

	view = scene.get_view_matrix();
	projection = scene.get_projection_matrix();

	//遍历每一个Mesh物体
	for (int i = 0; i < objs.size(); i++)
	{
		model = scene.get_model_matrix(objs[i]);

		//计算这个物体的MVP矩阵
		Matrix4f mvp = projection * view * model;
		
		//遍历物体的所有三角形
		for (auto t : objs[i].triangles)
		{
			Triangle newtri = t;

			/*得到摄像机坐标系下这个三角形的点坐标vertex_on_camera，只使用MV变换，没有投影变换，
			所以这时候空间中物体的相对位置不变，方便后面着色使用，为什么这样？
			因为着色的话，物体之间的相对位置不可以发生变化，不然着色的时候光线不对*/
			array<Eigen::Vector4f, 3> vertex_on_camera
			{
				(view * model * t.vertex[0]),
				(view * model * t.vertex[1]),
				(view * model * t.vertex[2])
			};
			array<Eigen::Vector3f, 3> camera_space_pos; //方便着色使用，先存进去，后面备用
			transform(vertex_on_camera.begin(), vertex_on_camera.end(), camera_space_pos.begin(), [](auto& v) 
			{
				return v.template head<3>();  //只存了前面三个xyz
			});
		

			//计算投影坐标系下的3个点的xyz坐标,记在v里面
			Vector4f v[] = {
					mvp * t.vertex[0],
					mvp * t.vertex[1],
					mvp * t.vertex[2]
			};
			

			// 齐次坐标化，注意此时并未
			// 处理齐次坐标的w维，因为经过了MVP变换后，w坐标
			// 记录了原本的z值，这对于后续进行插值十分有用
			for (auto& vec : v) {
				vec.x() /= vec.w();
				vec.y() /= vec.w();
				vec.z() /= vec.w();
			}

			//计算摄像机坐标系下的法向量,计算方式不是直接乘上view * model 这么简单
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

				//把z还原成投影变换之前的,就是摄像机坐标系下的
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
	//取出三角形的三个顶点坐标 ，是四维度的xyzw
	array<Vector4f, 3> v = { tri.vertex[0] , tri.vertex[1] , tri.vertex[2] };
	int min_x = std::min(v[0].x(), std::min(v[1].x(), v[2].x()));
	int min_y = std::min(v[0].y(), std::min(v[1].y(), v[2].y()));
	int max_x = std::max(v[0].x(), std::max(v[1].x(), v[2].x()));
	int max_y = std::max(v[0].y(), std::max(v[1].y(), v[2].y()));

	/* 光栅坐标
			   y
				↑
				┃
				┗━━→ x
	*/
	for (int x = min_x; x <= max_x; x++)
		for (int y = min_y; y <= max_y; y++)
			if (tri.insideTriangle(x + 0.5, y + 0.5))
			{
				//如果像素在屏幕空间的三角形里面，就差值计算点在三角形里面的权重
				std:: tuple<float, float, float> tmp = computeBarycentric2D(x + 0.5, y + 0.5, tri.vertex);
				float a, b, c;
				std::tie(a, b, c) = tmp;


				/* 假设二维屏幕空间中小三角形三个顶点的深度值分别为 Z'1, Z'2, Z'3，
				其内部一点的重心坐标为 α', β', γ'，对应深度值为 Z'；
				而三维空间三个顶点的深度值分别为 Z1, Z2, Z3，重心坐标为 α, β, γ，对应深度值为 Z。
				那  Z = 1 / ( (a' / Z1) + (b' / Z2) + (c' / Z3) )
				 由于目前w坐标记录了原本的z值， 所以代码使用的是Z = 1.0 / (a / v[0].w() + b / v[1].w() + c / v[2].w());
				且有任意的 I = ( (α' / Z1) * I1 + (β' / Z1) * I2 + (γ' / Z3) * I3 ) * Z ⑩式
				最后zp*=Z是⑩式的I1  I2  I3 代入需要修正的Z属性的结果
				zp的计算原理详见：https://blog.csdn.net/Q_pril/article/details/123598746 的1.3.2部分*/
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