#pragma once
#include "geometry.h"
#include <cmath>

float Radians(float degree);

// Model transformations
mat4f Translate(float tx, float ty, float tz);
mat4f Translate(const vec3f& t);

mat4f Scale(float sx, float sy, float sz);
mat4f Scale(float s);

mat4f RotateX(float deg);
mat4f RotateY(float deg);
mat4f RotateZ(float deg);

// View transformation
mat4f LookAt(const vec3f& camPosition, const vec3f& center, const vec3f& up);

// Projection
mat4f Orthographic(float fovY_degree, float aspect, float nearDistance, float farDistance);
mat4f Orthographic(float l, float r, float b, float t, float n, float f);
mat4f PerspectiveToOrthographic(float n, float f);
mat4f Perspective(float fovY_degree, float aspect, float nearDistance, float farDistance);
mat4f Viewport(vec4f vec, int w, int h);

// Point/vector transformation
vec3f TransformPoint(const mat4f& m, const vec3f& p);
vec3f TransformVector(const mat4f& m, const vec3f& v);

// Viewport transformation
mat4f Viewport(float x, float y, float width, float height);
mat4f Viewport(float width, float height);
