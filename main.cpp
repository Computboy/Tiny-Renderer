#include "rendering.h"
#include "tgaimage.h"
#include "tinyobjloader.h"
#include "geometry.h"

int main(int argc, char** argv) {
    const int width = 2000;
    const int height = 2000;

    TGAImage image_test(width, height, TGAImage::RGB);
    
    z_buffer zbuffer(width, std::vector<float>(height, 1.0f));
    // float精度的深度缓冲区

    Model test("diablo3_pose.obj");
    DrawFillFrame(test, image_test, zbuffer, width, height, white, red, yellow);
    image_test.write_tga_file("framebuffer1.tga");
    // Model-1

    // TGAImage image2_test(width, height, TGAImage::RGB);
    // Model test2("HotDog_Car.obj");
    // DrawWireFrame(test2, image2_test width, height, white, yellow,);
    // image2_test.write_tga_file("framebuffer2.tga");
    // // Model-2

    return 0;
}
