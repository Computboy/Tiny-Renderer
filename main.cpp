#include "tgaimage.h"
#include "linedraw.h"

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

    int ax = 70, ay = 30;
    int bx = 120, by = 370;
    int cx = 620, cy = 530;

    image_test.set(ax, ay, white);
    image_test.set(bx, by, white);
    image_test.set(cx, cy, white);

    point3f A(ax, ay, 0.0);
    point3f B{bx, by, 0.0};
    point3f C{cx, cy, 0.0};

    TriangleDraw(A, B, C, image_test, white);

    image_test.write_tga_file("imagetest_framebuffer.tga");

    return 0;
}