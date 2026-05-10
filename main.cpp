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
    const int width = 2000;
    const int height = 2000;

    TGAImage image_test(width, height, TGAImage::RGB);

    Model test("diablo3_pose.obj");
    
    auto vertices = test.vertices();
    for(auto& ver : vertices){
        ver.x = (ver.x+1.0) * width / 2.0;
        ver.y = (ver.y+1.0) * height / 2.0;
        // 视口变换
    }

    for(const auto& frag : test.faces()){
        vec3i face = frag.v_idx;
        TriangleDraw(vertices[face.x], vertices[face.y], vertices[face.z], image_test, red);
        image_test.set(vertices[face.x].x, vertices[face.x].y, white);
        image_test.set(vertices[face.y].x, vertices[face.y].y, white);
        image_test.set(vertices[face.z].x, vertices[face.z].y, white);
    }
    image_test.write_tga_file("imagetest_framebuffer.tga");

    return 0;
}