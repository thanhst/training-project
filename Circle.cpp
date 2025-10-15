#include "Circle.h"
#include <fstream>
#include <sys/stat.h>
#include "Helper.h"
#include <direct.h>
#include "sqlite3.h"

Circle::Circle(const std::string& imageName, std::string& _id, cv::Point _center, int _radius, std::vector<cv::Point> _contour)
	: imageName(imageName), id(_id), center(_center), radius(_radius), contour(_contour) {
}
Circle::Circle() {};

void Circle::draw(cv::Mat& img, cv::Scalar color) {
    cv::circle(img, center, radius, color, 2);
    cv::circle(img, center, 2, cv::Scalar(0, 0, 255), 3);
    if (!contour.empty())
        cv::drawContours(img, std::vector<std::vector<cv::Point>>{contour}, -1, color, 1);
}

void Circle::saveResult(const std::string& folderPath) const {
	struct stat info;
	if (stat(folderPath.c_str(), &info) != 0) {
		if (_mkdir(folderPath.c_str()) != 0) {
			std::cerr << "Error creating directory: " << folderPath << "\n";
			return;
		}
	}
    //saveToCSV(folderPath);
    saveToDB("circles.db");
    //saveImageMat(folderPath);
}


void Circle::saveToCSV(const std::string& folderPath) const {
    std::string csvPath = folderPath + "/CSV";
    struct stat info;
    if (stat(csvPath.c_str(), &info) != 0) {
        if (_mkdir(csvPath.c_str()) != 0) {
            std::cerr << "Error creating directory: " << csvPath << "\n";
            return;
        }
    }
    std::string filePath = csvPath + "/" + imageName + "_result.csv";
    bool writeHeader = !fileExists(filePath);

    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open())
    {
		std::cerr << "Failed to open file for writing: " << filePath << "\n";
        return;
    }
    if (writeHeader)
        file << "imageName,id,center_x,center_y,radius,contour\n";

    file << imageName << "," << id << "," << center.x << "," << center.y << "," << radius << ",";
    file << "\"[";
    for (size_t i = 0; i < contour.size(); ++i) {
        file << "[" << contour[i].x << "," << contour[i].y << "]";
        if (i != contour.size() - 1) file << ",";
    }
    file << "]\"\n";
}

void Circle::saveImageMat(const std::string& folderPath) const {
    if (image.empty())
    {
		std::cerr << "Image data is empty, cannot save image.\n";
        return;
    };
    std::string imageFolderPath = folderPath + "/Image";
    struct stat info;
    if (stat(imageFolderPath.c_str(), &info) != 0) {
        if (_mkdir(imageFolderPath.c_str()) != 0) {
            std::cerr << "Error creating directory: " << imageFolderPath << "\n";
            return;
        }
    }
    size_t dotPos = imageName.find_last_of('.');
    std::string stem = (dotPos == std::string::npos) ? imageName : imageName.substr(0, dotPos);
    std::string filePath = imageFolderPath + "/" + stem + "_result.png";
    cv::imwrite(filePath, image);
}

bool Circle::saveToDB(const std::string& dbPath) const {
    sqlite3* db;
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    std::string contourStr = "[";
    for (size_t i = 0; i < contour.size(); ++i) {
        contourStr += "(" + std::to_string(contour[i].x) + "," + std::to_string(contour[i].y) + ")";
        if (i != contour.size() - 1) contourStr += ",";
    }
    contourStr += "]";

    // UPSERT: nếu id tồn tại thì UPDATE, nếu chưa tồn tại thì INSERT
    std::string sqlInsert =
        "INSERT INTO circles (id, imageName, centerX, centerY, radius, contour) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "imageName=excluded.imageName, "
        "centerX=excluded.centerX, "
        "centerY=excluded.centerY, "
        "radius=excluded.radius, "
        "contour=excluded.contour;";

    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sqlInsert.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, imageName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, center.x);
    sqlite3_bind_int(stmt, 4, center.y);
    sqlite3_bind_int(stmt, 5, radius);
    sqlite3_bind_text(stmt, 6, contourStr.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}
