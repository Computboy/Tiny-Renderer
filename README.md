# Tiny-Renderer
不直接调用OpenGL的API，使用纯Cpp语言实现小型CPU图像渲染器

原仓库：[tinyrenderer](https://github.com/ssloy/tinyrenderer)

重新编译：
```bash
cmake --build build && ./build/MyTinyRenderer
```

# ConstructionLog-构建日志

## Day 1：像素绘制与线段光栅化

从零搭建渲染器的第一步，是解决"如何将像素写入图像"这一最基础的问题。项目直接复用了 tinyrenderer 中的 [TGAImage](https://haqr.eu/tinyrenderer/#the-starting-point) 类，作为轻量级的图像缓冲区与 `.tga` 格式输出工具——这一阶段的关注点并非图像文件格式本身，而是手动实现像素级的绘制逻辑，为后续渲染管线铺路。

在此基础上，实现了经典的 [Bresenham 线段绘制算法](https://haqr.eu/tinyrenderer/bresenham/)，完成了直线段的光栅化。这是光栅化渲染器的第一个可见成果：无需任何图形 API，仅凭数学推导就能在屏幕上画出一条直线。

## Day 2：OBJ 模型导入与线框绘制

有了线段绘制能力后，下一步是将三维几何数据引入管线。这一阶段手动实现了一个简易的 OBJ 文件解析器，能够读取 `.obj` 格式的模型文件，提取顶点坐标与面索引信息。随后结合透视投影与 Bresenham 线段绘制，将模型的三角面以线框（wireframe）形式渲染到屏幕上，首次完成了从三维数据到二维图像端到端的完整流程。

## Day 3：三角形光栅化与背面剔除

[光栅化-Rasterization]()
我们已经有了三角形三边的绘制算法（Bresenham线段绘制），那我们应该如何去填充这些三角形呢？
直接使用**叉乘检查向量是否位于三角形内部**的方式
<div align="center">
  <img src="attachments/光栅化.png" width="520">
</div>

