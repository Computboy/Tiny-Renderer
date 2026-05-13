#pragma once
#include <cmath>

#include "geometry.h"
#include "tgaimage.h"
#include "tinyobjloader.h"

void BresenhamLineDraw(int ax, int ay, int bx, int by, TGAImage& image, TGAColor color);

void TriangleDraw(point3f A, point3f B, point3f C, TGAImage& framebuffer, TGAColor linecolor);

void DrawWireFrame(const Model& model, TGAImage& image, int width, int height, TGAColor dotcolor = red,
                   TGAColor linecolor = white);
void DrawFillFrame(const Model& model, TGAImage& image, z_buffer& zbuffer, int width, int height, 
    TGAColor dotcolor = red, TGAColor linecolor = white, TGAColor fillcolor = transparent);

void Rasterization(point3f A, point3f B, point3f C, TGAImage& framebuffer, z_buffer& zbuffer, TGAColor color);

float triangle_area(float ax, float ay, float bx, float by, float cx, float cy);
float triangle_area(vec2f A, vec2f B, vec2f C); // 重载一版