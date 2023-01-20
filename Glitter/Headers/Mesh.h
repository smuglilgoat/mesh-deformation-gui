#pragma once

#include "Shader.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

struct Vertex {
	glm::vec3 m_position;
	glm::vec3 m_normal;
	glm::vec2 m_texcoord;
};

struct Mesh {
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;

	glm::vec3 m_position = glm::vec3(0, 0, 0);
	glm::quat m_rotation = glm::quat(1.0, 0.0, 0.0, 0.0);
	glm::vec3 m_scale = glm::vec3(1, 1, 1);

	unsigned int m_vao, m_vbo, m_ebo;

	Mesh(std::string path);
	void Render(Camera* camera, Shader* shader);
};