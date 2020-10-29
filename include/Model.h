#ifndef MODEL_H
#define MODEL_H

#include "Shader.h"
#include "Mesh.h"
#include "Animator.h"

#include <string>
#include <vector>
#include <map>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

class Model {
private:
	std::vector<Texture> loaded_textures;
	std::vector<Mesh> meshes;
	std::string dir;
	const aiScene* scene;
	std::vector<unsigned int> baseVertex;
	unsigned int totalVertices = 0;
	Animator *animator;
	//bones
	std::vector<VertexBoneData> bones;

	//loading model methods
	void loadModel(const std::string &path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(unsigned int meshId, aiMesh *mesh, const aiScene *scene);
	std::vector<Texture> getMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
public:
	Model(const char *path);
	void draw(Shader& shader);

	//animation
	void playAnimation(float time, Shader& shader);
};

#endif