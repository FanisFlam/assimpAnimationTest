#ifndef MODEL_H
#define MODEL_H

#include "shader.h"
#include "mesh.h"
#include "animator.h"

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
    std::vector<Texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    const aiScene* scene;
    Animator *animator;
    Assimp::Importer m_Importer;
    //loading model methods
    void loadModel(const std::string &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

public:
    Model(std::string const &path);
    void draw(Shader &shader);

    //animation
    void playAnimation(float time, Shader& shader);
};

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma=false);

#endif