#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include "tinyobjloader.h"

void BresenhamLineDraw(int ax, int ay, int bx, int by, TGAImage &image, TGAColor color);

void TriangleDraw(point3f A, point3f B, point3f C, TGAImage &framebuffer, TGAColor color);

void DrawWireFrame(const Model& model, TGAImage& image, TGAColor dotcolor, TGAColor linecolor, int width, int height);
