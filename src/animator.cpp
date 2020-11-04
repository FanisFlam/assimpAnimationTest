#include "animator.h"

glm::mat4 castMat4(const aiMatrix4x4 &mat);
glm::mat4 castMat4(const aiMatrix3x3 &mat);
glm::quat castQuat(const aiQuaternion &quat);

Animator::Animator(const aiScene *scene) {
    this->scene = scene;
    this->globalITransform = inverse(castMat4(scene->mRootNode->mTransformation));

}

void Animator::loadBones(Mesh &mesh, unsigned int i)
{
        aiMesh* pMesh = scene->mMeshes[i];
        for (unsigned int i = 0 ; i < pMesh->mNumBones ; i++) {
          unsigned int BoneIndex = 0;
          std::string BoneName(pMesh->mBones[i]->mName.data);

          if (boneMap.find(BoneName) == boneMap.end()) {
              // Allocate an index for a new bone
              BoneIndex = numBones;
              numBones++;
              BoneInfo bi;
              boneInfo.push_back(bi);
              boneInfo[BoneIndex].offset = castMat4(pMesh->mBones[i]->mOffsetMatrix);
              boneMap[BoneName] = BoneIndex;
          }
          else {
              BoneIndex = boneMap[BoneName];
          }
          for (unsigned int j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
              unsigned int VertexID = pMesh->mBones[i]->mWeights[j].mVertexId;
              float Weight  = pMesh->mBones[i]->mWeights[j].mWeight;
              mesh.bones[VertexID].addBoneData(BoneIndex, Weight);
          }
        }
}

void Animator::readNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4 &ParentTransform)
{
    std::string NodeName(pNode->mName.data);

    const aiAnimation* pAnimation = scene->mAnimations[0];

    glm::mat4 NodeTransformation =  castMat4(pNode->mTransformation);

    const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, NodeName);

    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        aiVector3D Scaling;
        calcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
        glm::mat4 ScalingM(1.0f);
        scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));


        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        calcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
        glm::mat4 RotationM = castMat4(RotationQ.GetMatrix());

        // Interpolate translation and generate translation transformation matrix
        aiVector3D Translation;
        calcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
        glm::mat4 TranslationM(1.0f);
        TranslationM = translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));

        NodeTransformation = TranslationM * RotationM * ScalingM;
    }
    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (boneMap.find(NodeName) != boneMap.end()) {
        unsigned int BoneIndex = boneMap[NodeName];
        glm::mat4 BoneOffset, FinalTransformation;
        BoneOffset = boneInfo[BoneIndex].offset;
        FinalTransformation = globalITransform * GlobalTransformation * BoneOffset;
        boneInfo[BoneIndex].finalTransform = FinalTransformation;
    }

    for (unsigned int i = 0 ; i < pNode->mNumChildren ; i++) {
        readNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}

void Animator::boneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
{
    glm::mat4 Identity(1.0f);

    float TicksPerSecond = (float)(scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, (float)scene->mAnimations[0]->mDuration);

    readNodeHeirarchy(AnimationTime, scene->mRootNode, Identity);

    Transforms.resize(numBones);

    for (unsigned int i = 0 ; i < numBones ; i++) {
        Transforms[i] = boneInfo[i].finalTransform;
    }
}

const aiNodeAnim* Animator::findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
    for (unsigned int i = 0 ; i < pAnimation->mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}

void Animator::calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    unsigned int PositionIndex = findPosition(AnimationTime, pNodeAnim);
    unsigned int NextPositionIndex = (PositionIndex + 1);
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}


void Animator::calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    unsigned int RotationIndex = findRotation(AnimationTime, pNodeAnim);
    unsigned int NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ   = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}


void Animator::calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    unsigned int ScalingIndex = findScaling(AnimationTime, pNodeAnim);
    unsigned int NextScalingIndex = (ScalingIndex + 1);
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

unsigned int Animator::findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (unsigned int i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}


unsigned int Animator::findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (unsigned int i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}


unsigned int Animator::findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (unsigned int i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}

glm::mat4 castMat4(const aiMatrix4x4& mat) {
    return glm::transpose(glm::make_mat4(&mat.a1));
}

glm::quat castQuat(const aiQuaternion& quat) {
    return glm::quat(quat.w, quat.x, quat.y, quat.z);
}

glm::mat4 castMat4(const aiMatrix3x3 &mat){
    return glm::transpose(glm::make_mat3(&mat.a1));
}
