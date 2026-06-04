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

    const int width = 1200;
    const int height = 1200;

    const int SSAA_SCALE = 1;

    const int renderwidth = width * SSAA_SCALE;
    const int renderheight = height * SSAA_SCALE;

    TGAImage renderframebuf(renderwidth, renderheight, TGAImage::RGB);
    TGAImage ultimateframebuf(width, height, TGAImage::RGB);

    // float精度的深度缓冲区
    z_buffer light_zbuffer(renderwidth, std::vector<float>(renderheight, 1.0f));

    // mat4f modelMatrix = Translate(0.2f, -0.2f, -0.5f) * RotateY(35.0f) * Scale(0.5f);
    // 顺序很重要：先缩放再旋转，最后平移
    mat4f viewMatrix  = LookAt(CamPos, CoordinateOrigin, vec3f(0.0f, 1.0f, 0.0f));
    mat4f perspectiveMatrix = Perspective(60.0f, (float)renderwidth / (float)renderheight, 0.1f, 100.0f);
    mat4f viewportMatrix = Viewport(renderwidth, renderheight);
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

    TGAImage shadowDebug(renderwidth, renderheight, TGAImage::RGB);

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
    // 2. Camera depth pre-pass
    // ----------------------

    z_buffer camera_zbuffer(renderwidth, std::vector<float>(renderheight, 1.0f));
    TGAImage cameraDepthDebug(renderwidth, renderheight, TGAImage::RGB);

    ShadowDepthCalcShader backpackCameraDepth(
        BackPack,
        backpackModelMatrix,
        viewMatrix,
        perspectiveMatrix
    );

    Draw(BackPack, backpackCameraDepth, cameraDepthDebug, camera_zbuffer);

    ShadowDepthCalcShader planeCameraDepth(
        Plane,
        planeModelMatrix,
        viewMatrix,
        perspectiveMatrix
    );

    Draw(Plane, planeCameraDepth, cameraDepthDebug, camera_zbuffer);


    // ----------------------
    // 3. Camera final pass
    // ----------------------

    z_buffer final_zbuffer(renderwidth, std::vector<float>(renderheight, 1.0f));

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
        camera_zbuffer,
        BackPack_diffuse_tga,
        BackPack_normal_tga,
        BackPack_specular_tga
    );

    Draw(BackPack, backpackShader, renderframebuf, final_zbuffer);

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
        camera_zbuffer,
        PlaneDiffuse
    );

    Draw(Plane, planeShader, renderframebuf, final_zbuffer);

    ultimateframebuf = ResolveSSAA(renderframebuf, width, height, SSAA_SCALE);
    // 将高分辨率采样结果转写到普通帧缓冲上

    ultimateframebuf.write_tga_file("framebuffer1.tga");

    return 0;
}
