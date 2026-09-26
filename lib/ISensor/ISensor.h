#pragma once

/*
Hàm = 0 tức là hàm không được viết ở lớp cha mà bắt buộc
phải đươc implement mở mỗi lớp con kế thừa lớp trừu tượng 
này
*/

class ISensor{
    public:
        virtual bool begin() = 0;
        virtual bool update() = 0;
        // Cú pháp const sau tên hàm -> không sửa biến
        // của đối tượng đang gọi hàm
        virtual const char* name() const = 0;
        // khi delete qua con trỏ ISensor*, không có 
        // virtual thì C++ chỉ chạy destructor của
        // ISensor, bỏ qua destructor của ADXL345. 
        // Mọi thứ ADXL345 tự cấp phát hay tự giữ sẽ không
        // được dọn. Có virtual thì nó chạy destructor con 
        // trước, rồi mới đến destructor cha.
        virtual ~ISensor() = default;
};
