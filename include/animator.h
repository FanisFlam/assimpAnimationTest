#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <glad/glad.h>
#include "mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct BoneInfo {
	glm::mat4 offset;
	glm::mat4 finalTransform;
};

class Animator {
private:
	const aiScene *scene;
	std::map<std::string, unsigned int> boneMap;
	unsigned int numBones = 0;
	glm::mat4 globalITransform;


	std::vector<BoneInfo> boneInfo;
	//private methods
	void readNodeHeirarchy(float animationTime, const aiNode* node, const glm::mat4 &parentTransform);
	const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string nodeName);

	unsigned int findScaling(float animationTime, const aiNodeAnim* nodeAnim);
	unsigned int findRotation(float animationTime, const aiNodeAnim* nodeAnim);
	unsigned int findPosition(float animationTime, const aiNodeAnim* nodeAnim);

	void calcInterpolatedScaling(aiVector3D &out, float animationTime, const aiNodeAnim* nodeAnim);
	void calcInterpolatedRotation(aiQuaternion &out, float animationTime, const aiNodeAnim* nodeAnim);
	void calcInterpolatedPosition(aiVector3D &out, float animationTime, const aiNodeAnim* nodeAnim);
public:
	Animator(const aiScene *scene);
	void loadBones(Mesh &mesh, unsigned int i);
	void boneTransform(float timeInSeconds, std::vector<glm::mat4> &transforms);

};

#endif