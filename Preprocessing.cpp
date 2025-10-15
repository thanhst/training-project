#include <opencv2/opencv.hpp>
#include <string>
#include "Preprocessing.h"
Preprocessor::Preprocessor(const cv::Mat& input) : image(input.clone()) {}

cv::Mat  Preprocessor::toGray() {
	if (grayImg.empty())
		cv::cvtColor(image, grayImg, cv::COLOR_BGR2GRAY);
	//cv::imshow("Gray image", grayImg);
	//cv::waitKey();
	return grayImg;
}

cv::Mat Preprocessor::gaussianBlur(int ksize, double sigma) {
	if (grayImg.empty()) toGray();
	cv::GaussianBlur(grayImg, blurImg, cv::Size(ksize, ksize), sigma);
	//cv::imshow("Gausssian image", blurImg);
	//cv::waitKey();
	return blurImg;
}

cv::Mat Preprocessor::canny(double thresh1, double thresh2) {
	if (blurImg.empty()) gaussianBlur();
	cv::Canny(blurImg, edgeImg, thresh1, thresh2);
	//cv::imshow("Canny image", edgeImg);
	//cv::waitKey();
	return edgeImg;
}

cv::Mat Preprocessor::getProcessedImage() const {
	if (!edgeImg.empty()) return edgeImg;
	if (!blurImg.empty()) return blurImg;
	if (!grayImg.empty()) return grayImg;
	return image;
}

cv::Mat Preprocessor::resize(double fx, double fy) {
	cv::Mat resized;
	cv::resize(image, resized, cv::Size(), fx, fy);
	return resized;
}

cv::Mat Preprocessor::getOriginal() const { return image; }

void Preprocessor::setImage(const cv::Mat& input) {
	image = input.clone();
	grayImg.release();
	blurImg.release();
	edgeImg.release();
}

cv::Mat Preprocessor::processingPipeline(int blurKSize, double blurSigma, double cannyThresh1, double cannyThresh2) {
	toGray();

	//cv::equalizeHist(grayImg, grayImg);
	cv::imwrite("results/image_processing/grayImage.png", grayImg);
	
	gaussianBlur(blurKSize, blurSigma);
	cv::imwrite("results/image_processing/blurImage.png", blurImg);


	//cv::imshow("Equalized image", blurImg);
	//cv::waitKey(0);

	//cv::bilateralFilter(grayImg, blurImg, 7, 92, 20);
	//cv::Mat thresholdImage;
	//cv::threshold(blurImg, thresholdImage, 167, 255, cv::THRESH_BINARY);
	//cv::imshow("",thresholdImage);
	//cv::waitKey(0);

	//cv::Mat thresh;


	//cv::adaptiveThreshold(
	//	blurImg,                  // ảnh input (grayscale hoặc đã Gaussian blur)
	//	thresh,                // ảnh output
	//	255,                   // max value
	//	cv::ADAPTIVE_THRESH_GAUSSIAN_C, // phương pháp tính threshold cục bộ
	//	cv::THRESH_BINARY_INV,     // loại threshold
	//	7,                    // blockSize: vùng lân cận để tính threshold
	//	2                      // C: hằng số trừ vào mean (tinh chỉnh)
	//);
	//cv::imwrite("results/image_processing/thresholdImage.png", thresh);

	//cv::threshold(blurImg, thresh, 180, 255, cv::THRESH_BINARY);
	//cv::Mat resizedThresh;
	//cv::resize(thresh, resizedThresh, cv::Size(1000, 800));

	//cv::imshow("Threshold image", resizedThresh);
	//cv::waitKey(0);

	cv::Canny(blurImg, edgeImg, cannyThresh1, cannyThresh2);
	//canny(cannyThresh1, cannyThresh2);
	cv::imwrite("results/image_processing/cannyImage.png", edgeImg);
	//cv::Mat blurResized, edgeResized;

	//cv::resize(blurImg, blurResized, cv::Size(1000, 800));
	//cv::imshow("Blur image", blurResized);
	//cv::waitKey(0);

	//cv::resize(edgeImg, edgeResized, cv::Size(1000, 800));
	//cv::imshow("Processed image", edgeResized);
	//cv::waitKey(0);
	return edgeImg;
}
