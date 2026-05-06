#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor gray    = {100, 100, 100, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv){
    const int width = 150;
    const int height = 150;

    TGAImage image_test(width, height, TGAImage::RGB);
    // 头文件中定义图片解析的方式
    // TGAImage::RGB是枚举类型，为整型3

    for(size_t x = width / 10; x <= width / 2; x++){
        for(size_t y = height / 10; y <= height / 2; y++){
            image_test.set(x, y, gray);
        }
    }

    image_test.set(30, 90, red);
    image_test.set(90, 120, yellow);

    image_test.write_tga_file("imagetest_framebuffer.tga");

    return 0;
}