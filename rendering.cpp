#include "rendering.h"
#include <random>

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

void TriangleDraw(point3f A, point3f B, point3f C, TGAImage& framebuffer, TGAColor linecolor) {
    BresenhamLineDraw(A.x, A.y, B.x, B.y, framebuffer, linecolor);
    BresenhamLineDraw(A.x, A.y, C.x, C.y, framebuffer, linecolor);
    BresenhamLineDraw(C.x, C.y, B.x, B.y, framebuffer, linecolor);
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
        TriangleDraw(vertices[face.x], vertices[face.y], vertices[face.z], image, linecolor);
        image.set(vertices[face.x].x, vertices[face.x].y, dotcolor);
        image.set(vertices[face.y].x, vertices[face.y].y, dotcolor);
        image.set(vertices[face.z].x, vertices[face.z].y, dotcolor);
    }
}

void DrawFillFrame(const Model& model, TGAImage& image, z_buffer& zbuffer, int width, int height, TGAColor dotcolor, TGAColor linecolor,
                   TGAColor fillcolor) {
    auto vertices = model.vertices();
    for (auto& ver : vertices) {
        ver.x = (ver.x + 1.0) * width / 2.0;
        ver.y = (ver.y + 1.0) * height / 2.0;
        ver.z = (-ver.z + 1.0f) * 0.5f;  // 重点：深度是近小远大 转成[0,1]区间
        // 视口变换
    }
    for (const auto& frag : model.faces()) {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, 255);

        fillcolor = {
            static_cast<unsigned char>(dist(rng)), // B
            static_cast<unsigned char>(dist(rng)), // G
            static_cast<unsigned char>(dist(rng)), // R
            255                                    // A
        };
        vec3i face = frag.v_idx;
        Rasterization(vertices[face.x], vertices[face.y], vertices[face.z], image, zbuffer, fillcolor);
    }
}

void Rasterization(point3f A, point3f B, point3f C, TGAImage& framebuffer, z_buffer& zbuffer,
                   TGAColor color) {

    // 利用向量叉乘的正负性进行光栅化判断
    float ax = A.x, ay = A.y, az = A.z;
    vec2f a(ax, ay);
    float bx = B.x, by = B.y, bz = B.z;
    vec2f b(bx, by);
    float cx = C.x, cy = C.y, cz = C.z;
    vec2f c(cx, cy);

    float total_area = triangle_area(a,b,c);
    // 鞋带公式计算三角形面积

    int x_min = std::min(std::min(ax, bx), cx);
    int y_min = std::min(std::min(ay, by), cy);
    int x_max = std::max(std::max(ax, bx), cx);
    int y_max = std::max(std::max(ay, by), cy);

    x_min = std::max(0, x_min);
    y_min = std::max(0, y_min);
    x_max = std::min(framebuffer.width() - 1, x_max);
    y_max = std::min(framebuffer.height() - 1, y_max);
    // 标定光栅化的Boundnig-Box，且防止越界

    for (int x = x_min; x <= x_max; x++) {
        for (int y = y_min; y <= y_max; y++) {
            vec2f p(static_cast<float>(x), static_cast<float>(y));
            float w0 = cross(b - a, p - a);
            float w1 = cross(c - b, p - b);
            float w2 = cross(a - c, p - c);

            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                // 则证明是内部像素
                float alpha = triangle_area(p,b,c) / total_area;
                float beta  = triangle_area(p,a,c) / total_area;
                float gamma = 1.0f - alpha - beta;
                // 利用面积计算中心坐标
                // TGAColor interpolatecolor = interpolate(alpha, beta, gamma, red, green, blue);
                // framebuffer.set(x, y, interpolatecolor); 插值的颜色
                float depth = alpha * az + beta * bz + gamma * cz;
                if(depth < zbuffer[x][y]){
                    // 当前插值出来的深度小于深度缓冲区中的（说明是最前）
                    framebuffer.set(x, y, color);
                    zbuffer[x][y] = depth;
                    // 绘制并更新
                }
            }
        }
    }
}

float triangle_area(float ax, float ay, float bx, float by, float cx, float cy){
    return std::abs(0.5f * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)));
    // 鞋带公式(其实就是向量叉乘)计算三角形面积
}

float triangle_area(vec2f A, vec2f B, vec2f C){
    return std::abs(0.5f * (A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y)));
}