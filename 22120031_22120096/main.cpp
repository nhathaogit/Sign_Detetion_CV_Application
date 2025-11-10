#include "lib.h"
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

cv::dnn::Net d_net;
cv::dnn::Net c_net;

bool loadConfig(AppConfig& config, const string& config_path) {
    ifstream f(config_path);
    if (!f.is_open()) {
        cerr << "Loi: Khong the mo file config.json tai: " << config_path << endl;
        return false;
    }

    try {
        json data = json::parse(f);

        config.detector_path = data["model_paths"]["detector"];
        config.classifier_path = data["model_paths"]["classifier"];

        config.detector_input_size = data["detector_config"]["input_size"];
        config.conf_threshold = data["detector_config"]["conf_threshold"];

        config.classifier_input_size = data["classifier_config"]["input_size"];

        config.max_display_width = data["display_config"]["max_width"];
        config.display_conf_threshold = data["display_config"]["display_threshold_percent"];

        cout << "Tai cau hinh thanh cong." << endl;
        return true;
    }
    catch (json::exception& e) {
        cerr << "Loi: Parse JSON that bai: " << e.what() << endl;
        return false;
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    // Không hiển thị log của opencv
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    AppConfig config;
    if (!loadConfig(config, "config.json")) {
        cout << "Vui long kiem tra file config.json!" << endl;
        system("pause");
        return -1;
    }

    cout << "Dang tai mo hinh phat hien va phan loai";
    d_net = cv::dnn::readNetFromONNX(config.detector_path);
    c_net = cv::dnn::readNetFromONNX(config.classifier_path);

    if (d_net.empty() || c_net.empty()) {
        cout << "Khong tai duoc mo hinh" << endl;
        return -1;
    }

    string selectedFile;
    vector<Sign> signs;

    while (true) {
        int choice = getMenuChoice();

        clearScreen();

        switch (choice) {
        case 0: {
            cout << "Dang mo File Explorer...\n";
            if (openFileDialog(selectedFile)) {
                if (isImageFile(selectedFile)) {
                    cout << "\n[THANH CONG] Duong dan anh da chon:\n";
                    cout << selectedFile << endl;
                    cout << "\nNhan phim bat ky de tiep tuc" << endl;
                    system("pause");
                    clearScreen();
                    signs.clear();
                    runModels(selectedFile, signs, config);
                }
                else {
                    cout << "\n[CANH BAO] File ban chon khong phai la dinh dang anh hop le!\n";
                    cout << "Vui long chon file voi dinh dang: .jpg, .jpeg, .png, .bmp, .gif, .tiff, .webp, .ico\n";
                    cout << "File da chon: " << selectedFile << endl;
                }
            }
            else {
                cout << "\n[HUY BO] Khong chon anh nao!\n";
            }

            cout << "\nNhan phim bat ky de tiep tuc...";
            int unused_key = _getch();
            break;
        }
        case 1:
            clearScreen();
            cout << "\nTam biet!\n";
            return 0;
        default:
            break;
        }
    }

    return 0;
}