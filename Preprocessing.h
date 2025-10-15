#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Preprocessor {
private:
    cv::Mat image;
    cv::Mat grayImg;
    cv::Mat blurImg;
    cv::Mat edgeImg;
public:
	Preprocessor() = default;
    Preprocessor(const cv::Mat& input);

    cv::Mat toGray();

    cv::Mat gaussianBlur(int ksize = 5, double sigma = 1.5);

    cv::Mat canny(double thresh1 = 50, double thresh2 = 150);

    cv::Mat getProcessedImage() const;

    cv::Mat resize(double fx, double fy);

    cv::Mat getOriginal() const;
	void setImage(const cv::Mat& input);
    cv::Mat processingPipeline(int blurKSize = 5, double blurSigma = 1.5, double cannyThresh1 = 50, double cannyThresh2 = 150);
};
