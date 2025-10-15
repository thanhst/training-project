#include <iostream>
#include "Helper.h"
#include "opencv2/opencv.hpp"
#include "CircleManager.h"
#include "Circle.h"
#include <filesystem>
int main()
{
	const std::string imageFolderPath = "assets";
	std::vector<std::string> imagePaths = listImagesInFolder(imageFolderPath);
	std::vector<cv::Mat> images;
	std::vector<std::string> imageNames;
	for (const auto& path : imagePaths) {
		cv::Mat img = cv::imread(path);
		if (!img.empty()) {
			images.push_back(img);
			size_t pos = path.find_last_of("/\\");
			imageNames.push_back(path.substr(pos + 1));
		}
	}
	Manager manager;
	//auto allCircles = manager.findCircles(images, imageNames, 0.8);
	std::cout << "Loaded " << images.size() << " images.\n";
	std::cout << "Processed image ...\n";
	//for (size_t i = 0; i < allCircles.size(); ++i) {
	//	for (auto it = allCircles[i].begin(); it != allCircles[i].end(); ++it) {
	//		//std::cout << "Draw image : " << it->imageName << " with circle id: " << it->id << "\n";
	//		it->draw(images[i]);
	//		it->saveResult("results");
	//	}
	//}
	const std::string dbPath = "circles.db";
	manager.dropTableCircles(dbPath);
	manager.createDatabase(dbPath);
	for (size_t i = 0; i < images.size(); ++i) {
		auto circles = manager.findCircle(images[i], imageNames[i], 0.8, 10, 100);
		if(circles.empty()) {
			std::cout << "Image " << imageNames[i] << " has no circles detected. Skipping...\n";
			continue;
		}
		std::sort(circles.begin(), circles.end(),
			[](const Circle& a, const Circle& b) {
				return a.radius > b.radius;
			});
		std::vector<Circle> topCircles(circles.begin(), circles.begin() + 5);

		float bestDistance = std::numeric_limits<float>::max();
		float bestTotalRadius = 0.0f;
		Circle c1, c2;

		for (size_t i = 0; i < topCircles.size(); ++i) {
			for (size_t j = i + 1; j < topCircles.size(); ++j) {
				float dx = topCircles[i].center.x - topCircles[j].center.x;
				float dy = topCircles[i].center.y - topCircles[j].center.y;
				float dist2 = dx * dx + dy * dy;
				float totalRadius = topCircles[i].radius + topCircles[j].radius;

				if ((dist2 < bestDistance || dist2/bestDistance>=0.9 && dist2/bestDistance<=1) && (totalRadius>bestTotalRadius||totalRadius/bestTotalRadius>=0.9)) {
					bestDistance = dist2;
					bestTotalRadius = totalRadius;
					c1 = topCircles[i];
					c2 = topCircles[j];
				}
			}
		}

		if (circles.size() < 2) {
			std::cout << "Image " << imageNames[i] << " has less than 2 circles detected. Skipping...\n";
			continue;
		}
		
		cv::Point2f p1 = c1.center;
		cv::Point2f p2 = c2.center;

		float left = std::min(p1.x - c1.radius, p2.x - c2.radius);
		float right = std::max(p1.x + c1.radius, p2.x + c2.radius);
		float top = std::min(p1.y - c1.radius, p2.y - c2.radius);
		float bottom = std::max(p1.y + c1.radius, p2.y + c2.radius);

		cv::Rect boundingBox(
			cv::Point(left, top),
			cv::Point(right, bottom)
		);

		int padding = 10;
		boundingBox.x = std::max(boundingBox.x - padding, 0);
		boundingBox.y = std::max(boundingBox.y - padding, 0);
		boundingBox.width = std::min(boundingBox.width + 2 * padding, images[i].cols - boundingBox.x);
		boundingBox.height = std::min(boundingBox.height + 2 * padding, images[i].rows - boundingBox.y);

		c1.draw(images[i], cv::Scalar(0, 0, 255));
		c1.setImage(images[i]);
		c1.saveResult("results");

		c2.draw(images[i], cv::Scalar(0, 0, 255));
		c2.setImage(images[i]);
		c2.saveResult("results");
		
		for (auto it = circles.begin(); it != circles.end(); ++it) {
			if (it->center == c1.center && it->radius == c1.radius) continue;
			if (it->center == c2.center && it->radius == c2.radius) continue;

			it->draw(images[i]);
			it->setImage(images[i]);
			it->saveResult("results");
		}


		size_t dotPos = imageNames[i].find_last_of('.');
		std::string stem = (dotPos == std::string::npos) ? imageNames[i] : imageNames[i].substr(0, dotPos);

		cv::rectangle(images[i], boundingBox, cv::Scalar(255, 0, 0), 2);
		if(saveImage("results", stem, images[i]))
			std::cout << "Saved result image: " << "results/" + stem + ".png" << "\n";
		else
			std::cout << "Failed to save result image: " << "results/" + stem + ".png" << "\n";

		const std::string outputPath = "results/CSV/" + stem +".csv";
		manager.exportTwoMaxCircles(imageNames[i], dbPath, outputPath);
	}
	manager.exportDBToCSV("circles.db", "results/CSV/circles_export.csv");
	return 0;
}