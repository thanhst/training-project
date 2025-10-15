#pragma once
#include "Circle.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "CircleManager.h"
#include "Preprocessing.h"
#include "picosha2.h"
#include <direct.h>

std::vector<std::vector<Circle>> Manager::findCircles(
	const std::vector<cv::Mat>& images,
	const std::vector<std::string>& imageNames,
	double roundnessThreshold,
	int minRadius,
	int maxRadius)
{
	std::vector<std::vector<Circle>> allCircles;
	Preprocessor processor;
	for (size_t idx = 0; idx < images.size(); ++idx) {
		const cv::Mat& img = images[idx];
		const std::string& imgName = imageNames[idx];


		std::vector<std::vector<cv::Point>> contours;
		processor.setImage(img);
		cv::Mat edgeImg = processor.processingPipeline(7, 1.5, 50, 100);
		cv::findContours(edgeImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		std::vector<Circle> circles;
		std::string id = "";
		for (auto& contour : contours) {
			double area = cv::contourArea(contour);
			double perimeter = cv::arcLength(contour, true);
			if (perimeter == 0) continue;

			double roundness = 4 * CV_PI * area / (perimeter * perimeter);
			if (roundness < roundnessThreshold) continue;

			cv::Point2f centerF;
			float radiusF;
			cv::minEnclosingCircle(contour, centerF, radiusF);

			if (radiusF < minRadius || radiusF > maxRadius) continue;

			id = picosha2::hash256_hex_string(imgName + std::to_string(centerF.x) + "_" + std::to_string(centerF.y) + std::to_string(radiusF));
			Circle c(imgName, id, cv::Point(static_cast<int>(centerF.x), static_cast<int>(centerF.y)), static_cast<int>(radiusF), contour);
			circles.push_back(c);
		}
		allCircles.push_back(circles);
	}
	return allCircles;
}

std::vector<Circle> Manager::findCircle(
	const cv::Mat& image,
	const std::string& imageName,
	double roundnessThreshold,
	int minRadius,
	int maxRadius)
{
	std::vector<std::vector<Circle>> allCircles;
	Preprocessor processor;

	std::vector<std::vector<cv::Point>> contours;
	processor.setImage(image);
	cv::Mat edgeImg = processor.processingPipeline(7, 1.5, 150, 220);
	cv::findContours(edgeImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	cv::Mat result = image.clone();
	cv::drawContours(result, contours, -1, cv::Scalar(230, 128, 64), 2);

	cv::imwrite("results/image_processing/contours.png", result);


	std::vector<Circle> circles;
	std::string id = "";
	std::vector<std::vector<cv::Point>> listContoursTrue;
	for (auto& contour : contours) {

		//cv::Point2f center;
		//float radius;
		//cv::minEnclosingCircle(contour, center, radius);

		double area = cv::contourArea(contour);
		double perimeter = cv::arcLength(contour, true);
		if (perimeter == 0) continue;

		//if ( (int)radius>=35 && (int)radius<=38) {
		//	cv::circle(image, center, (int)radius, cv::Scalar(255, 128, 32), 2);
		//}

		double roundness = 4 * CV_PI * area / (perimeter * perimeter);
		if (roundness < roundnessThreshold) continue;


		cv::Point2f centerF;
		float radiusF;
		cv::minEnclosingCircle(contour, centerF, radiusF);

		if (radiusF < minRadius || radiusF > maxRadius) continue;
		listContoursTrue.push_back(contour);

		id = picosha2::hash256_hex_string(imageName + std::to_string(centerF.x) + "_" + std::to_string(centerF.y) + std::to_string(radiusF));
		Circle c(imageName, id, cv::Point(static_cast<int>(centerF.x), static_cast<int>(centerF.y)), static_cast<int>(radiusF), contour);
		circles.push_back(c);
	}
	cv::drawContours(image, listContoursTrue, -1, cv::Scalar(255, 128, 32), 2);
	cv::imwrite("results/image_processing/detected_circle.png", image);
	return circles;
}


bool Manager::createDatabase(const std::string& dbPath) {
	sqlite3* db;
	int rc = sqlite3_open(dbPath.c_str(), &db);
	if (rc) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
		return false;
	}

	const char* sqlCreateTable = R"(
            CREATE TABLE IF NOT EXISTS circles (
                id TEXT PRIMARY KEY,
                imageName TEXT,
                centerX INTEGER,
                centerY INTEGER,
                radius INTEGER,
                contour TEXT
            );
        )";

	char* errMsg = nullptr;
	rc = sqlite3_exec(db, sqlCreateTable, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << errMsg << "\n";
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return false;
	}

	sqlite3_close(db);
	return true;
}

bool Manager::exportDBToCSV(const std::string& dbPath, const std::string& csvFileName) {
	sqlite3* db;
	if (sqlite3_open(dbPath.c_str(), &db)) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
		return false;
	}

	std::string csvFolder = "results/CSV";
	struct stat info;
	if (stat(csvFolder.c_str(), &info) != 0) {
		if (_mkdir(csvFolder.c_str()) != 0) {
			std::cerr << "Error creating directory: " << csvFolder << "\n";
			sqlite3_close(db);
			return false;
		}
	}

	std::string csvPath = csvFileName;

	std::ofstream file(csvPath);
	if (!file.is_open()) {
		std::cerr << "Can't open CSV file for writing: " << csvPath << "\n";
		sqlite3_close(db);
		return false;
	}

	file << "imageName,id,center_x,center_y,radius,contour\n";

	const char* query = "SELECT imageName,id,centerX,centerY,radius,contour FROM circles;";
	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << "\n";
		sqlite3_close(db);
		return false;
	}

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const unsigned char* imageNameText = sqlite3_column_text(stmt, 0);
		const unsigned char* idText = sqlite3_column_text(stmt, 1);
		int centerX = sqlite3_column_int(stmt, 2);
		int centerY = sqlite3_column_int(stmt, 3);
		int radius = sqlite3_column_int(stmt, 4);
		const unsigned char* contourText = sqlite3_column_text(stmt, 5);

		file << (imageNameText ? reinterpret_cast<const char*>(imageNameText) : "") << ","
			<< (idText ? reinterpret_cast<const char*>(idText) : "") << ","
			<< centerX << ","
			<< centerY << ","
			<< radius << ","
			<< "\""<< (contourText ? reinterpret_cast<const char*>(contourText) : "")<<"\""<< "\n";
	}

	if (rc != SQLITE_DONE) {
		std::cerr << "Error during sqlite3_step: " << sqlite3_errmsg(db) << "\n";
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return true;
}

bool Manager::exportTwoMaxCircles(const std::string& imageName,const std::string& dbPath, const std::string& csvPath) {
	sqlite3* db;
	if (sqlite3_open(dbPath.c_str(), &db)) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
		return false;
	}

	std::string csvFolder = "results/CSV";
	struct stat info;
	if (stat(csvFolder.c_str(), &info) != 0) {
		if (_mkdir(csvFolder.c_str()) != 0) {
			std::cerr << "Error creating directory: " << csvFolder << "\n";
			sqlite3_close(db);
			return false;
		}
	}

	std::ofstream file(csvPath);
	if (!file.is_open()) {
		std::cerr << "Can't open CSV file for writing: " << csvPath << "\n";
		sqlite3_close(db);
		return false;
	}

	file << "imageName,id,center_x,center_y,radius,contour\n";

	const char* query = "WITH top_circles AS ( "
		"SELECT * "
		"FROM circles "
		"WHERE imageName = ? "
		"ORDER BY radius DESC "
		"LIMIT 3 ) "
		"SELECT c1.*, c2.*, "
		"((c1.centerX - c2.centerX)*(c1.centerX - c2.centerX) + "
		"(c1.centerY - c2.centerY)*(c1.centerY - c2.centerY)) AS dist2 "
		"FROM top_circles c1 "
		"JOIN top_circles c2 "
		"  ON c1.id != c2.id "
		"ORDER BY dist2 / (c1.radius + c2.radius) ASC "
		"LIMIT 1;";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << "\n";
		sqlite3_close(db);
		return false;
	}

	sqlite3_bind_text(stmt, 1, imageName.c_str(), -1, SQLITE_STATIC);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const unsigned char* imageNameText = sqlite3_column_text(stmt, 0);
		const unsigned char* idText = sqlite3_column_text(stmt, 1);
		int centerX = sqlite3_column_int(stmt, 2);
		int centerY = sqlite3_column_int(stmt, 3);
		int radius = sqlite3_column_int(stmt, 4);
		const unsigned char* contourText = sqlite3_column_text(stmt, 5);

		file << (imageNameText ? reinterpret_cast<const char*>(imageNameText) : "") << ","
			<< (idText ? reinterpret_cast<const char*>(idText) : "") << ","
			<< centerX << ","
			<< centerY << ","
			<< radius << ","
			<<"\""<< (contourText ? reinterpret_cast<const char*>(contourText) : "")<<"\"" << "\n";
	}

	if (rc != SQLITE_DONE) {
		std::cerr << "Error during sqlite3_step: " << sqlite3_errmsg(db) << "\n";
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return true;
};

bool Manager::dropTableCircles(const std::string& dbPath){
	sqlite3* db;
	if (sqlite3_open(dbPath.c_str(), &db)) {
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
		return false;
	}
	const char* sqlDropTable = "DROP TABLE IF EXISTS circles;";
	char* errMsg = nullptr;
	int rc = sqlite3_exec(db, sqlDropTable, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::cerr << "SQL error: " << errMsg << "\n";
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return false;
	}
	sqlite3_close(db);
	return true;
}

