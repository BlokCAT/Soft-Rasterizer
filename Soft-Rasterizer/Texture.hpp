#pragma once
#include <Eigen/Dense>
#include <string>
#include <opencv2/opencv.hpp>
using namespace Eigen;
class Texture
{
public:
    std::string texture_name;
    std::string texture_path;
    cv::Mat image;  //use opencv

    // ���캯��  VB
    Texture() {};
    Texture(const std::string& pngPath)
    {
        texture_path = pngPath;
        image = cv::imread(texture_path, cv::IMREAD_COLOR);
        if (image.empty())
        {
            std::cerr << "Texture::�޷���ȡ����ͼ��" << std::endl;
        }
    };

    // ʹ��OpenCV��ȡ�������� (u, v) ��Ӧ����ɫ
    Vector3f getColorAt(const float& u, const float& v)
    {
        float x = u * (image.cols - 1);
        float y = (1.0f - v) * (image.rows - 1);
        int ix = static_cast<int>(x);
        int iy = static_cast<int>(y);

        cv::Vec3b color = image.at<cv::Vec3b>(iy, ix);
        float red = color[2] / 255.0f; // OpenCV ����ɫֵ��Χ�� [0, 255]����Ҫת��Ϊ [0, 1]
        float green = color[1] / 255.0f;
        float blue = color[0] / 255.0f;
        return Vector3f(red, green, blue);
    }
};
