#include "tgaimage.h"
#include "rendering.h"
#include "tinyobjloader.h"

int main(int argc, char** argv){
    const int width = 2000;
    const int height = 2000;

    TGAImage image_test(width, height, TGAImage::RGB);
    Model test("diablo3_pose.obj");
    DrawFillFrame(test, image_test, width, height, white, red, yellow);
    image_test.write_tga_file("framebuffer1.tga");
    // Model-1

    // TGAImage image2_test(width, height, TGAImage::RGB);
    // Model test2("HotDog_Car.obj");
    // DrawWireFrame(test2, image2_test width, height, white, yellow,);
    // image2_test.write_tga_file("framebuffer2.tga");
    // // Model-2

    return 0;
}
