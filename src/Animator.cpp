#include "Animator.h"

glm::mat4 castMat4(const aiMatrix4x4 &mat);
glm::quat castQuat(const aiQuaternion &quat);

Animator::Animator(const aiScene *scene) {
	this->scene = scene;
	globalITransform = glm::inverse(castMat4(scene->mRootNode->mTransformation));
}

void Animator::loadBones(unsigned int meshId, const aiMesh* mesh, std::vector<VertexBoneData> &bones, std::vector<unsigned int> baseVertex) {
	for (unsigned int i = 0; i < mesh->mNumBones; i++) {
		unsigned int boneId = 0;
		std::string boneName(mesh->mBones[i]->mName.data);
		if (boneMap.find(boneName) == boneMap.end()) {
			boneId = numBones;
			numBones++;
			BoneInfo bi;
			boneInfo.push_back(bi);
			boneMap[boneName] = boneId;
			boneInfo[boneId].offset = castMat4(mesh->mBones[i]->mOffsetMatrix);
		}
		else {
			boneId = boneMap[boneName];
		}

		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
			unsigned int vertexId = baseVertex[meshId] + mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			bones[vertexId].addBoneData(boneId, weight);
		}
	}
}

std::vector<glm::mat4> Animator::boneTransform(float timeInSeconds, std::vector<glm::mat4> transforms) {
	glm::mat4 identity = glm::mat4(1.0f);

	unsigned int numPosKeys = scene->mAnimations[0]->mChannels[0]->mNumPositionKeys;
	double animDuration = scene->mAnimations[0]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime;

	float ticksPerSecond = (float)(scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f);

	float timeInTicks = timeInSeconds * ticksPerSecond;
	float animationTime = std::fmod(timeInTicks, animDuration);
	
	readNodeHeirarchy(animationTime, scene->mRootNode, identity);
	transforms.resize(numBones);

	for (unsigned int i = 0; i < numBones; i++) {
		transforms[i] = boneInfo[i].finalTransform;
	}

	return transforms;
}

void Animator::readNodeHeirarchy(float animationTime, const aiNode* node, const glm::mat4& parentTransform) {
	std::string nodeName(node->mName.data);

	const aiAnimation* animation = scene->mAnimations[0];

	glm::mat4 nodeTransformation = castMat4(node->mTransformation);

	const aiNodeAnim* nodeAnim = findNodeAnim(animation, nodeName);

	if (nodeAnim) {
		aiVector3D scaling;
		calcInterpolatedScaling(scaling, animationTime, nodeAnim);
		glm::vec3 scale = glm::vec3(scaling.x, scaling.y, scaling.z);
		glm::mat4 scalingM = glm::scale(glm::mat4(1.0f), scale);

		aiQuaternion rotationQ;
		calcInterpolatedRotation(rotationQ, animationTime, nodeAnim);
		glm::quat rotation = castQuat(rotationQ);
		glm::mat4 rotationM = glm::mat4_cast(rotation);

		aiVector3D translation;
		calcInterpolatedPosition(translation, animationTime, nodeAnim);
		glm::vec3 transVec = glm::vec3(translation.x, translation.y, translation.z);
		glm::mat4 translationM = glm::translate(glm::mat4(1.0f), transVec);

		nodeTransformation = translationM * rotationM * scalingM;
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransformation;

	if (boneMap.find(nodeName) != boneMap.end()) {
		unsigned int boneId = boneMap[nodeName];
		boneInfo[boneId].finalTransform = globalTransformation * boneInfo[boneId].offset;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		readNodeHeirarchy(animationTime, node->mChildren[i], globalTransformation);
	}
}

unsigned int Animator::findScaling(float animationTime, const aiNodeAnim* nodeAnim) {
	assert(nodeAnim->mNumScalingKeys > 0);
	for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys-1; i++) {
		if (animationTime < (float)nodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);
	return 0;
}

unsigned int Animator::findRotation(float animationTime, const aiNodeAnim* nodeAnim) {
	assert(nodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys-1; i++) {
		if (animationTime < (float)nodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);
	return 0;
}

unsigned int Animator::findPosition(float animationTime, const aiNodeAnim* nodeAnim) {
	for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
		if (animationTime < (float)nodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);
	return 0;
}

void Animator::calcInterpolatedScaling(aiVector3D& out, float animationTime, const aiNodeAnim* nodeAnim) {
	if (nodeAnim->mNumScalingKeys == 1) {
		out = nodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int scalingIndex = findScaling(animationTime, nodeAnim);
	unsigned int nextScalingIndex = (scalingIndex + 1);
	assert(nextScalingIndex < nodeAnim->mNumScalingKeys);

	float deltaTime = (float)(nodeAnim->mScalingKeys[nextScalingIndex].mTime - nodeAnim->mScalingKeys[scalingIndex].mTime);
	float factor = (animationTime - (float)nodeAnim->mScalingKeys[scalingIndex].mTime) / deltaTime;
	assert(factor >= 0.0f && factor >= 1.0f);

	const aiVector3D& start = nodeAnim->mScalingKeys[scalingIndex].mValue;
	const aiVector3D& end = nodeAnim->mScalingKeys[nextScalingIndex].mValue;
	aiVector3D delta = end - start;
	out = start + factor * delta;
}

void Animator::calcInterpolatedRotation(aiQuaternion& out, float animationTime, const aiNodeAnim* nodeAnim) {
	if (nodeAnim->mNumRotationKeys == 1) {
		out = nodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int rotationIndex = findRotation(animationTime, nodeAnim);
	unsigned int nextRotationIndex = (rotationIndex + 1);
	assert(nextRotationIndex < nodeAnim->mNumRotationKeys);
	
	float deltaTime = (float)(nodeAnim->mRotationKeys[nextRotationIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime);
	float factor = (animationTime - (float)nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);

	const aiQuaternion& start = nodeAnim->mRotationKeys[rotationIndex].mValue;
	const aiQuaternion& end = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
	aiQuaternion::Interpolate(out, start, end, factor);
	out = out.Normalize();
}

void Animator::calcInterpolatedPosition(aiVector3D& out, float animationTime, const aiNodeAnim* nodeAnim) {
	if (nodeAnim->mNumPositionKeys == 1) {
		out = nodeAnim->mPositionKeys[0].mValue;
		return;
	}
	
	unsigned int positionIndex = findPosition(animationTime, nodeAnim);
	unsigned int nextPositionIndex = (positionIndex + 1);
	assert(nextPositionIndex < nodeAnim->mNumPositionKeys);

	float deltaTime = (float)(nodeAnim->mPositionKeys[nextPositionIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime);
	float factor = (animationTime - (float)nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);

	const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
	const aiVector3D& end = nodeAnim->mPositionKeys[nextPositionIndex].mValue;
	aiVector3D delta = end - start;
	out = start + factor * delta;
}

const aiNodeAnim* Animator::findNodeAnim(const aiAnimation* animation, const std::string nodeName) {
	for (unsigned int i = 0; i < animation->mNumChannels; i++) {
		const aiNodeAnim* nodeAnim = animation->mChannels[i];

		if (std::string(nodeAnim->mNodeName.data) == nodeName) {
			return nodeAnim;
		}
	}

	return NULL;
}

glm::mat4 castMat4(const aiMatrix4x4& mat) {
	return glm::transpose(glm::make_mat4(&mat.a1));
}

glm::quat castQuat(const aiQuaternion& quat) {
	return glm::quat(quat.w, quat.x, quat.y, quat.z);
}