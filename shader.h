#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "tinyobjloader.h"
#include "transformation.h"

#include <utility>
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>


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

    TGAColor RandomColor() const;

public:
    FlatShader(
        const Model& mesh_,
        const mat4f& modelMatrix_,
        const mat4f& viewMatrix_,
        const mat4f& projectionMatrix_
    );

    vec4f vertex(int faceIndex, int vertexIndex) override;
    std::pair<bool, TGAColor> fragment(const vec3f& bar) const override;

    void setColor(const TGAColor& _color);
};

class Blinn_PhongShader : public IShader {
protected:
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
    uv2f  varying_uv[3];

    // 当前三角形的切线空间基向量
    // 这里先采用 soft rasterizer 里最直接的 face-level tangent。
    // fragment 中会用插值后的 N 对 T 做一次正交化，再重建 B。
    vec3f faceTangent;
    vec3f faceBitangent;
    float faceTangentSign;

    TGAImage diffusemap;
    TGAImage normalmap;
    TGAImage specularmap;

    color3f lightColor;
    vec3f lightPosition;
    // 光照属性
    vec3f cameraPos;

    int clampTexelX(float u, const TGAImage& image) const;
    int clampTexelY(float v, const TGAImage& image) const;

    float sampleAlpha(const uv2f& uv) const;
    color3f sampleDiffuse(const uv2f& uv) const;
    float sampleSpecular(const uv2f& uv) const;
    vec3f sampleNormalTangent(const uv2f& uv) const;

    void computeFaceTangent();

    color3f blinnPhong(
        const color3f& baseColor,
        const float& specularColor,
        const vec3f& frag_WorldPos,
        const normal3f& frag_Normal
    ) const;

    TGAColor toTGAColor(const color3f& result) const;

public:
    Blinn_PhongShader(
        const Model& mesh_,
        const mat4f& modelMatrix_,
        const mat4f& viewMatrix_,
        const mat4f& projectionMatrix_,
        const color3f& lightColor_,
        const vec3f& lightPosition_,
        const vec3f& cameraPos_,
        const TGAImage& diffusemap_ = TGAImage(),
        const TGAImage& normalmap_ = TGAImage(),
        const TGAImage& specularmap_ = TGAImage()
    );

    vec4f vertex(int faceIndex, int vertexIndex) override;
    std::pair<bool, TGAColor> fragment(const vec3f& bar) const override;

    void setColor(const TGAColor& _color);
};

class ShadowDepthCalcShader : public IShader {
private:
    const Model& mesh;

    mat4f modelMatrix;
    mat4f viewMatrix;
    mat4f projectionMatrix;
public:
    ShadowDepthCalcShader(
        const Model& mesh_,
        const mat4f& modelMatrix_,
        const mat4f& viewMatrix_,
        const mat4f& projectionMatrix_
    );
    vec4f vertex(int faceIndex, int vertexIndex) override;
    std::pair<bool, TGAColor> fragment(const vec3f& bar) const;
};

class Shadow_Blinn_PhongShader : public Blinn_PhongShader {
private:
    const z_buffer& shadowBuffer;
    mat4f lightMVP;
    int shadowWidth;
    int shadowHeight;
    mat4f Lightviewport;
    float bias;
    const int sample_Num = 20;
    std::vector<vec3f> sampleKernel;
    const std::vector<std::vector<float>>& camera_zbuffer;
    void generateSampleKernel();
    color3f shadow_BlinnPhong(
        const color3f& baseColor,
        const float& specularColor,
        const vec3f& frag_WorldPos,
        const normal3f& frag_Normal,
        float ao_factor
    ) const;

public:
    Shadow_Blinn_PhongShader(
        const Model& mesh_,
        const mat4f& modelMatrix_,
        const mat4f& viewMatrix_,
        const mat4f& projectionMatrix_,
        const color3f& lightColor_,
        const vec3f& lightPosition_,
        const vec3f& cameraPos_,
        const z_buffer& shadowBuffer_,
        const mat4f& lightMVP_,
        const TGAImage& diffusemap_ = TGAImage(),
        const TGAImage& normalmap_ = TGAImage(),
        const TGAImage& specularmap_ = TGAImage(),
        const std::vector<std::vector<float>>& depthbuffer_
    );

    std::pair<bool, TGAColor> fragment(const vec3f& bar) const override;
    float getShadowDepth(int x, int y) const;
    float CalculateSSAO(const vec3f& frag_ViewPos, const vec3f& frag_ViewNormal) const;
};
