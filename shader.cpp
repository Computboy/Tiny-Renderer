#include "shader.h"

#include <algorithm>
#include <cmath>
#include <random>

FlatShader::FlatShader(
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

vec4f FlatShader::vertex(int faceIndex, int vertexIndex) {
    // 重写父类中的虚函数
    vec3f v = mesh.vert(faceIndex, vertexIndex);

    if (vertexIndex == 0) {
        setColor(RandomColor());
    }
    // 让每个三角形面片拥有一个随机颜色

    return projectionMatrix * viewMatrix * modelMatrix * vec4f(v.x, v.y, v.z, 1.0f);
}

std::pair<bool, TGAColor> FlatShader::fragment(const vec3f& bar) const {
    return {false, color};
}

void FlatShader::setColor(const TGAColor& _color) {
    color = _color;
}

TGAColor FlatShader::RandomColor() const {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 255);

    return TGAColor{
        static_cast<unsigned char>(dist(rng)),
        static_cast<unsigned char>(dist(rng)),
        static_cast<unsigned char>(dist(rng)),
        255
    };
}

Blinn_PhongShader::Blinn_PhongShader(
    const Model& mesh_,
    const mat4f& modelMatrix_,
    const mat4f& viewMatrix_,
    const mat4f& projectionMatrix_,
    const color3f& lightColor_,
    const vec3f& lightPosition_,
    const vec3f& cameraPos_,
    const TGAImage& diffusemap_,
    const TGAImage& normalmap_,
    const TGAImage& specularmap_
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
      specularmap(specularmap_),
      lightColor(lightColor_),
      lightPosition(lightPosition_),
      cameraPos(cameraPos_)
{}

vec4f Blinn_PhongShader::vertex(int faceIndex, int vertexIndex) {
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
        // 当前三角形的最后一个需要处理的顶点，此时 worldPos 与 uv 坐标等都已经刷新成当前三角形
    }

    return gl_Position[vertexIndex];
}

std::pair<bool, TGAColor> Blinn_PhongShader::fragment(const vec3f& bar) const {
    float inv_w0 = 1.0f / gl_Position[0].w;
    float inv_w1 = 1.0f / gl_Position[1].w;
    float inv_w2 = 1.0f / gl_Position[2].w;

    float z = bar.x * inv_w0 + bar.y * inv_w1 + bar.z * inv_w2;

    vec3f bc(
        bar.x * inv_w0 / z,
        bar.y * inv_w1 / z,
        bar.z * inv_w2 / z
    );

    vec3f frag_WorldPos =
        varying_worldPos[0] * bc.x +
        varying_worldPos[1] * bc.y +
        varying_worldPos[2] * bc.z;

    normal3f frag_Normal = (
        varying_normal[0] * bc.x +
        varying_normal[1] * bc.y +
        varying_normal[2] * bc.z
    ).normalize();

    uv2f bary_uv_coordinate =
        varying_uv[0] * bc.x +
        varying_uv[1] * bc.y +
        varying_uv[2] * bc.z;

    float alpha = sampleAlpha(bary_uv_coordinate);

    if (alpha < 0.1f) {
        // Alpha-Testing 丢弃高透明度部分纹理
        return {true, TGAColor{0, 0, 0, 0}};
    }

    color3f baseColor = sampleDiffuse(bary_uv_coordinate);
    // 从 diffuse map 中获取基础颜色。
    float specColor = sampleSpecular(bary_uv_coordinate);

    // ===== Normal Mapping: tangent space -> world space =====
    vec3f N = frag_Normal.normalize();

    vec3f T = faceTangent.normalize();
    T = (T - N * dot(T, N)).normalize();

    vec3f B = cross(N, T).normalize() * faceTangentSign;

    vec3f n_tangent = sampleNormalTangent(bary_uv_coordinate);
    // n_tangent = vec3f(0.0f, 0.0f ,1.0f);
    // 如果怀疑是 TBN 矩阵计算出错，可以手动将采样到的 Normal 全部置换成 (0,0,1)，看效果是否回退至原先样式
    vec3f finalNormal =
        T * n_tangent.x +
        B * n_tangent.y +
        N * n_tangent.z;

    finalNormal = finalNormal.normalize();
    // 计算最终法线，切线空间扰动 → 世界坐标系下计算光照使用
    // =======================================================

    color3f result = blinnPhong(baseColor, specColor, frag_WorldPos, finalNormal);
    // 调用封装函数计算 Blinn-Phong 光照。

    TGAColor color = toTGAColor(result);
    // 将 vec3f 转换成 TGAColor 类型，以在 tga 图像中显示。

    // vec3f n = sampleNormalTangent(bary_uv_coordinate);
    // vec3f debug = n * 0.5f + vec3f(0.5f, 0.5f, 0.5f);
    // return {false, toTGAColor(debug)};
    return {false, color};
}

void Blinn_PhongShader::setColor(const TGAColor& _color) {
    color = _color;
}

int Blinn_PhongShader::clampTexelX(float u, const TGAImage& image) const {
    if (image.width() <= 0) return 0;
    int x = static_cast<int>(u * image.width());
    return std::clamp(x, 0, image.width() - 1);
}

int Blinn_PhongShader::clampTexelY(float v, const TGAImage& image) const {
    if (image.height() <= 0) return 0;
    int y = static_cast<int>(v * image.height());
    return std::clamp(y, 0, image.height() - 1);
}

float Blinn_PhongShader::sampleAlpha(const uv2f& uv) const {
    // 获取透明通道值，目前用于 Alpha-Testing
    if (diffusemap.width() <= 0 || diffusemap.height() <= 0) {
        return 1.0f;
    }

    if (diffusemap.getBpp() < TGAImage::RGBA) {
        return 1.0f;
    }

    int tx = clampTexelX(uv.x, diffusemap);
    int ty = clampTexelY(uv.y, diffusemap);

    TGAColor color_in_tex = diffusemap.get(tx, ty);

    return color_in_tex[3] / 255.0f;
}

color3f Blinn_PhongShader::sampleDiffuse(const uv2f& uv) const {
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

float Blinn_PhongShader::sampleSpecular(const uv2f& uv) const {
    if (specularmap.width() <= 0 || specularmap.height() <= 0) {
        return 0.0f;
    }

    int tx = clampTexelX(uv.x, specularmap);
    int ty = clampTexelY(uv.y, specularmap);

    TGAColor color_in_tex = specularmap.get(tx, ty);

    // TGAColor 存储顺序是 BGRA。specular map 是单通道时，取任意颜色通道都可以。
    float specColor = color_in_tex[0] / 255.0f;

    return specColor;
}

vec3f Blinn_PhongShader::sampleNormalTangent(const uv2f& uv) const {
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

void Blinn_PhongShader::computeFaceTangent() {
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

color3f Blinn_PhongShader::blinnPhong(
    const color3f& baseColor,
    const float& specularColor,
    const vec3f& frag_WorldPos,
    const normal3f& frag_Normal
) const {
    vec3f lightDir = (lightPosition - frag_WorldPos).normalize();
    vec3f viewDir  = (cameraPos - frag_WorldPos).normalize();

    vec3f halfwayDir = (lightDir + viewDir).normalize();

    // attenuation
    float distance = (lightPosition - frag_WorldPos).norm();

    float constant  = 1.0f;
    float linear    = 0.02f;
    float quadratic = 0.002f;

    float attenuation = 1.0f / (
        constant +
        linear * distance +
        quadratic * distance * distance
    );

    float ambientStrength = 0.10f;
    vec3f ambient = baseColor * ambientStrength;

    float diff = std::max(0.0f, dot(frag_Normal, lightDir));
    vec3f diffuse = baseColor.cwiseproduct(lightColor) * diff;

    float specularStrength = 0.9f * specularColor;
    float shininess = 32.0f;

    float spec = std::pow(std::max(0.0f, dot(frag_Normal, halfwayDir)), shininess);

    vec3f specular = lightColor * specularStrength * spec;
    vec3f result = ambient + (diffuse + specular) * attenuation;

    result.x = std::clamp(result.x, 0.0f, 1.0f);
    result.y = std::clamp(result.y, 0.0f, 1.0f);
    result.z = std::clamp(result.z, 0.0f, 1.0f);

    return result;
}

TGAColor Blinn_PhongShader::toTGAColor(const color3f& result) const {
    TGAColor color{
        static_cast<unsigned char>(result.z * 255.0f),
        static_cast<unsigned char>(result.y * 255.0f),
        static_cast<unsigned char>(result.x * 255.0f),
        255
    };
    return color;
}

Shadow_Blinn_PhongShader::Shadow_Blinn_PhongShader(
    const Model& mesh_,
    const mat4f& modelMatrix_,
    const mat4f& viewMatrix_,
    const mat4f& projectionMatrix_,
    const color3f& lightColor_,
    const vec3f& lightPosition_,
    const vec3f& cameraPos_,
    const z_buffer& shadowBuffer_,
    const mat4f& lightMVP_,
    const TGAImage& diffusemap_,
    const TGAImage& normalmap_,
    const TGAImage& specularmap_
)
    : Blinn_PhongShader(
          mesh_,
          modelMatrix_,
          viewMatrix_,
          projectionMatrix_,
          lightColor_,
          lightPosition_,
          cameraPos_,
          diffusemap_,
          normalmap_,
          specularmap_
      ),
      shadowBuffer(shadowBuffer_),
      lightMVP(lightMVP_),
      bias(0.005f)
{
    shadowHeight = shadowBuffer.size();
    shadowWidth = shadowHeight > 0 ? shadowBuffer[0].size() : 0;
    Lightviewport = Viewport(shadowWidth, shadowHeight);
}

float Shadow_Blinn_PhongShader::getShadowDepth(int x, int y) const{
    return shadowBuffer[x][y];
}

color3f Shadow_Blinn_PhongShader::shadow_BlinnPhong(
        const color3f& baseColor,
        const float& specularColor,
        const vec3f& frag_WorldPos,
        const normal3f& frag_Normal
)const {
    vec3f lightDir = (lightPosition - frag_WorldPos).normalize();
    vec3f viewDir  = (cameraPos - frag_WorldPos).normalize();

    vec3f halfwayDir = (lightDir + viewDir).normalize();

    // attenuation
    float distance = (lightPosition - frag_WorldPos).norm();

    float constant  = 1.0f;
    float linear    = 0.02f;
    float quadratic = 0.002f;

    float attenuation = 1.0f / (
        constant +
        linear * distance +
        quadratic * distance * distance
    );

    float ShadowFactor = 1.0f;
    vec3f lightDirectionPos = (lightMVP * vec4f(frag_WorldPos.x, frag_WorldPos.y, frag_WorldPos.z, 1.0f)).to_vec3();
                                                                                                // 该函数已经有/w逻辑
    vec3f lightScreenPos = TransformPoint(Lightviewport, lightDirectionPos);
    // 使用世界坐标下的片段位置进行MVP+透视除法+视窗变换，得到光源视角的屏幕坐标

    int sx = static_cast<int>(lightScreenPos.x);
    int sy = static_cast<int>(lightScreenPos.y);
    float currentDepth = lightScreenPos.z;
    // 简易变量名

    if (sx >= 0 && sx <= shadowWidth && sy >= 0 && sy < shadowHeight){
        if(getShadowDepth(sx, sy) < currentDepth - bias)
            // 采用的是z值越小越前的逻辑
            ShadowFactor = 0.3f;
    }

    float ambientStrength = 0.10f;
    vec3f ambient = baseColor * ambientStrength;

    float diff = std::max(0.0f, dot(frag_Normal, lightDir));
    vec3f diffuse = baseColor.cwiseproduct(lightColor) * diff;

    float specularStrength = 0.9f * specularColor;
    float shininess = 32.0f;

    float spec = std::pow(std::max(0.0f, dot(frag_Normal, halfwayDir)), shininess);

    vec3f specular = lightColor * specularStrength * spec;
    vec3f result = ambient + (diffuse + specular) * attenuation * ShadowFactor;

    result.x = std::clamp(result.x, 0.0f, 1.0f);
    result.y = std::clamp(result.y, 0.0f, 1.0f);
    result.z = std::clamp(result.z, 0.0f, 1.0f);

    return result;
}

std::pair<bool, TGAColor> Shadow_Blinn_PhongShader::fragment(const vec3f& bar) const {
    // 有阴影的片段着色器着色函数
    float inv_w0 = 1.0f / gl_Position[0].w;
    float inv_w1 = 1.0f / gl_Position[1].w;
    float inv_w2 = 1.0f / gl_Position[2].w;

    float z = bar.x * inv_w0 + bar.y * inv_w1 + bar.z * inv_w2;

    vec3f bc(
        bar.x * inv_w0 / z,
        bar.y * inv_w1 / z,
        bar.z * inv_w2 / z
    );

    vec3f frag_WorldPos =
        varying_worldPos[0] * bc.x +
        varying_worldPos[1] * bc.y +
        varying_worldPos[2] * bc.z;

    normal3f frag_Normal = (
        varying_normal[0] * bc.x +
        varying_normal[1] * bc.y +
        varying_normal[2] * bc.z
    ).normalize();

    uv2f bary_uv_coordinate =
        varying_uv[0] * bc.x +
        varying_uv[1] * bc.y +
        varying_uv[2] * bc.z;

    float alpha = sampleAlpha(bary_uv_coordinate);

    if (alpha < 0.1f) {
        // Alpha-Testing 丢弃高透明度部分纹理
        return {true, TGAColor{0, 0, 0, 0}};
    }

    color3f baseColor = sampleDiffuse(bary_uv_coordinate);
    // 从 diffuse map 中获取基础颜色。
    float specColor = sampleSpecular(bary_uv_coordinate);

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

    color3f result = shadow_BlinnPhong(baseColor, specColor, frag_WorldPos, finalNormal);

    TGAColor color = toTGAColor(result);

    return {false, color};
}

ShadowDepthCalcShader::ShadowDepthCalcShader(
    const Model& mesh_,
    const mat4f& modelMatrix_,
    const mat4f& viewMatrix_,
    const mat4f& projectionMatrix_
)
    : mesh(mesh_),
      modelMatrix(modelMatrix_),
      viewMatrix(viewMatrix_),
      projectionMatrix(projectionMatrix_)
{}

vec4f ShadowDepthCalcShader::vertex(int faceIndex, int vertexIndex) {
    vec3f v = mesh.vert(faceIndex, vertexIndex);
    return projectionMatrix * viewMatrix * modelMatrix * vec4f(v.x, v.y, v.z, 1.0f);
}

std::pair<bool, TGAColor> ShadowDepthCalcShader::fragment(const vec3f& bar) const {
    return {false, TGAColor{255, 255, 255, 255}};
    // 还是要实现一个假的片元着色器函数否则父类纯虚函数不能实例化
}