#pragma once
#include <Eigen/Dense>
#include "Scene.hpp"
using namespace Eigen;
using namespace std;
class Render
{
public:
    Matrix4f model;
    Matrix4f view;
    Matrix4f projection;

    vector<float> depth_buf; //记录深度缓存的
    vector<Vector3f> frame_buf; //存的是最后的颜色，最后需要显示在屏幕上的

    float w;
    float h;
    Render(float _w , float _h): w(_w) , h(_h){}

    void render( Scene &scene);
    void rasterize_triangle(Triangle &tri , array<Eigen::Vector3f, 3> &viewspace_pos);
    auto to_vec4(const Eigen::Vector3f& v3, float w);
    tuple<float, float, float> computeBarycentric2D(float x, float y, Vector4f* v);
    int get_index(int x, int y);
    Vector2f interpolate(float a, float b, float c, Vector2f *vert);
    Vector3f interpolate(float a, float b, float c, Vector3f *vert);

};