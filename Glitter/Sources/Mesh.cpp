#include "Mesh.h"

#include <filesystem>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include <cstddef>
#include <glm/gtx/string_cast.hpp>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {

    m_vertices = vertices;
    m_indices = indices;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
        &m_indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));


    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_texcoord));



    glBindVertexArray(0);
}


Mesh::Mesh(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << importer.GetErrorString() << std::endl;
        return;
    }
    auto mesh = scene->mMeshes[0];
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.m_position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        deform_vertices.push_back(Point_3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        if (mesh->HasNormals()) {
            v.m_normal = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        }
        if (mesh->mTextureCoords[0]) {
            v.m_texcoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        m_vertices.push_back(v);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        std::vector < std::size_t> indices_;
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            m_indices.push_back(face.mIndices[j]);
            std::size_t index_ = face.mIndices[j];
            indices_.push_back(index_);
        }
        deform_indices.push_back(indices_);
    }

    for (const Point_3& p : deform_vertices) {
        m_surface_mesh.add_vertex(p);
    }

    for (const auto& face : deform_indices) {
        std::vector<Surface_mesh::Vertex_index> vertex_indices;
        for (std::size_t i : face) {
            vertex_indices.push_back(Surface_mesh::Vertex_index(i));
        }
        m_surface_mesh.add_face(vertex_indices);
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
        &m_indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    if (mesh->HasNormals()) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
    }
    if (mesh->mTextureCoords[0]) {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_texcoord));

    }

    glBindVertexArray(0);
}

void Mesh::ReloadMesh(std::string path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << importer.GetErrorString() << std::endl;
        return;
    }
    auto mesh = scene->mMeshes[0];
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.m_position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        deform_vertices.push_back(Point_3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        if (mesh->HasNormals()) {
            v.m_normal = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        }
        if (mesh->mTextureCoords[0]) {
            v.m_texcoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        m_vertices.push_back(v);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        std::vector < std::size_t> indices_;
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            m_indices.push_back(face.mIndices[j]);
            std::size_t index_ = face.mIndices[j];
            indices_.push_back(index_);
        }
        deform_indices.push_back(indices_);
    }

    for (const Point_3& p : deform_vertices) {
        m_surface_mesh.add_vertex(p);
    }

    for (const auto& face : deform_indices) {
        std::vector<Surface_mesh::Vertex_index> vertex_indices;
        for (std::size_t i : face) {
            vertex_indices.push_back(Surface_mesh::Vertex_index(i));
        }
        m_surface_mesh.add_face(vertex_indices);
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
        &m_indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    if (mesh->HasNormals()) {
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
    }
    if (mesh->mTextureCoords[0]) {
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_texcoord));

    }

    glBindVertexArray(0);
}

void Mesh::ResetSurfaceMesh() {
    m_surface_mesh.clear();
    for (const Point_3& p : deform_vertices) {
        m_surface_mesh.add_vertex(p);
    }

    for (const auto& face : deform_indices) {
        std::vector<Surface_mesh::Vertex_index> vertex_indices;
        for (std::size_t i : face) {
            vertex_indices.push_back(Surface_mesh::Vertex_index(i));
        }
        m_surface_mesh.add_face(vertex_indices);
    }
}

void Mesh::Render(Camera* camera, Shader* shader) {
    if (m_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    }
    else {
        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glEnable(GL_DEPTH_TEST);


    glm::mat4 translation = glm::translate(glm::mat4(1.0), m_position);
    glm::mat4 rotation = glm::mat4_cast(m_rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0), m_scale);
    glm::mat4 model = translation * rotation * scale;
    glUseProgram(shader->m_program);

    shader->SetMat4("u_model", model);
    shader->SetMat4("u_view", camera->m_view);
    shader->SetMat4("u_projection", camera->m_projection);
    shader->SetVec3("u_view_pos", camera->m_position);
    shader->SetVec3("u_light_pos", glm::vec3(5, 5, 1));

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::UpdateVertexBuffer(std::vector<Vertex> vertices) {
    m_vertices = vertices;
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);
}


void Mesh::UpdateMeshToCGAL() {
    std::vector < Vertex> new_vertices;
    int counter = 0;
    for (const auto& face : m_surface_mesh.faces()) {
        for (const auto& v : m_surface_mesh.vertices_around_face(m_surface_mesh.halfedge(face))) {
            auto point = m_surface_mesh.point(v);
            if (counter >= m_vertices.size()) {
                break;
            }
            new_vertices.push_back({
                {point.x(), point.y(), point.z()},
                m_vertices[counter].m_normal,
                glm::vec2(0, 0)
                });
            counter++;
        }
    }
    m_vertices = new_vertices;
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);
}