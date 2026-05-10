#pragma once

#include <vector>
#include <string>
#include "geometry.h"

struct Fragment {
    // 对应 f a/b/c d/e/f g/h/i 注意索引从1开始
    vec3i v_idx;   // 三个顶点位置索引
    vec3i vt_idx;  // 三个纹理坐标索引
    vec3i vn_idx;  // 三个法线索引
};

class Model {
public:
    Model(const std::string& filename) {
        load(filename);
    }
    // 构造函数直接 Model test("diablo3_pose.obj"); 非常方便

    bool load(const std::string& filename);
    // 从.obj文件中进行读取

    const std::vector<point3f>& vertices() const {
        return vertices_;
    }

    const std::vector<Fragment>& faces() const {
        return faces_;
    }

private:
    std::vector<point3f> vertices_;
    // 储存obj文件顶点的动态数组
    std::vector<Fragment> faces_;
    // 储存obj文件每个面的动态数组
};