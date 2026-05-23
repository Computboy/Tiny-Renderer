#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include "tinyobjloader.h"
#include <random>
#include <cmath>
#include <algorithm>

class IShader {
public:
    // 顶点着色器
    virtual vec4f vertex(int faceIndex, int vertexIndex) = 0;

    // 片段着色器
    virtual std::pair<bool, TGAColor> fragment(const vec3f& bar) const = 0;
    // std::pair<bool, TGAColor> 第一个数据表示是否要进行丢弃[Alpha-Testing等准备]
    //                           第二个数据表示当前Fragment的颜色
    // bar 表示重心坐标[alpha, beta, gamma]

    virtual ~IShader() = default;
};

class FlatShader : public IShader {
private:
    const Model& mesh;

    mat4f modelMatrix;
    mat4f viewMatrix;
    mat4f projectionMatrix;
    TGAColor color;

public:

    FlatShader(
        const Model& mesh_,
        const mat4f& modelMatrix_,
        const mat4f& viewMatrix_,
        const mat4f& projectionMatrix_
    )
        : mesh(mesh_),
          modelMatrix(modelMatrix_),
          viewMatrix(viewMatrix_),
          projectionMatrix(projectionMatrix_),
          color{255, 255, 255, 255}
    {}

    vec4f vertex(int faceIndex, int vertexIndex) override {
        // 重写：重写父类中的虚函数
        vec3f v = mesh.vert(faceIndex, vertexIndex);
        
        if(vertexIndex == 0){
            setColor(RandomColor());
        }
        // 让每个三角形面片拥有一个随机颜色

        return projectionMatrix * viewMatrix * modelMatrix * vec4f(v.x, v.y, v.z, 1.0f);
    }

    std::pair<bool, TGAColor> fragment(const vec3f& bar) const override {
        return {false, color};
    }
    void setColor(const TGAColor& _color){
        color = _color;
    }

    TGAColor RandomColor() const {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 255);

    return TGAColor{
        static_cast<unsigned char>(dist(rng)),
        static_cast<unsigned char>(dist(rng)),
        static_cast<unsigned char>(dist(rng)),
        255
    };
}
};

class Blinn_PhongShader : public IShader {
private:
    const Model& mesh;

    mat4f modelMatrix;
    mat4f viewMatrix;
    mat4f projectionMatrix;
    TGAColor color;

    vec4f gl_Position[3];
    // 传出的顶点坐标数据

    // 传给 fragment 阶段的插值数据
    vec3f varying_worldPos[3];
    vec3f varying_normal[3];

    uv2f varying_uv[3];
    TGAImage diffusemap;

    color3f lightColor;
    vec3f lightPosition;
    // 光照属性
    vec3f cameraPos;

    color3f sampleDiffuse(const uv2f& uv) const {
        int tx = uv.x * diffusemap.width();
        int ty = uv.y * diffusemap.height();

        TGAColor color_in_tex = diffusemap.get(tx, ty);

        color3f baseColor(color_in_tex[2] / 255.0f, color_in_tex[1] / 255.0f, color_in_tex[0] / 255.0f);

        return baseColor;
    }

    color3f blinnPhong(
        const color3f& baseColor,
        const vec3f& frag_WorldPos,
        const normal3f& frag_Normal
    ) const {
        vec3f lightDir = (lightPosition - frag_WorldPos).normalize();
        vec3f viewDir  = (cameraPos - frag_WorldPos).normalize();

        vec3f halfwayDir = (lightDir + viewDir).normalize();

        float ambientStrength = 0.10f;
        vec3f ambient = baseColor * ambientStrength;

        float diff = std::max(0.0f, dot(frag_Normal, lightDir));
        vec3f diffuse = baseColor.cwiseproduct(lightColor) * diff;

        float specularStrength = 0.45f;
        float shininess = 32.0f;

        float spec = std::pow(std::max(0.0f, dot(frag_Normal, halfwayDir)), shininess);

        vec3f specular = lightColor * specularStrength * spec;
        vec3f result = ambient + diffuse + specular;

        result.x = std::clamp(result.x, 0.0f, 1.0f);
        result.y = std::clamp(result.y, 0.0f, 1.0f);
        result.z = std::clamp(result.z, 0.0f, 1.0f);

        return result;
    }

    TGAColor toTGAColor(const color3f& result) const {
        TGAColor color{
            static_cast<unsigned char>(result.z * 255.0f),
            static_cast<unsigned char>(result.y * 255.0f),
            static_cast<unsigned char>(result.x * 255.0f),
            255
        };
        return color;
    }

public:
    Blinn_PhongShader(
        const Model& mesh_,
        const mat4f& modelMatrix_,
        const mat4f& viewMatrix_,
        const mat4f& projectionMatrix_,
        const color3f& lightColor_,
        const vec3f& lightPosition_,
        const vec3f& cameraPos_,
        const TGAImage& diffusemap_
    )
        : mesh(mesh_),
          modelMatrix(modelMatrix_),
          viewMatrix(viewMatrix_),
          projectionMatrix(projectionMatrix_),
          color{255, 255, 255, 255},
          lightColor(lightColor_),
          lightPosition(lightPosition_),
          cameraPos(cameraPos_),
          diffusemap(diffusemap_)
    {}

    vec4f vertex(int faceIndex, int vertexIndex) override {
        vec3f v  = mesh.vert(faceIndex, vertexIndex);
        vec3f n  = mesh.normal(faceIndex, vertexIndex);
        vec2f uv = mesh.uv(faceIndex, vertexIndex);
        vec4f worldPos = modelMatrix * vec4f(v.x, v.y, v.z, 1.0f);

        gl_Position[vertexIndex] = projectionMatrix * viewMatrix * worldPos;
        varying_worldPos[vertexIndex] = vec3f(worldPos.x, worldPos.y, worldPos.z);
        varying_normal[vertexIndex] = n.normalize();
        // 暂时不考虑非均匀缩放
        varying_uv[vertexIndex] = uv;

        return gl_Position[vertexIndex];
    }

    std::pair<bool, TGAColor> fragment(const vec3f& bar) const override {
        vec3f frag_WorldPos =
            varying_worldPos[0] * bar.x +
            varying_worldPos[1] * bar.y +
            varying_worldPos[2] * bar.z;

        normal3f frag_Normal =(
                varying_normal[0] * bar.x +
                varying_normal[1] * bar.y +
                varying_normal[2] * bar.z
            ).normalize();

        uv2f bary_uv_coordinate =
            varying_uv[0] * bar.x +
            varying_uv[1] * bar.y +
            varying_uv[2] * bar.z;

        color3f baseColor = sampleDiffuse(bary_uv_coordinate);
        // 从纹理图像中获取纹理颜色

        color3f result = blinnPhong(baseColor, frag_WorldPos, frag_Normal);
        // 调用封装函数计算Blinn-Phong光照

        TGAColor color = toTGAColor(result);
        // 将vec3f转换成TGAColor类型，以在tga图像中显示

        return {false, color};
    }
    void setColor(const TGAColor& _color){
        color = _color;
    }
};