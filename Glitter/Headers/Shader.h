#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <string>

struct Shader {
	unsigned int m_program = 0;

	Shader(std::string vertex_shader_path, std::string fragment_shader_path);
	void SetMat4(std::string name, glm::mat4 mat);
	void SetVec3(std::string name, const glm::vec3& vec);
};