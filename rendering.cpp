#include "rendering.h"

void BresenhamLineDraw(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
    bool steep = std::abs(ax - bx) < std::abs(ay - by);
    // 首先判断该直线是否是陡峭的[斜率绝对值大于1]
    if (steep) {
        // 如果直线是陡峭的，我们将转置图片
        // 将预绘制的线段x,y坐标对调，此后我们通过Bresenham算法选择的每个像素坐标都是转置后的
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax > bx) {
        // 自左向右绘制
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int dy = std::abs(by - ay);
    int dx = bx - ax;  // 前面已经确保自左向右绘制
    int pk = -1 * dx;
    // 不同于理论笔记中的2*dy，由于该函数循环中必定先绘制端点坐标像素，因此误差从0开始计算
    for (int x = ax; x <= bx; x++) {
        if (steep)
            // 如果转置了绘制线段，则最终绘制时再度转置
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        pk += 2 * dy;
        y += (by > ay ? 1 : -1) * (pk >= 0);
        pk -= 2 * dx * (pk >= 0);
        // 如果相对误差pk >= 0，则选择东北/东南方向像素[y坐标递进±1]，同时消除2dx误差
    }
}

void TriangleDraw(point3f A, point3f B, point3f C, TGAImage& framebuffer, TGAColor linecolor, TGAColor fillcolor) {
    BresenhamLineDraw(A.x, A.y, B.x, B.y, framebuffer, linecolor);
    BresenhamLineDraw(A.x, A.y, C.x, C.y, framebuffer, linecolor);
    BresenhamLineDraw(C.x, C.y, B.x, B.y, framebuffer, linecolor);
    Rasterization(A, B, C, framebuffer, fillcolor);
}

void DrawWireFrame(const Model& model, TGAImage& image, int width, int height, TGAColor dotcolor, TGAColor linecolor) {
    // 输入obj模型与要绘入的帧缓冲属性
    // 可指定端点颜色、线段颜色
    auto vertices = model.vertices();
    for (auto& ver : vertices) {
        ver.x = (ver.x + 1.0) * width / 2.0;
        ver.y = (ver.y + 1.0) * height / 2.0;
        // 视口变换
    }

    for (const auto& frag : model.faces()) {
        vec3i face = frag.v_idx;
        TriangleDraw(vertices[face.x], vertices[face.y], vertices[face.z], image, linecolor, transparent);
        image.set(vertices[face.x].x, vertices[face.x].y, dotcolor);
        image.set(vertices[face.y].x, vertices[face.y].y, dotcolor);
        image.set(vertices[face.z].x, vertices[face.z].y, dotcolor);
    }
}

void DrawFillFrame(const Model& model, TGAImage& image, int width, int height, TGAColor dotcolor, TGAColor linecolor,
                   TGAColor fillcolor) {
    auto vertices = model.vertices();
    for (auto& ver : vertices) {
        ver.x = (ver.x + 1.0) * width / 2.0;
        ver.y = (ver.y + 1.0) * height / 2.0;
        // 视口变换
    }

    for (const auto& frag : model.faces()) {
        fillcolor = {0, 0, 0, 0};
        vec3i face = frag.v_idx;
        TriangleDraw(vertices[face.x], vertices[face.y], vertices[face.z], image, linecolor, fillcolor);
        image.set(vertices[face.x].x, vertices[face.x].y, dotcolor);
        image.set(vertices[face.y].x, vertices[face.y].y, dotcolor);
        image.set(vertices[face.z].x, vertices[face.z].y, dotcolor);
    }
}

void Rasterization(point3f A, point3f B, point3f C, TGAImage& framebuffer, TGAColor color) {
    // 利用向量叉乘的正负性进行光栅化判断

    int ax = A.x, ay = A.y;
    vec2 a(ax, ay);
    int bx = B.x, by = B.y;
    vec2 b(bx, by);
    int cx = C.x, cy = C.y;
    vec2 c(cx, cy);

    int x_min = std::min(std::min(ax, bx), cx);
    int y_min = std::min(std::min(ay, by), cy);
    int x_max = std::max(std::max(ax, bx), cx);
    int y_max = std::max(std::max(ay, by), cy);
    // 标定光栅化的Boundnig-Box

    for (int x = x_min; x <= x_max; x++) {
        for (int y = y_min; y <= y_max; y++) {
            vec2 p(x, y);
            bool inTriangle = cross(p - c, c - a) > 0 && cross(a - p, b - a) > 0 && cross(p - b, b - c) > 0;
            if (inTriangle) {
                framebuffer.set(x, y, color);
            }
        }
    }
}