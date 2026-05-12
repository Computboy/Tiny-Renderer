#include "tgaimage.h"
#include "rendering.h"
#include "tinyobjloader.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor gray    = {100, 100, 100, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv){
    const int width = 2000;
    const int height = 2000;

    TGAImage image_test(width, height, TGAImage::RGB);
    Model test("diablo3_pose.obj");
    DrawWireFrame(test, image_test, white, red, width, height);
    image_test.write_tga_file("framebuffer1.tga");
    // Model-1

    TGAImage image2_test(width, height, TGAImage::RGB);
    Model test2("HotDog_Car.obj");
    DrawWireFrame(test2, image2_test, white, yellow, width, height);
    image2_test.write_tga_file("framebuffer2.tga");
    // Model-2

    return 0;
}
