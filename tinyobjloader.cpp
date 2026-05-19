#include "tinyobjloader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

bool Model::load(const std::string& filename) {
    std::ifstream in(filename);

    if (!in.is_open()) {
        std::cerr << "Failed to open obj file: " << filename << std::endl;
        return false;
    }

    vertices_.clear();
    faces_.clear();
    // 清空原有数据

    std::string line;

    while (std::getline(in, line)) {
        std::istringstream iss(line);

        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertices_.push_back(point3f(x, y, z));
        } else if (prefix == "vn"){
            float x, y, z;
            iss >> x >> y >> z;
            normals_.push_back(normal3f(x, y, z).normalize());
            // 阅读法线
        } else if (prefix == "f") {
            Fragment face;

            std::string s0, s1, s2;
            iss >> s0 >> s1 >> s2;

            auto parseFaceVertex = [](const std::string& s) {
                vec3i idx(-1, -1, -1);

                std::stringstream ss(s);
                std::string item;

                int part = 0;

                while (std::getline(ss, item, '/')) {
                    if (!item.empty()) {
                        int value = std::stoi(item) - 1;

                        if (part == 0)
                            idx.x = value;  // vertex index
                        else if (part == 1)
                            idx.y = value;  // texture index
                        else if (part == 2)
                            idx.z = value;  // normal index
                    }

                    part++;
                }

                return idx;
            };

            vec3i a = parseFaceVertex(s0);
            vec3i b = parseFaceVertex(s1);
            vec3i c = parseFaceVertex(s2);

            face.v_idx = vec3i(a.x, b.x, c.x);
            face.vt_idx = vec3i(a.y, b.y, c.y);
            face.vn_idx = vec3i(a.z, b.z, c.z);
            // 读取每个面索引

            faces_.push_back(face);
        }
    }

    std::cout << "Loaded OBJ: " << filename << std::endl;
    std::cout << "Vertices: " << vertices_.size() << std::endl;
    std::cout << "Faces: " << faces_.size() << std::endl;

    return true;
}

const point3f Model::vert(int faceIndex, int vertexIndex) const{
    const Fragment& frag = faces_[faceIndex];
    int vertexID = frag.v_idx[vertexIndex];
    return vertices_[vertexID];
}
// 获取第faceIndex个面的第vertexIndex个顶点位置坐标

const normal3f Model::normal(int faceIndex, int normalIndex) const{
    const Fragment& frag = faces_[faceIndex];
    int normalID = frag.vn_idx[normalIndex];
    return normals_[normalID];
}
// 获取第faceIndex个面的第vertexIndex个法线位置坐标