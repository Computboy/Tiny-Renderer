#pragma once
#include <cmath>

#include "geometry.h"
#include "tgaimage.h"
#include "tinyobjloader.h"

void BresenhamLineDraw(int ax, int ay, int bx, int by, TGAImage& image, TGAColor color);

void TriangleDraw(point3f A, point3f B, point3f C, TGAImage& framebuffer, TGAColor linecolor, TGAColor fillcolor);

void DrawWireFrame(const Model& model, TGAImage& image, int width, int height, TGAColor dotcolor = red,
                   TGAColor linecolor = white);
void DrawFillFrame(const Model& model, TGAImage& image, int width, int height, TGAColor dotcolor = red,
                   TGAColor linecolor = white, TGAColor fillcolor = transparent);

void Rasterization(point3f A, point3f B, point3f C, TGAImage& framebuffer, TGAColor color, int width, int height);

float triangle_area(int ax, int ay, int bx, int by, int cx, int cy);
float triangle_area(vec2i A, vec2i B, vec2i C); // 重载一版