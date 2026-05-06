# Tiny-Renderer
不直接调用OpenGL的API，使用纯Cpp语言实现小型CPU图像渲染器

原仓库：[tinyrenderer](https://github.com/ssloy/tinyrenderer)

# Included

## set pixel and line drawing

[TGAImage 类](https://haqr.eu/tinyrenderer/#the-starting-point)

初期直接复用 tinyrenderer 的 `TGAImage` 类，作为最小依赖的图像缓冲区与 `.tga` 输出工具。现阶段重点不在图像文件格式本身，而在手动实现像素绘制、线段光栅化与后续渲染管线。

[Bresenham线段绘制算法](https://haqr.eu/tinyrenderer/bresenham/)
