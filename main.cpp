#include "tgaimage.h"
#include "linedraw.h"
#include "tinyobjloader.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor gray    = {100, 100, 100, 255};
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv){
    const int width = 640;
    const int height = 640;

    TGAImage image_test(width, height, TGAImage::RGB);

    Model test("diablo3_pose.obj");

    return 0;
}