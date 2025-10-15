#pragma once
#include<opencv2/opencv.hpp>
#include<iostream>
class Circle {
public:
	std::string imageName;
	cv::Point center;
	int radius;
	std::vector<cv::Point> contour;
	std::string id;
	cv::Mat image;

	Circle(const std::string& imageName, std::string& _id, cv::Point _center, int _radius, std::vector<cv::Point> _contour = {});
	Circle();
	void draw(cv::Mat& img, cv::Scalar color = cv::Scalar(0, 255, 0));
	void setImage(const cv::Mat& img) { image = img; }
	void saveResult(const std::string& folderPath) const;

private:
	void saveToCSV(const std::string& folderPath) const;
	bool saveToDB(const std::string& dbPath) const;
	void saveImageMat(const std::string& folderPath) const;
};