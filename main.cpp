#include "rendering.h"
#include "tgaimage.h"
#include "tinyobjloader.h"
#include "transformation.h"
#include "geometry.h"

vec3f CamPos(0.0f, 0.0f, 3.0f);

int main(int argc, char** argv) {
    const int width = 2000;
    const int height = 2000;

    TGAImage framebuf(width, height, TGAImage::RGB);
    
    z_buffer zbuffer(width, std::vector<float>(height, 1.0f));
    // float精度的深度缓冲区

    mat4f modelMatrix = RotateY(35.0f) * Scale(1.2f);
    mat4f viewMatrix  = LookAt(CamPos, vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f));
    mat4f perspectiveMatrix = Perspective(60.0f, (float)width / (float)height, 0.1f, 100.0f);
    mat4f viewportMatrix = Viewport(width, height);
    // 矩阵变换

    Model Diablo3("HotDog_Car.obj");
    // FlatShader myShader(Diablo3, modelMatrix, viewMatrix, perspectiveMatrix);
    Blinn_PhongShader myShader(Diablo3, modelMatrix, viewMatrix, perspectiveMatrix, vec3f(1.0f, 1.0f, 1.0f), vec3f(2.0f, -4.0f, 6.0f), CamPos);

    Draw(Diablo3, myShader, framebuf, zbuffer, width, height);
    framebuf.write_tga_file("framebuffer1.tga");
    // Model-1

    // TGAImage image2_test(width, height, TGAImage::RGB);
    // Model test2("HotDog_Car.obj");
    // DrawWireFrame(test2, image2_test width, height, white, yellow,);
    // image2_test.write_tga_file("framebuffer2.tga");
    // // Model-2

    return 0;
}
