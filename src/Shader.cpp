#include "Shader.h"

void handleErrors(unsigned int el, char type);

Shader::Shader(const char *vPath, const char *fPath) {
	std::string vCode, fCode;
	std::ifstream vFile;
	std::ifstream fFile;

	vFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	fFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);

	try {
		std::stringstream vStream, fStream;

		vFile.open(vPath);
		fFile.open(fPath);

		vStream << vFile.rdbuf();
		fStream << fFile.rdbuf();

		fFile.close();
		vFile.close();
		
		vCode = vStream.str();
		fCode = fStream.str();
	}
	catch (std::ifstream::failure e){
		std::cout << "ShaderProgramError: Could not load shader files." << std::endl;
	}

	const char* vSrc = vCode.c_str();
	const char* fSrc = fCode.c_str();

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vSrc, NULL);
	glCompileShader(vertexShader);
	handleErrors(vertexShader, 'S');

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fSrc, NULL);
	glCompileShader(fragmentShader);
	handleErrors(fragmentShader, 'S');

	id = glCreateProgram();
	glAttachShader(id, vertexShader);
	glAttachShader(id, fragmentShader);
	glLinkProgram(id);
	handleErrors(id, 'P');
}

void Shader::use() {
	glUseProgram(id);
}

void handleErrors(unsigned int el, char type) {
	int success;
	char infoLog[512];
	switch (type) {
	case 'S':
		glGetShaderiv(el, GL_COMPILE_STATUS, &success);
		break;
	case 'P':
		glGetProgramiv(el, GL_LINK_STATUS, &success);
		break;
	}

	if (!success) {
		switch (type) {
		case 'S':
			glGetShaderInfoLog(el, 512, NULL, infoLog);
			break;
		case 'P':
			glGetProgramInfoLog(el, 512, NULL, infoLog);
			break;
		}

		std::cout << "ShaderProgramError: " << infoLog << std::endl;
	}
}

void Shader::setFloat(const std::string &name, float value) const {
	glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int) value);
}

void Shader::setVec3(const std::string &name, glm::vec3 value) const {
	glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
}

void Shader::setMat4(const std::string &name, glm::mat4 value) const {
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}