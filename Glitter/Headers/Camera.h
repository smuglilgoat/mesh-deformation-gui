#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

struct Camera {
	glm::vec3 m_position = glm::vec3(0, 0, 0);
	glm::quat m_rotation = glm::quat(1, 0, 0, 0);
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

	float m_orbit_distance = 4;
	glm::vec3 m_orbit_pivot = glm::vec3(0, 0, 0);
	glm::vec2 m_orbit_rotation = glm::vec2(0, 0);

	float m_fov = 70;
	float m_aspect_ratio = 1;
	float m_z_near = 0.1f;
	float m_z_far = 1000.0f;

	glm::mat4x4 m_view = glm::mat4x4();
	glm::mat4x4 m_projection = glm::mat4x4();

	void Update();
};