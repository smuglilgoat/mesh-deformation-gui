#include "Shader.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <glad/glad.h>

Shader::Shader(std::string vertex_shader_path, std::string fragment_shader_path) {

    std::ifstream vertex_file_stream(vertex_shader_path);
    std::stringstream buffer_vertex;
    buffer_vertex << vertex_file_stream.rdbuf();
    std::string vertex_shader_source = buffer_vertex.str();

    std::ifstream fragment_file_stream(fragment_shader_path);
    std::stringstream buffer_fragment;
    buffer_fragment << fragment_file_stream.rdbuf();
    std::string fragment_shader_source = buffer_fragment.str();

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertex_c_str = vertex_shader_source.c_str();
    glShaderSource(vertex_shader, 1, &vertex_c_str, NULL);
    glCompileShader(vertex_shader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        std::cout << infoLog << std::endl;
        return;
    }

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragment_c_str = fragment_shader_source.c_str();
    glShaderSource(fragment_shader, 1, &fragment_c_str, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        std::cout << infoLog << std::endl;
        return;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex_shader);
    glAttachShader(m_program, fragment_shader);
    glLinkProgram(m_program);

    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        std::cout << infoLog << std::endl;
        return;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::SetMat4(std::string name, glm::mat4 mat) {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetVec3(std::string name, const glm::vec3& vec) {
    glUniform3fv(glGetUniformLocation(m_program, name.c_str()), 1, &vec[0]);
}