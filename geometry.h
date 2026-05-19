#pragma once
#include <cmath>
#include <iostream>
#include <vector>

using z_buffer = std::vector<std::vector<float>>;

// ==================== 三维向量 vec3 ====================
template <typename T>
// 定义一个抽象的模板类
struct vec3 {
    T x, y, z;  // 向量的x, y, z分量

    vec3() : x(0), y(0), z(0) {}
    vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    T& operator[](int i) {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    const T& operator[](int i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    vec3<T> operator+(const vec3<T>& other) const { return vec3<T>(x + other.x, y + other.y, z + other.z); }

    vec3<T> operator-(const vec3<T>& other) const { return vec3<T>(x - other.x, y - other.y, z - other.z); }

    vec3<T> operator*(T value) const { return vec3<T>(x * value, y * value, z * value); }

    vec3<T> operator/(T value) const { return vec3<T>(x / value, y / value, z / value); }

    vec3<T> cwiseproduct(const vec3<T>& other) const {
        return vec3<T>(x * other.x, y * other.y, z * other.z);
    }

    T norm() const {
        // 向量范数(模长)
        return std::sqrt(x * x + y * y + z * z);
    }

    vec3<T> normalize() const {
        T n = norm();
        if (std::abs(n) < 1e-6) return vec3<T>(0, 0, 0);
        return vec3<T>(x / n, y / n, z / n);
    }
};

template <typename T>
T dot(const vec3<T>& a, const vec3<T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
// 向量的点乘

template <typename T>
vec3<T> cross(const vec3<T>& a, const vec3<T>& b) {
    return vec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
// 向量的叉乘

template <typename T>
std::ostream& operator<<(std::ostream& out, const vec3<T>& v) {
    out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return out;
}
// 向量的输出调试


// ==================== 四维向量 vec4 ====================
template <typename T>
struct vec4 {
    T x, y, z, w;  // 四维分量（w通常用于齐次坐标）

    // 构造函数
    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    // 下标访问（支持读写）
    T& operator[](int i) {
        switch(i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: return w;
        }
    }

    // 下标访问（只读）
    const T& operator[](int i) const {
        switch(i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: return w;
        }
    }

    // 算术运算符重载
    vec4<T> operator+(const vec4<T>& other) const {
        return vec4<T>(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    vec4<T> operator-(const vec4<T>& other) const {
        return vec4<T>(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    vec4<T> operator*(T value) const {
        return vec4<T>(x * value, y * value, z * value, w * value);
    }

    vec4<T> operator/(T value) const {
        return vec4<T>(x / value, y / value, z / value, w / value);
    }

    // 向量范数（模长）
    T norm() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    // 归一化（单位向量）
    vec4<T> normalize() const {
        T n = norm();
        return (n == 0) ? vec4<T>(0, 0, 0, 0) : *this / n;
    }

    // 齐次坐标转三维向量（w≠0时）
    vec3<T> to_vec3() const {
        if (w == 0) return vec3<T>(x, y, z);
        return vec3<T>(x/w, y/w, z/w);
    }
};

// 四维点乘（内积）
template <typename T>
T dot(const vec4<T>& a, const vec4<T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// 四维向量输出（调试用）
template <typename T>
std::ostream& operator<<(std::ostream& out, const vec4<T>& v) {
    out << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return out;
}

// ==================== 二维向量 vec2 ====================
template <typename T>
struct vec2 {
    T x, y;

    vec2() : x(0), y(0) {}
    vec2(T x, T y) : x(x), y(y) {}

    T& operator[](int i) {
        if (i == 0) return x;
        return y;
    }

    const T& operator[](int i) const {
        if (i == 0) return x;
        return y;
    }

    vec2<T> operator+(const vec2<T>& other) const { return vec2<T>(x + other.x, y + other.y); }

    vec2<T> operator-(const vec2<T>& other) const { return vec2<T>(x - other.x, y - other.y); }

    vec2<T> operator*(T value) const { return vec2<T>(x * value, y * value); }

    vec2<T> operator/(T value) const { return vec2<T>(x / value, y / value); }

    T norm() const { return std::sqrt(x * x + y * y); }

    vec2<T> normalize() const {
        T n = norm();
        return (n == 0) ? vec2<T>(0, 0) : *this / n;
    }
};

// 二维点乘
template <typename T>
T dot(const vec2<T>& a, const vec2<T>& b) {
    return a.x * b.x + a.y * b.y;
}

// 二维叉乘（判断方向/是否共线）
template <typename T>
T cross(const vec2<T>& a, const vec2<T>& b) {
    return a.x * b.y - a.y * b.x;
}

// 二维输出
template <typename T>
std::ostream& operator<<(std::ostream& out, const vec2<T>& v) {
    out << "(" << v.x << ", " << v.y << ")";
    return out;
}

// ==================== 类型别名定义 ====================
using vec3f = vec3<float>;
using point3f = vec3<float>;
using color3f = vec3<float>;
using normal3f = vec3<float>;

using vec3i = vec3<int>;
using vec3d = vec3<double>;

using vec2f = vec2<float>;
using point2f = vec2<float>;

using vec2i = vec2<int>;
using vec2d = vec2<double>;

using vec4f = vec4<float>;
using point4f = vec4<float>;
using color4f = vec4<float>;

using vec4i = vec4<int>;
using vec4d = vec4<double>;

// -------------- Mat4矩阵 -----------------
template <typename T>
struct mat4 {
    T m[4][4] = {};

    static mat4<T> identity() {
        mat4<T> result;
        for (int i = 0; i < 4; i++) {
            result[i][i] = 1;
        }
        return result;
    }

    T* operator[](int i) {
        return m[i];
    }

    const T* operator[](int i) const {
        return m[i];
    }
};

using mat4f = mat4<float>;

// ====== Matrices-Multiplication 矩阵乘矩阵 ======
template <typename T>
mat4<T> operator*(const mat4<T>& a, const mat4<T>& b) {
    mat4<T> result;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return result;
}
// ====== Matrix-Vector-Multiplication 矩阵乘向量 ======
template <typename T>
vec4<T> operator*(const mat4<T>& m, const vec4<T>& v) {
    return vec4<T>(
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
        m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
    );
}

// -------------- Mat3矩阵 -----------------
template <typename T>
struct mat3 {
    T m[3][3] = {};

    static mat3<T> identity() {
        mat3<T> result;
        for (int i = 0; i < 3; i++) {
            result[i][i] = 1;
        }
        return result;
    }

    T* operator[](int i) {
        return m[i];
    }

    const T* operator[](int i) const {
        return m[i];
    }
};

using mat4f = mat4<float>;

// ====== Matrices-Multiplication 矩阵乘矩阵 ======
template <typename T>
mat3<T> operator*(const mat3<T>& a, const mat3<T>& b) {
    mat3<T> result;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return result;
}
// ====== Matrix-Vector-Multiplication 矩阵乘向量 ======
template <typename T>
vec3<T> operator*(const mat3<T>& m, const vec3<T>& v) {
    return vec3<T>(
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
    );
}

using mat3f = mat3<float>;