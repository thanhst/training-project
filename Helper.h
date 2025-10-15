#pragma once
#include <iostream>
#include <vector>
#include<opencv2/opencv.hpp>
#include <sstream>
#include <iomanip>
#include <string>

bool fileExists(const std::string& filename);
std::vector<std::string> listImagesInFolder(const std::string& folderPath);
cv::Mat loadImage(const std::string& imagePath);
bool saveImage(const std::string& folderPath, const std::string& imageName, const cv::Mat& image);