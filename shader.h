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
        // 重写父类中的虚函数
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
    uv2f  varying_uv[3];

    // 当前三角形的切线空间基向量
    // 这里先采用 soft rasterizer 里最直接的 face-level tangent。
    // fragment 中会用插值后的 N 对 T 做一次正交化，再重建 B。
    vec3f faceTangent;
    vec3f faceBitangent;
    float faceTangentSign = 1.0f;

    TGAImage diffusemap;
    TGAImage normalmap;

    color3f lightColor;
    vec3f lightPosition;
    // 光照属性
    vec3f cameraPos;

    int clampTexelX(float u, const TGAImage& image) const {
        if (image.width() <= 0) return 0;
        int x = static_cast<int>(u * image.width());
        return std::clamp(x, 0, image.width() - 1);
    }

    int clampTexelY(float v, const TGAImage& image) const {
        if (image.height() <= 0) return 0;
        int y = static_cast<int>(v * image.height());
        return std::clamp(y, 0, image.height() - 1);
    }

    color3f sampleDiffuse(const uv2f& uv) const {
        if (diffusemap.width() <= 0 || diffusemap.height() <= 0) {
            return color3f(1.0f, 1.0f, 1.0f);
        }

        int tx = clampTexelX(uv.x, diffusemap);
        int ty = clampTexelY(uv.y, diffusemap);

        TGAColor color_in_tex = diffusemap.get(tx, ty);

        // TGAColor 存储顺序是 BGRA，这里转成 RGB float。
        color3f baseColor(
            color_in_tex[2] / 255.0f,
            color_in_tex[1] / 255.0f,
            color_in_tex[0] / 255.0f
        );

        return baseColor;
    }

    vec3f sampleNormalTangent(const uv2f& uv) const {
        // 如果暂时没有 normal map，就返回切线空间默认法线方向。
        if (normalmap.width() <= 0 || normalmap.height() <= 0) {
            return vec3f(0.0f, 0.0f, 1.0f);
        }

        int tx = clampTexelX(uv.x, normalmap);
        int ty = clampTexelY(uv.y, normalmap);

        TGAColor color_in_tex = normalmap.get(tx, ty);

        // TGAColor: BGRA -> RGB
        vec3f n(
            color_in_tex[2] / 255.0f,
            color_in_tex[1] / 255.0f,
            color_in_tex[0] / 255.0f
        );

        // 真实向量需要还原到 [-1, 1]。
        n = n * 2.0f - vec3f(1.0f, 1.0f, 1.0f);

        // 如果绿色通道方向相反，可以打开这一句：
        // n.y = -n.y;

        return n.normalize();
    }

    void computeFaceTangent() {
        vec3f p0 = varying_worldPos[0];
        vec3f p1 = varying_worldPos[1];
        vec3f p2 = varying_worldPos[2];

        uv2f uv0 = varying_uv[0];
        uv2f uv1 = varying_uv[1];
        uv2f uv2 = varying_uv[2];

        vec3f e1 = p1 - p0;
        vec3f e2 = p2 - p0;

        uv2f duv1 = uv1 - uv0;
        uv2f duv2 = uv2 - uv0;

        float det = duv1.x * duv2.y - duv2.x * duv1.y;

        // 当前三角形的几何法线，只用于 fallback / handedness 判断。
        vec3f faceNormal = cross(e1, e2).normalize();

        if (std::abs(det) < 1e-6f) {
            // UV 退化时，无法从 UV 反解 T/B。
            // 这里构造一组和 faceNormal 垂直的任意切线基。
            vec3f helper = std::abs(faceNormal.x) > 0.9f
                ? vec3f(0.0f, 1.0f, 0.0f)
                : vec3f(1.0f, 0.0f, 0.0f);

            faceTangent = cross(helper, faceNormal).normalize();
            faceBitangent = cross(faceNormal, faceTangent).normalize();
            faceTangentSign = 1.0f;
            return;
        }

        float r = 1.0f / det;

        faceTangent =
            (e1 * duv2.y - e2 * duv1.y) * r;

        faceBitangent =
            (e2 * duv1.x - e1 * duv2.x) * r;

        faceTangent = faceTangent.normalize();
        faceBitangent = faceBitangent.normalize();

        // 记录 handedness：之后 fragment 中用 cross(N, T) 重建 B。
        // 如果 UV 有镜像，这个符号可以避免法线贴图方向翻转。
        vec3f T = (faceTangent - faceNormal * dot(faceTangent, faceNormal)).normalize();
        vec3f B_from_cross = cross(faceNormal, T).normalize();

        faceTangentSign = dot(B_from_cross, faceBitangent) < 0.0f ? -1.0f : 1.0f;
        faceTangent = T;
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
        const TGAImage& diffusemap_,
        const TGAImage& normalmap_
    )
        : mesh(mesh_),
          modelMatrix(modelMatrix_),
          viewMatrix(viewMatrix_),
          projectionMatrix(projectionMatrix_),
          color{255, 255, 255, 255},
          faceTangent(1.0f, 0.0f, 0.0f),
          faceBitangent(0.0f, 1.0f, 0.0f),
          faceTangentSign(1.0f),
          diffusemap(diffusemap_),
          normalmap(normalmap_),
          lightColor(lightColor_),
          lightPosition(lightPosition_),
          cameraPos(cameraPos_)
    {}

    vec4f vertex(int faceIndex, int vertexIndex) override {
        vec3f v  = mesh.vert(faceIndex, vertexIndex);
        vec3f n  = mesh.normal(faceIndex, vertexIndex);
        uv2f uv = mesh.uv(faceIndex, vertexIndex);

        vec4f worldPos = modelMatrix * vec4f(v.x, v.y, v.z, 1.0f);

        gl_Position[vertexIndex] = projectionMatrix * viewMatrix * worldPos;
        varying_worldPos[vertexIndex] = vec3f(worldPos.x, worldPos.y, worldPos.z);

        // 暂时不考虑非均匀缩放：用 modelMatrix 变换 normal。
        // 如果之后加入非均匀缩放，则要改成 normalMatrix = transpose(inverse(mat3(modelMatrix)))。
        vec4f worldNormal = modelMatrix * vec4f(n.x, n.y, n.z, 0.0f);
        varying_normal[vertexIndex] = vec3f(worldNormal.x, worldNormal.y, worldNormal.z).normalize();

        varying_uv[vertexIndex] = uv;

        if (vertexIndex == 2) {
            computeFaceTangent();
            // 当前三角形的最后一个需要处理的顶点，此时worldPos与uv坐标等都已经刷新成当前三角形
        }

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
        // 从 diffuse map 中获取基础颜色。

        // ===== Normal Mapping: tangent space -> world space =====
        vec3f N = frag_Normal.normalize();

        vec3f T = faceTangent.normalize();
        T = (T - N * dot(T, N)).normalize();

        vec3f B = cross(N, T).normalize() * faceTangentSign;

        vec3f n_tangent = sampleNormalTangent(bary_uv_coordinate);

        vec3f finalNormal =
            T * n_tangent.x +
            B * n_tangent.y +
            N * n_tangent.z;

        finalNormal = finalNormal.normalize();
        // 计算最终法线，切线空间扰动 → 世界坐标系下计算光照使用
        // =======================================================

        color3f result = blinnPhong(baseColor, frag_WorldPos, finalNormal);
        // 调用封装函数计算 Blinn-Phong 光照。

        TGAColor color = toTGAColor(result);
        // 将 vec3f 转换成 TGAColor 类型，以在 tga 图像中显示。

        return {false, color};
    }

    void setColor(const TGAColor& _color){
        color = _color;
    }
};