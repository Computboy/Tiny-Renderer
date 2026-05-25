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

    mat4f modelMatrix = Translate(0.15f, 0.0f, 0.1f) * RotateY(35.0f) * Scale(0.4f);
    // 顺序很重要：先缩放再旋转，最后平移
    mat4f viewMatrix  = LookAt(CamPos, vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f));
    mat4f perspectiveMatrix = Perspective(60.0f, (float)width / (float)height, 0.1f, 100.0f);
    mat4f viewportMatrix = Viewport(width, height);
    // 矩阵变换

    Model BackPack("media/Backpack/backpack.obj");
    TGAImage BackPack_diffuse_tga, BackPack_normal_tga, BackPack_specular_tga;
    BackPack_diffuse_tga.read_tga_file("media/Backpack/backpackdiffuse_tga.tga");
    BackPack_normal_tga.read_tga_file("media/Backpack/backpacknormal_tga.tga");
    BackPack_specular_tga.read_tga_file("media/Backpack/backpackspecular_tga.tga");

    Blinn_PhongShader myShader(BackPack, modelMatrix, viewMatrix, perspectiveMatrix, vec3f(1.0f, 1.0f, 1.0f), vec3f(2.0f, -4.0f, 6.0f), CamPos, BackPack_diffuse_tga, BackPack_normal_tga, BackPack_specular_tga);

    Draw(BackPack, myShader, framebuf, zbuffer);
    framebuf.write_tga_file("framebuffer1.tga");

    return 0;
}
