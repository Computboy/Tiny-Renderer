#pragma once
#include "tgaimage.h"
#include "geometry.h"

void BresenhamLineDraw(int ax, int ay, int bx, int by, TGAImage &image, TGAColor color);

void TriangleDraw(point3f A, point3f B, point3f C, TGAImage &framebuffer, TGAColor color);