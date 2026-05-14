#include "transformation.h"

float Radians(float degree) {
    // 弧度制/角度制转换
    return degree * 3.14159265358979323846f / 180.0f;
}

// ==================== Model Transformations ====================

// 平移矩阵
mat4f Translate(float tx, float ty, float tz) {
    return mat4f{
        1, 0, 0, tx,
        0, 1, 0, ty,
        0, 0, 1, tz,
        0, 0, 0, 1
    };
}
// 重载一版
mat4f Translate(const vec3f& t) {
    return Translate(t.x, t.y, t.z);
}

// 缩放矩阵
mat4f Scale(float sx, float sy, float sz) {
    return mat4f{
        sx, 0,  0,  0,
        0,  sy, 0,  0,
        0,  0,  sz, 0,
        0,  0,  0,  1
    };
}
mat4f Scale(float s) {
    return Scale(s, s, s);
}

mat4f RotateX(float deg) {
    float rad = Radians(deg);
    float c = std::cos(rad);
    float s = std::sin(rad);

    return mat4f{
        1, 0,  0, 0,
        0, c, -s, 0,
        0, s,  c, 0,
        0, 0,  0, 1
    };
}

mat4f RotateY(float deg) {
    float rad = Radians(deg);
    float c = std::cos(rad);
    float s = std::sin(rad);

    return mat4f{
         c, 0, s, 0,
         0, 1, 0, 0,
        -s, 0, c, 0,
         0, 0, 0, 1
    };
}

mat4f RotateZ(float deg) {
    float rad = Radians(deg);
    float c = std::cos(rad);
    float s = std::sin(rad);

    return mat4f{
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1
    };
}

// ==================== View Transformation ====================

// camPosition: 相机位置
// center:      相机看向的目标点
// up:          世界空间中的上方向
//
// 约定：相机标准方向看向 -Z。
// 所以当 camPosition=(0,0,0), center=(0,0,-1), up=(0,1,0) 时，View 应该是单位矩阵。

mat4f LookAt(const vec3f& camPosition, const vec3f& center, const vec3f& up) {
    vec3f g = (center - camPosition).normalize();        // 观察方向
    vec3f right = cross(g, up).normalize();              // 右方向
    vec3f cameraUp = cross(right, g).normalize();

    mat4f R{
         right.x,     right.y,     right.z,     0,
         cameraUp.x,  cameraUp.y,  cameraUp.z,  0,
        -g.x,        -g.y,        -g.z,         0,
         0,           0,           0,           1
    };

    mat4f T = Translate(-camPosition.x, -camPosition.y, -camPosition.z);

    return R * T;
}

mat4f Orthographic(float fovY_degree,
                   float aspect,
                   float nearDistance,
                   float farDistance) {
    float n = -nearDistance;
    float f = -farDistance;

    float top = nearDistance * std::tan(Radians(fovY_degree) * 0.5f);
    float right = aspect * top;

    return mat4f{
        1.0f / right, 0,          0,            0,
        0,            1.0f / top, 0,            0,
        0,            0,          2.0f / (n-f), (-n-f) / (n-f),
        0,            0,          0,            1
    };
}
mat4f Orthographic(float l, float r,
                   float b, float t,
                   float n, float f) {
    return mat4f{
        2.0f / (r - l), 0,              0,                -(r + l) / (r - l),
        0,              2.0f / (t - b), 0,                -(t + b) / (t - b),
        0,              0,              2.0f / (n - f),   -(n + f) / (n - f),
        0,              0,              0,                1
    };
}

mat4f PerspectiveToOrthographic(float n, float f) {
    return mat4f{
        n, 0,     0,      0,
        0, n,     0,      0,
        0, 0, n + f, -n * f,
        0, 0,     1,      0
    };
}

// ==================== Perspective Projection ====================
// 使用 fovY + aspect + near/far 距离来生成透视矩阵
// nearDistance / farDistance 标定了视锥体的近远平面
// nearDistance / farDistance 是正数，例如 0.1f, 100.0f
mat4f Perspective(float fovY_degree,
                         float aspect,
                         float nearDistance,
                         float farDistance) {
    float n = -nearDistance;
    float f = -farDistance;

    float top = nearDistance * std::tan(Radians(fovY_degree) * 0.5f);
    float right = aspect * top;
    return Orthographic(fovY_degree, aspect, nearDistance, farDistance)
           *PerspectiveToOrthographic(n, f);
}

// ==================== 视口变换 ====================
mat4f Viewport(vec4f vec, int w, int h){
    return mat4f{
        0.5f*w,      0,  0,         vec.x+0.5f*w,
             0, 0.5f*h,  0,         vec.y+0.5f*h,
             0,      0, -0.5f,      0.5f,
             0,      0,  0,         1
        };
}

mat4f Viewport(float x, float y, float width, float height) {
    return mat4f{
        width * 0.5f, 0,             0,     x + width * 0.5f,
        0,            height * 0.5f, 0,     y + height * 0.5f,
        0,            0,            -0.5f,  0.5f,
        0,            0,             0,     1
    };
}

mat4f Viewport(float width, float height) {
    return Viewport(0.0f, 0.0f, width, height);
}

// ==================== 三维点/向量的变换函数 ====================

vec3f TransformPoint(const mat4f& m, const vec3f& p) {
    vec4f hp(p.x, p.y, p.z, 1.0f);
    return (m * hp).to_vec3();
}

vec3f TransformVector(const mat4f& m, const vec3f& v) {
    vec4f hv(v.x, v.y, v.z, 0.0f);
    vec4f result = m * hv;
    return vec3f(result.x, result.y, result.z);
}