# Dự án Phát hiện Biển báo (Sign Detection)

Dự án C++ trong Visual Studio sử dụng OpenCV và ONNX để phát hiện và phân loại biển báo giao thông.

## Yêu cầu hệ thống

1.  **Visual Studio 2019** (hoặc mới hơn).
    * Khi cài đặt, đảm bảo chọn workload **"Desktop development with C++"**.
2.  **Git** (để clone dự án).

## Hướng dẫn Build

Dự án này sử dụng **NuGet Package Restore** để tự động cài đặt tất cả các thư viện cần thiết.

1.  Clone dự án về máy của bạn:
    ```bash
    git clone https://github.com/nhathaogit/Sign_Detetion_CV_Application
    cd <ten-thu-muc-du-an>
    ```

2.  Mở tệp `.sln` bằng Visual Studio.

3.  Chọn cấu hình build (ví dụ: **`x64`** và **Debug**).

4.  Nhấn **Build Solution**.
   Lưu ý: Nếu thực hiện build file thực thi **`.exe`**, cần di chuyển file **config.json** đến vị trí ngang cấp với file thực thi.

6.  Hãy **Start without Debugging** để có thể tự động chạy chương trình thành công.

Việc chuẩn bị môi trường (như tải về thư viện, cấu hình directory bên trong Microsoft Visual Studio) là không cần thiết, VS Studio sẽ tự động nhận diện thư viện còn thiếu nhờ **pakages.config** và tự động tải về.

**Lưu ý:** Lần build đầu tiên có thể mất vài phút để tải các thư viện về.
