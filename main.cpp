#include "rendering.h"
#include "tgaimage.h"
#include "tinyobjloader.h"
#include "transformation.h"
#include "geometry.h"

vec3f CamPos(0.0f, 0.0f, 3.0f);
vec3f EasyPointLightPos(0.0f, 2.0f, 4.0f);
vec3f LightColor(1.0f);
vec3f CoordinateOrigin(0.0f);

int main(int argc, char** argv) {

    const int width = 2000;
    const int height = 2000;

    TGAImage framebuf(width, height, TGAImage::RGB);
    
    z_buffer zbuffer(width, std::vector<float>(height, 1.0f));
    // float精度的深度缓冲区
    z_buffer light_zbuffer(width, std::vector<float>(height, 1.0f));

    mat4f modelMatrix = Translate(0.2f, -0.2f, -0.5f) * RotateY(35.0f) * Scale(0.5f);
    // 顺序很重要：先缩放再旋转，最后平移
    mat4f viewMatrix  = LookAt(CamPos, CoordinateOrigin, vec3f(0.0f, 1.0f, 0.0f));
    mat4f perspectiveMatrix = Perspective(60.0f, (float)width / (float)height, 0.1f, 100.0f);
    mat4f viewportMatrix = Viewport(width, height);
    // 矩阵变换

    mat4f lightviewMatrix = LookAt(EasyPointLightPos, CoordinateOrigin, vec3f(0.0f, 1.0f, 0.0f));
    mat4f lightperspectiveMatrix = Perspective(90.0f, 1.0f, 0.1f, 100.0f);

    Model BackPack("media/Backpack/backpack.obj");
    TGAImage BackPack_diffuse_tga, BackPack_normal_tga, BackPack_specular_tga;
    BackPack_diffuse_tga.read_tga_file("media/Backpack/backpackdiffuse_tga.tga");
    BackPack_normal_tga.read_tga_file("media/Backpack/backpacknormal_tga.tga");
    BackPack_specular_tga.read_tga_file("media/Backpack/backpackspecular_tga.tga");

    Blinn_PhongShader myShader(BackPack, modelMatrix, viewMatrix, perspectiveMatrix, LightColor, EasyPointLightPos, CamPos, BackPack_diffuse_tga, BackPack_normal_tga, BackPack_specular_tga);

    Draw(BackPack, myShader, framebuf, zbuffer);

    Model Plane("media/Plane/plane.obj");
    TGAImage PlaneDiffuse(1, 1, TGAImage::RGB, TGAColor{30, 55, 120, 255});
    modelMatrix = Translate(0.0f, -1.0f, 0.0f) * RotateY(45.0f) * Scale(1.3f);

    Blinn_PhongShader myShader2(Plane, modelMatrix, viewMatrix, perspectiveMatrix, LightColor, EasyPointLightPos, CamPos, PlaneDiffuse);

    Draw(Plane, myShader2, framebuf, zbuffer);

    // Model OldHouse("media/OldHouse/OldHouse.obj");
    // TGAImage OldHouse_diffuse_tga;
    // OldHouse_diffuse_tga.read_tga_file("media/OldHouse/housediff.tga");

    // modelMatrix = Translate(0.25f, -0.3f, 0.1f) * RotateY(-55.0f) * Scale(0.105f);

    // Blinn_PhongShader myShader2(OldHouse, modelMatrix, viewMatrix, perspectiveMatrix, vec3f(1.0f), vec3f(2.0f, -4.0f, 6.0f), CamPos, OldHouse_diffuse_tga);

    // Draw(OldHouse, myShader2, framebuf, zbuffer);

    framebuf.write_tga_file("framebuffer1.tga");

    return 0;
}
