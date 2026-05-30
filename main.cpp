#include "rendering.h"
#include "tgaimage.h"
#include "tinyobjloader.h"
#include "transformation.h"
#include "geometry.h"

vec3f CamPos(0.0f, 0.0f, 3.0f);
vec3f EasyPointLightPos(0.2f, 1.8f, 1.2f);
vec3f LightColor(1.0f);
vec3f CoordinateOrigin(0.0f);

int main(int argc, char** argv) {

    const int width = 2000;
    const int height = 2000;

    TGAImage framebuf(width, height, TGAImage::RGB);
    
    z_buffer zbuffer(width, std::vector<float>(height, 1.0f));
    // float精度的深度缓冲区
    z_buffer light_zbuffer(width, std::vector<float>(height, 1.0f));

    // mat4f modelMatrix = Translate(0.2f, -0.2f, -0.5f) * RotateY(35.0f) * Scale(0.5f);
    // 顺序很重要：先缩放再旋转，最后平移
    mat4f viewMatrix  = LookAt(CamPos, CoordinateOrigin, vec3f(0.0f, 1.0f, 0.0f));
    mat4f perspectiveMatrix = Perspective(60.0f, (float)width / (float)height, 0.1f, 100.0f);
    mat4f viewportMatrix = Viewport(width, height);
    // 矩阵变换

    mat4f lightviewMatrix = LookAt(EasyPointLightPos, CoordinateOrigin, vec3f(0.0f, 0.0f, 1.0f));
    mat4f lightperspectiveMatrix = Perspective(90.0f, 1.0f, 0.1f, 20.0f);

    Model BackPack("media/Backpack/backpack.obj");

    TGAImage BackPack_diffuse_tga, BackPack_normal_tga, BackPack_specular_tga;
    BackPack_diffuse_tga.read_tga_file("media/Backpack/backpackdiffuse_tga.tga");
    BackPack_normal_tga.read_tga_file("media/Backpack/backpacknormal_tga.tga");
    BackPack_specular_tga.read_tga_file("media/Backpack/backpackspecular_tga.tga");

    Model Plane("media/Plane/plane.obj");
    TGAImage PlaneDiffuse(1, 1, TGAImage::RGB, TGAColor{30, 55, 120, 255});

    mat4f backpackModelMatrix =
        Translate(0.2f, -0.2f, -0.5f)
        * RotateY(35.0f)
        * Scale(0.5f);

    mat4f planeModelMatrix =
        Translate(0.0f, -1.2f, -1.2f)
        * RotateY(45.0f)
        * Scale(2.0f);

    mat4f lightMVP = lightperspectiveMatrix * lightviewMatrix;

    // ----------------------
    // 1. Shadow pass
    // ----------------------

    TGAImage shadowDebug(width, height, TGAImage::RGB);

    ShadowDepthCalcShader backpackShadow(
        BackPack,
        backpackModelMatrix,
        lightviewMatrix,
        lightperspectiveMatrix
    );

    Draw(BackPack, backpackShadow, shadowDebug, light_zbuffer);

    ShadowDepthCalcShader planeShadow(
        Plane,
        planeModelMatrix,
        lightviewMatrix,
        lightperspectiveMatrix
    );

    Draw(Plane, planeShadow, shadowDebug, light_zbuffer);

    // shadowDebug.write_tga_file("shadow_pass_debug.tga");
    // ----------------------
    // 2. Camera pass
    // ----------------------

    Shadow_Blinn_PhongShader backpackShader(
        BackPack,
        backpackModelMatrix,
        viewMatrix,
        perspectiveMatrix,
        LightColor,
        EasyPointLightPos,
        CamPos,
        light_zbuffer,
        lightMVP,
        BackPack_diffuse_tga,
        BackPack_normal_tga,
        BackPack_specular_tga
    );

    Draw(BackPack, backpackShader, framebuf, zbuffer);

    Shadow_Blinn_PhongShader planeShader(
        Plane,
        planeModelMatrix,
        viewMatrix,
        perspectiveMatrix,
        LightColor,
        EasyPointLightPos,
        CamPos,
        light_zbuffer,
        lightMVP,
        PlaneDiffuse
    );

    Draw(Plane, planeShader, framebuf, zbuffer);

    // Model OldHouse("media/OldHouse/OldHouse.obj");
    // TGAImage OldHouse_diffuse_tga;
    // OldHouse_diffuse_tga.read_tga_file("media/OldHouse/housediff.tga");

    // modelMatrix = Translate(0.25f, -0.3f, 0.1f) * RotateY(-55.0f) * Scale(0.105f);

    // Blinn_PhongShader myShader2(OldHouse, modelMatrix, viewMatrix, perspectiveMatrix, vec3f(1.0f), vec3f(2.0f, -4.0f, 6.0f), CamPos, OldHouse_diffuse_tga);

    // Draw(OldHouse, myShader2, framebuf, zbuffer);

    framebuf.write_tga_file("framebuffer1.tga");


    // TGAImage shadowDepthVis(width, height, TGAImage::RGB);

    // float minDepth = 1.0f;
    // float maxDepth = 0.0f;

    // for (int x = 0; x < width; x++) {
    //     for (int y = 0; y < height; y++) {
    //         if (light_zbuffer[x][y] < 1.0f) {
    //             minDepth = std::min(minDepth, light_zbuffer[x][y]);
    //             maxDepth = std::max(maxDepth, light_zbuffer[x][y]);
    //         }
    //     }
    // }

    // for (int x = 0; x < width; x++) {
    //     for (int y = 0; y < height; y++) {
    //         float d = light_zbuffer[x][y];

    //         if (d >= 1.0f) {
    //             shadowDepthVis.set(x, y, TGAColor{0, 0, 0, 255});
    //         } else {
    //             float t = (d - minDepth) / (maxDepth - minDepth + 1e-6f);
    //             t = 1.0 - t;
    //             unsigned char c = static_cast<unsigned char>(t * 255.0f);
    //             shadowDepthVis.set(x, y, TGAColor{c, c, c, 255});
    //         }
    //     }
    // }

    // shadowDepthVis.write_tga_file("shadow_depth_vis.tga");
    // 输出深度图的可视化调试

    return 0;
}
