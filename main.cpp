#include "rendering.h"
#include "tgaimage.h"
#include "tinyobjloader.h"
#include "transformation.h"
#include "geometry.h"

vec3f CamPos(0.0f, 0.0f, 3.0f);

int main(int argc, char** argv) {
    const int width = 2000;
    const int height = 2000;

    TGAImage image_test(width, height, TGAImage::RGB);
    
    z_buffer zbuffer(width, std::vector<float>(height, 1.0f));
    // float精度的深度缓冲区

    mat4f model = RotateY(5.0f) * Scale(1.2f);
    mat4f view  = LookAt(CamPos, vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f));
    mat4f perspective = Perspective(60.0f, (float)width / (float)height, 0.1f, 100.0f);
    mat4f viewport = Viewport(width, height);
    // 矩阵变换

    Model test("diablo3_pose.obj");
    // Model test("HotDog_Car.obj");
    auto vertices = test.vertices();
    for(auto& ver : vertices){
        vec4f clip = perspective * view * model * vec4f(ver.x, ver.y, ver.z, 1.0f);
        // 先将坐标转换到剪裁空间内
        vec3f ndc = clip.to_vec3();
        // 再将vec4转换成vec3(齐次坐标变换)，这一步同时做了w坐标除法
        ver = TransformPoint(viewport, ndc);
    }
    DrawFillFrame(vertices, test, image_test, zbuffer, width, height, white, red, yellow);
    image_test.write_tga_file("framebuffer1.tga");
    // Model-1

    // TGAImage image2_test(width, height, TGAImage::RGB);
    // Model test2("HotDog_Car.obj");
    // DrawWireFrame(test2, image2_test width, height, white, yellow,);
    // image2_test.write_tga_file("framebuffer2.tga");
    // // Model-2

    return 0;
}
