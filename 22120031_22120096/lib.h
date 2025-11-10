#pragma once

#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <string>
#include <sstream>
#include <conio.h>
#include <algorithm>
#include <vector>
#include <math.h>
#include <iomanip>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <stdexcept>

using namespace std;

extern cv::dnn::Net d_net;
extern cv::dnn::Net c_net;

struct AppConfig {
    string detector_path;
    string classifier_path;
    int detector_input_size;
    float conf_threshold;
    int classifier_input_size;
    int max_display_width;
    float display_conf_threshold;
};

bool loadConfig(AppConfig& config, const string& config_path);

struct Sign {
    int x1;
    int y1;
    int x2;
    int y2;
    string label;
    float confidence;
};

// Hàm xóa nội dung hiển thị trên console
void clearScreen();

// Hàm hiển thị nội dung Menu
void displayMenu(int selectedOption);

// Hàm kiểm tra có phải file ảnh
bool isImageFile(const string& filePath);

// Hàm mở cửa sổ File Explorer
bool openFileDialog(string& filePath);

// Hàm điều khiển chọn trong Menu
int getMenuChoice();

// Hàm tiền xử lý ảnh thủ công để bảo toàn tỷ lệ cho khâu phát hiện
cv::Mat letterbox(const cv::Mat& src, int new_size);

// Hàm tiền xử lý ảnh đầu vào cho mô hình phát hiện
cv::Mat processImageForDetection(const string& imagePath, int input_size);

// Hàm chạy phát hiện biển báo trên ảnh
void detect(const string& imagePath, vector<Sign>& signs, const AppConfig& config);

// Hàm tiền xử lý ảnh đầu vào cho mô hình phân loại
cv::Mat processImageForClassification(const string& imagePath, int x1, int y1, int x2, int y2, int classifier_input_size);

// Hàm chạy phân loại biển báo sau khi đã phát hiện biển báo
void classify(const string& imagePath, vector<Sign>& signs, const AppConfig& config);

// Hàm điều phối 2 tác vụ phát hiện và phân loại để xuất kết quả cuối cùng
void runModels(const string& imagePath, vector<Sign>& signs, const AppConfig& config);