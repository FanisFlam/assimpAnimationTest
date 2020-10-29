#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

uniform mat4 projection, view, model;

out vec2 texCoord;

const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];

void main(){
	mat4 boneTransform = bones[boneIds[0]] * weights[0];
	boneTransform += bones[boneIds[1]] * weights[1];
	boneTransform += bones[boneIds[2]] * weights[2];
	boneTransform += bones[boneIds[3]] * weights[3];

	texCoord = aTexCoord;

	vec4 fPos = boneTransform * vec4(aPos, 1.0);
	gl_Position = projection * view * model * fPos;
}