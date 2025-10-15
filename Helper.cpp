#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <direct.h>

std::wstring stringToWString(const std::string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), slength, 0, 0);
    std::wstring ws(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), slength, &ws[0], len);
    if (!ws.empty() && ws.back() == L'\0') ws.pop_back();
    return ws;
}

bool fileExists(const std::string& filename) {
    std::ifstream f(filename);
    return f.is_open();
}

cv::Mat loadImage(const std::string& imagePath) {
    if (!fileExists(imagePath)) {
        throw std::runtime_error("File does not exist: " + imagePath);
    }
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        throw std::runtime_error("Failed to load image: " + imagePath);
    }
    return image;
}

std::vector<std::string> listImagesInFolder(const std::string& folderPath) {
    std::vector<std::string> files;
    WIN32_FIND_DATA fileData;
    std::wstring searchPath = stringToWString(folderPath + "/*.*");
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::wstring wfilename = fileData.cFileName;
            std::string filename(wfilename.begin(), wfilename.end());
            if (filename != "." && filename != "..") {
                std::string ext = filename.substr(filename.find_last_of(".") + 1);
                if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp") {
                    files.push_back(folderPath + "/" + filename);
                }
            }
        } while (FindNextFileW(hFind, &fileData));
        FindClose(hFind);
    }
    return files;
}

bool saveImage(const std::string& folderPath, const std::string& imageName, const cv::Mat& image) {
    if (image.empty())
    {
        std::cerr << "Image data is empty, cannot save image.\n";
        return false;
    };
    std::string imageFolderPath = folderPath + "/Image";
    struct stat info;
    if (stat(imageFolderPath.c_str(), &info) != 0) {
        if (_mkdir(imageFolderPath.c_str()) != 0) {
            std::cerr << "Error creating directory: " << imageFolderPath << "\n";
            return false;
        }
    }
    size_t dotPos = imageName.find_last_of('.');
    std::string stem = (dotPos == std::string::npos) ? imageName : imageName.substr(0, dotPos);
    std::string filePath = imageFolderPath + "/" + stem + "_result.png";
    cv::imwrite(filePath, image);
    return true;
}