#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>

class Shader {
public:
	Shader(const char *vPath, const char *fPath);
	void use();

	unsigned int id;
	void setFloat(const std::string &name, float value) const;
	void setInt(const std::string &name, int value) const;
	void setBool(const std::string &name, bool value) const;
	void setVec3(const std::string &name, glm::vec3 value) const;
	void setMat4(const std::string &name, glm::mat4 value) const;
};

#endif