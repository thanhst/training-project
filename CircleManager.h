#pragma once
#pragma once
#include "Circle.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "sqlite3.h"
#include <fstream>
#include <sys/stat.h>
class Manager {
public:
	Manager() {}
	~Manager() {}

	std::vector<std::vector<Circle>> findCircles(
		const std::vector<cv::Mat>& images,
		const std::vector<std::string>& imageNames,
		double roundnessThreshold = 0.8,
		int minRadius = 10,
		int maxRadius = 100);
	std::vector<Circle> findCircle(
		const cv::Mat& image,
		const std::string& imageName,
		double roundnessThreshold,
		int minRadius,
		int maxRadius);

	bool createDatabase(const std::string& dbPath);
	bool dropTableCircles(const std::string& dbPath);

	static bool exportDBToCSV(const std::string& dbPath, const std::string& csvPath);
	static bool exportTwoMaxCircles(const std::string& imageName,const std::string& dbPath, const std::string& csvPath);
};
