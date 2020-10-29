#include "Model.h"

unsigned int textureFromFile(const char* path, const std::string& dir);
glm::vec3 getVec(aiVector3D el);
glm::mat4 castMat4(const aiMatrix4x4 &mat);
glm::quat castQuat(aiQuaternion &q);

Model::Model(const char* path) {
	loadModel(path);
}

void Model::draw(Shader &shader) {
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].draw(shader);
	}
}

void Model::loadModel(const std::string& path) {
	Assimp::Importer importer;

	scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	scene = importer.GetOrphanedScene();

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "Could not load model: " << importer.GetErrorString() << std::endl;
		return;
	}

	animator = new Animator(scene);

	dir = path.substr(0, path.find_last_of('/'));

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		baseVertex.push_back(totalVertices);
		totalVertices += scene->mMeshes[i]->mNumVertices;
	}

	bones.resize(totalVertices);

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		animator->loadBones(i, mesh, bones, baseVertex);
		meshes.push_back(processMesh(i, mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(unsigned int meshId, aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Vertex vertex;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		vertex.position = getVec(mesh->mVertices[i]);
		vertex.normal = getVec(mesh->mNormals[i]);
		vertex.tangent = getVec(mesh->mTangents[i]);
		vertex.bitangent = getVec(mesh->mBitangents[i]);

		if (mesh->mTextureCoords[0])
			vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		for(unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			indices.push_back(mesh->mFaces[i].mIndices[j]);

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = getMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> normalMaps = getMaterialTextures(mat, aiTextureType_HEIGHT, "texture_normals");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> aoMaps = getMaterialTextures(mat, aiTextureType_AMBIENT, "texture_ao");
		textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
	}

	return Mesh(vertices, indices, textures, bones);
}

std::vector<Texture> Model::getMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture> textures;
	Texture texture;
	aiString str;
	bool skip;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		mat->GetTexture(type, i, &str);
		skip = false;
		for (unsigned int j = 0; j < loaded_textures.size(); j++) {
			if (std::strcmp(loaded_textures[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(loaded_textures[j]);
				skip = true;
				break;
			}
		}

		if (!skip) {
			texture.id = textureFromFile(str.C_Str(), this->dir);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			loaded_textures.push_back(texture);
		}
	}

	return textures;
}

unsigned int textureFromFile(const char* path, const std::string& dir) {
	std::string filename = std::string(path);
	filename = dir + '/' + filename;
	unsigned int id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format;
		switch (nrChannels) {
			case 1:
				format = GL_RED;
				break;
			case 3:
				format = GL_RGB;
				break;
			case 4:
				format = GL_RGBA;
				break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Could not load model texture: " << path << std::endl;

	stbi_image_free(data);

	return id;
}

glm::vec3 getVec(aiVector3D el) {
	return glm::vec3(el.x, el.y, el.z); 
}

void Model::playAnimation(float time, Shader &shader) {
	std::vector<glm::mat4> transforms;

	transforms = animator->boneTransform(time, transforms);

	glUniformMatrix4fv(glGetUniformLocation(shader.id, "bones"), (GLsizei)transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
}