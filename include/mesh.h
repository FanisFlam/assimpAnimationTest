#ifndef MESH_H
#define MESH_H

#define MAX_NUM_BONES 4

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "shader.h"

struct VertexBoneData {
    unsigned int boneIds[MAX_NUM_BONES];
    float weights[MAX_NUM_BONES];

    VertexBoneData() {
        reset();
    }

    void reset() {
        for (unsigned int i = 0; i < MAX_NUM_BONES; i++) {
            boneIds[i] = 0;
            weights[i] = 0;
        }
    }

    void addBoneData(unsigned int BoneID, float weight) {
        for (unsigned int i = 0 ; i < sizeof(boneIds)/sizeof(boneIds[0]); i++) {
            if (weights[i] == 0.0) {
                boneIds[i]     = BoneID;
                weights[i] = weight;
                return;
            }
        }

        // should never get here - more bones than we have space for
        //assert(0);
    }
};

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;


    unsigned int VAO, VBO, boneVBO, EBO;

public:
    void loadMesh();
    std::vector<VertexBoneData> bones;
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void draw(Shader &shader);
};

#endif