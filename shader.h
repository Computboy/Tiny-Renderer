#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include "tinyobjloader.h"
#include <random>

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