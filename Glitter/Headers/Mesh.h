#pragma once

#include "Shader.h"
#include "Camera.h"

#include <CGAL/Barycentric_coordinates_3/Mean_value_coordinates_3.h>
#include <CGAL/Barycentric_coordinates_3/Wachspress_coordinates_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/IO/OBJ_reader.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>


using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Tr = CGAL::Surface_mesh_default_triangulation_3;
using C2t3 = CGAL::Complex_2_in_triangulation_3<Tr>;
using Point_3 = Kernel::Point_3;
using FT = Kernel::FT;
typedef FT(*Function)(Point_3);
using Surface_3 = CGAL::Implicit_surface_3<Kernel, Function>;
using Surface_mesh = CGAL::Surface_mesh<Point_3>;
namespace PMP = CGAL::Polygon_mesh_processing;

struct Vertex {
	glm::vec3 m_position;
	glm::vec3 m_normal;
	glm::vec2 m_texcoord;
};

struct Mesh {
	bool m_wireframe = false;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Point_3> deform_vertices;
	std::vector<std::vector<std::size_t>> deform_indices;
	Surface_mesh m_surface_mesh;

	glm::vec3 m_position = glm::vec3(0, 0, 0);
	glm::quat m_rotation = glm::quat(1.0, 0.0, 0.0, 0.0);
	glm::vec3 m_scale = glm::vec3(1, 1, 1);

	unsigned int m_vao, m_vbo, m_ebo;

	void ResetSurfaceMesh();

	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	Mesh(std::string path);
	void ReloadMesh(std::string path);
	void Render(Camera* camera, Shader* shader);
	void UpdateVertexBuffer(std::vector<Vertex> vertices);
	void UpdateMeshToCGAL();
};