#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include "Triangle.hpp"
using namespace std; 
using namespace Eigen;
int main(int argc, const char** argv)
{
	Vector3f eye(0, 0, 10);
	Triangle a;
	a.color[0].x() = 1;

	cout << a.color[0].x();


	return 0;
}