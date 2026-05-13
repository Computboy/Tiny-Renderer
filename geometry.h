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

    T norm() const {
        // 向量范数(模长)
        return std::sqrt(x * x + y * y + z * z);
    }

    vec3<T> normalize() const {
        T n = norm();
        if (n == 0) return vec3<T>(0, 0, 0);
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

using vec3i = vec3<int>;
using vec3d = vec3<double>;

using vec2f = vec2<float>;
using point2f = vec2<float>;

using vec2i = vec2<int>;
using vec2d = vec2<double>;
