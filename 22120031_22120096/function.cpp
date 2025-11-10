#include "lib.h"

// Hàm xóa nội dung hiển thị trên console
void clearScreen() {
    system("cls");
}

// Hàm hiển thị nội dung Menu
void displayMenu(int selectedOption) {
    cout << "========== MENU ==========\n";

    if (selectedOption == 0)
        cout << "> ";
    else
        cout << "  ";
    cout << "1. Chon anh tu File Explorer\n";

    if (selectedOption == 1)
        cout << "> ";
    else
        cout << "  ";
    cout << "2. Thoat\n";

    cout << "==========================\n";
    cout << "\nSu dung phim mui ten len/xuong de chon, Enter de xac nhan\n";
}

// Hàm kiểm tra có phải file ảnh
bool isImageFile(const string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == string::npos) {
        return false;
    }

    string extension = filePath.substr(dotPos + 1);

    for (char& c : extension) {
        c = tolower(c);
    }

    return (extension == "jpg" || extension == "jpeg" ||
        extension == "png" || extension == "bmp" ||
        extension == "gif" || extension == "tiff" ||
        extension == "webp" || extension == "ico");
}

// Hàm mở cửa sổ File Explorer
bool openFileDialog(string& filePath) {
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.gif;*.tiff;*.webp;*.ico\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Chon anh can nhan dien";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        filePath = ofn.lpstrFile;
        return true;
    }
    return false;
}

// Hàm điều khiển chọn trong Menu
int getMenuChoice() {
    int selectedOption = 0;
    int totalOptions = 2;

    while (true) {
        clearScreen();
        displayMenu(selectedOption);

        int key = _getch();

        if (key == 224) {
            key = _getch();

            if (key == 72) {
                selectedOption--;
                if (selectedOption < 0) {
                    selectedOption = totalOptions - 1;
                }
            }
            else if (key == 80) {
                selectedOption++;
                if (selectedOption >= totalOptions) {
                    selectedOption = 0;
                }
            }
        }
        else if (key == 13) {
            return selectedOption;
        }
    }
}

// Hàm tiền xử lý ảnh thủ công để bảo toàn tỷ lệ cho khâu phát hiện
cv::Mat letterbox(const cv::Mat& src, int new_size = 640) {
    float r = min((float)new_size / src.cols, (float)new_size / src.rows);
    int new_w = int(src.cols * r);
    int new_h = int(src.rows * r);

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(new_w, new_h));

    cv::Mat canvas = cv::Mat::zeros(new_size, new_size, CV_8UC3);
    resized.copyTo(canvas(cv::Rect((new_size - new_w) / 2, (new_size - new_h) / 2, new_w, new_h)));

    return canvas;
}

// Hàm tiền xử lý ảnh đầu vào cho mô hình phát hiện
cv::Mat processImageForDetection(const string& imagePath, int input_size = 640) {
    // Đọc ảnh và kiểm tra ảnh
    cv::Mat src = cv::imread(imagePath);
    if (src.empty()) {
        cerr << "Loi: Khong the doc anh tu duong dan" << imagePath << endl;
        return cv::Mat();
    }

    // Letterbox (Đổi kích thước và Đệm)
    cv::Mat letterboxed_img = letterbox(src, input_size);

    // Đổi màu (BGR -> RGB)
    cv::Mat rgb_img;
    cv::cvtColor(letterboxed_img, rgb_img, cv::COLOR_BGR2RGB);

    // Chuẩn hóa, tạo Blob
    cv::Mat blob;
    cv::dnn::blobFromImage(rgb_img, blob, 1.0 / 255.0, cv::Size(input_size, input_size), cv::Scalar(), false, false, CV_32F);

    return blob;
}

// Hàm chạy phát hiện biển báo trên ảnh
void detect(const string& imagePath, vector<Sign>& signs, const AppConfig& config) {
    // Đọc ảnh gốc
    cv::Mat img = cv::imread(imagePath);
    if (img.empty()) {
        cout << "Loi: Khong the doc anh goc" << endl;
        return;
    }

    // Tiền xử lý ảnh
    cv::Mat inputBlob = processImageForDetection(imagePath, config.detector_input_size);
    if (inputBlob.empty()) return;

    // Chạy mô hình phát hiện
    d_net.setInput(inputBlob);
    cv::Mat output;
    d_net.forward(output);

    // Hậu xử lý output
    if (output.dims == 3 && output.size[0] == 1) {
        output = cv::Mat(cv::Size(output.size[2], output.size[1]), CV_32F, output.ptr<float>()).clone();
    }

    cv::Mat prob;
    cv::transpose(output, prob);

    vector<cv::Rect> boxes;
    vector<float> confidences;

    for (int i = 0; i < prob.rows; i++) {
        float confidence = prob.at<float>(i, 4);
        // Bỏ qua các kết quả phát hiện có độ tin cậy < 50%
        if (confidence < config.conf_threshold) continue;

        // Đây là tọa độ pixel trên ảnh letterbox 640x640
        float x_center = prob.at<float>(i, 0);
        float y_center = prob.at<float>(i, 1);
        float w = prob.at<float>(i, 2);
        float h = prob.at<float>(i, 3);

        // Chuyển từ center format (pixel) sang corner format (pixel)
        int x1 = int(x_center - w / 2);
        int y1 = int(y_center - h / 2);
        int x2 = int(x_center + w / 2);
        int y2 = int(y_center + h / 2);

        x1 = max(0, min(x1, config.detector_input_size - 1));
        y1 = max(0, min(y1, config.detector_input_size - 1));
        x2 = max(0, min(x2, config.detector_input_size - 1));
        y2 = max(0, min(y2, config.detector_input_size - 1));

        // Lưu tọa độ boudingbox và độ tin cậy
        // Trong boxes là tọa độ những boudingbox có độ tin cậy từ 50% trở lên 
        boxes.emplace_back(x1, y1, x2 - x1, y2 - y1);
        confidences.emplace_back(confidence);
    }

    // Trong boxes còn chứa các tọa độ boudingbox nằm chồng lấn lên nhau dù đã loại những cái < 50%
    // Dùng NMS để loại các boudingbox chồng lấn lên nhau
    vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, config.conf_threshold, 0.4, indices);

    // Kiểm tra có phát hiện được biển báo nào không
    if (indices.empty()) {
        cout << "\n[CANH BAO] Khong phat hien duoc bien bao nao!" << endl;
        cv::imshow("Ket qua phat hien", img);
        cv::waitKey(0);
        cv::destroyAllWindows();
        return;
    }

    // Tính tỷ lệ scale từ letterbox về ảnh gốc
    float scale = min((float)config.detector_input_size / img.cols,
        (float)config.detector_input_size / img.rows);
    int pad_w = (config.detector_input_size - int(img.cols * scale)) / 2;
    int pad_h = (config.detector_input_size - int(img.rows * scale)) / 2;

    cout << "\n=== PHAT HIEN " << indices.size() << " BIEN BAO ===" << endl;

    for (size_t i = 0; i < indices.size(); i++) {
        int idx = indices[i];
        cv::Rect box = boxes[idx];

        // Chuyển tọa độ từ letterbox (640x640) về ảnh gốc
        int x1 = int((box.x - pad_w) / scale);
        int y1 = int((box.y - pad_h) / scale);
        int x2 = int((box.x + box.width - pad_w) / scale);
        int y2 = int((box.y + box.height - pad_h) / scale);

        // Đảm bảo tọa độ trong phạm vi ảnh
        x1 = max(0, min(x1, img.cols - 1));
        y1 = max(0, min(y1, img.rows - 1));
        x2 = max(0, min(x2, img.cols - 1));
        y2 = max(0, min(y2, img.rows - 1));

        // Kiểm tra xem box có kích thước hợp lệ không
        if (x1 >= x2 || y1 >= y2) {
            cout << "Box " << i + 1 << ": Bi loai do kich thuoc khong hop le sau khi scale." << endl;
            continue;
        }

        // Xuất tọa độ boudingbox các biển báo và lưu tọa độ từng biển
        cout << "Box " << i + 1 << ": (" << x1 << "," << y1 << ") -> (" << x2 << "," << y2 << ")" << endl;
        signs.push_back({ x1, y1, x2, y2, "", 0.0f });
    }
}

// Hàm tiền xử lý ảnh đầu vào cho mô hình phân loại
cv::Mat processImageForClassification(const string& imagePath, int x1, int y1, int x2, int y2, int classifier_input_size) {
    cv::Mat img = cv::imread(imagePath);
    if (img.empty()) return cv::Mat();

    int width = x2 - x1;
    int height = y2 - y1;
    cv::Mat crop_img = img(cv::Rect(x1, y1, width, height));

    cv::resize(crop_img, crop_img, cv::Size(classifier_input_size, classifier_input_size));

    cv::Mat blob = cv::dnn::blobFromImage(crop_img, 1.0 / 255.0, cv::Size(classifier_input_size, classifier_input_size), cv::Scalar(), false, false);
    return blob;
}

// Hàm chạy phân loại biển báo sau khi phát hiện biển báo
void classify(const string& imagePath, vector<Sign>& signs, const AppConfig& config) {
    // Lặp qua danh sách lưu thông tin các biển báo được phát hiện
    for (int i = 0; i < signs.size(); i++) {
        int x1 = signs[i].x1;
        int y1 = signs[i].y1;
        int x2 = signs[i].x2;
        int y2 = signs[i].y2;

        // Tiền xử lý ảnh (crop và tiền xử lý từng khung biển báo trên ảnh gốc)
        cv::Mat blob = processImageForClassification(imagePath, x1, y1, x2, y2, config.classifier_input_size);

        // Đưa ảnh crop quanh biển báo vào mô hình phân loại
        c_net.setInput(blob);

        // Chạy mô hình phân loại
        cv::Mat output = c_net.forward();

        // Phân tích kết quả
        cv::Point classIdPoint;
        double confidence;
        cv::minMaxLoc(output, nullptr, &confidence, nullptr, &classIdPoint);

        int classId = classIdPoint.x;  // chỉ số lớp có xác suất cao nhất
        // Ánh xạ nhãn biển báo từ chữ số sang dạng chữ viết
        string signClass;
        switch (classIdPoint.x) {
        case 0: signClass = "Stop"; break;
        case 1: signClass = "No Traffic"; break;
        case 2: signClass = "No Entry"; break;
        case 3: signClass = "No Cars"; break;
        case 4: signClass = "Speed Limit (50 km/h)"; break;
        case 5: signClass = "No Honking"; break;
        case 6: signClass = "No Stopping/Parking"; break;
        }

        // Xuất kết quả dự đoán của mô hình phân loại và lưu vào danh sách biển báo
        cout << "\n--- KET QUA DU DOAN ---" << endl;
        cout << "Anh: " << imagePath << endl;
        cout << "Nhan du doan: " << signClass << endl;
        cout << "Do tin cay: " << confidence * 100.0 << "%" << endl;
        signs[i].label = signClass;
        signs[i].confidence = confidence * 100.0;
    }
}

// Hàm điều phối 2 tác vụ phát hiện và phân loại để xuất kết quả cuối cùng
void runModels(const string& imagePath, vector<Sign>& signs, const AppConfig& config) {
    cout << "Dang xuat anh ket qua" << endl;

    // Chạy 2 tác vụ phát hiện và phân loại để lưu thông tin từng biển báo
    detect(imagePath, signs, config);
    classify(imagePath, signs, config);

    // Đọc ảnh gốc để điều chỉnh rồi xuất ảnh kết quả
    cv::Mat img = cv::imread(imagePath);

    // Chiều rộng hiển thị tối đa mong muốn, thông thường ảnh có width lớn hơn height nên chỉ giới hạn width
    const int MAX_DISPLAY_WIDTH = config.max_display_width;

    cv::Mat img_display = img.clone(); // Tạo bản sao để vẽ
    double scale_factor = 1.0;

    // Chỉ resize nếu ảnh gốc quá lớn
    if (img.cols > MAX_DISPLAY_WIDTH) {
        scale_factor = (double)MAX_DISPLAY_WIDTH / img.cols;
        int new_height = (int)(img.rows * scale_factor);

        // Resize ảnh để hiển thị
        cv::resize(img, img_display, cv::Size(MAX_DISPLAY_WIDTH, new_height));
    }

    // Lặp qua danh sách biển báo để vẽ boudingbox và nhãn từng biển báo lên ảnh gốc
    for (int i = 0; i < signs.size(); i++) {
        int x1 = (int)(signs[i].x1 * scale_factor);
        int y1 = (int)(signs[i].y1 * scale_factor);
        int x2 = (int)(signs[i].x2 * scale_factor);
        int y2 = (int)(signs[i].y2 * scale_factor);

        if (signs[i].confidence < config.display_conf_threshold)
        {
            continue;
        }
        // Vẽ boudingbox
        cv::rectangle(img_display, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 3);
        // Vẽ nhãn biển báo
        stringstream str;
        str << signs[i].label << " " << setprecision(4) << signs[i].confidence << "%";
        string box_label = str.str();
        cv::putText(img_display, box_label, cv::Point(x2+5, max(y1+20, 0)),
            cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    }
    cv::imshow("Ket qua cuoi cung", img_display);
    cout << "\nNhan phim bat ky tren cua so anh de dong..." << endl;
    cv::waitKey(0);
    cv::destroyAllWindows();
}