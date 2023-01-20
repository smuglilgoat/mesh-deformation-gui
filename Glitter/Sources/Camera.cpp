#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

void Camera::Update() {
	glm::mat4 m = glm::mat4(1.0);
	m *= glm::translate(glm::mat4(1.0), m_orbit_pivot);
	m *= glm::yawPitchRoll(m_orbit_rotation.x, m_orbit_rotation.y, 0.0f);
	m *= glm::translate(glm::mat4(1.0), glm::vec3(0, 0, m_orbit_distance));
	auto tmp = m * glm::vec4(0, 0, 0, 1);
	m_position = glm::vec3(tmp.x, tmp.y, tmp.z);

	m_view = glm::inverse(m);

	m_projection = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_z_near, m_z_far);
}
