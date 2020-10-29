#ifndef MESH_H
#define MESH_H

#define MAX_NUM_BONES 4

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

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

	void addBoneData(unsigned int boneId, float weight) {
		for (unsigned int i = 0; i < sizeof(boneIds); i++) {
			if (weights[i] == 0.0f) {
				boneIds[i] = boneId;
				weights[i] = weight;
				return;
			}
		}
		
		assert(0);
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
	std::vector<VertexBoneData> bones;

	unsigned int VAO, VBO, boneVBO, EBO;

	void loadMesh();
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::vector<VertexBoneData> bones);
	void draw(Shader &shader);
};

#endif