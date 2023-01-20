#define NOMINMAX
#include "Vendors.h"
#include "Mesh.h"
#include "Camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

glm::vec2 last_mouse_pos = glm::vec2(0, 0);
bool dragging = false;
Camera camera;
int m_width = 1500;
int m_height = 1000;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.m_orbit_distance -= yoffset * 0.3f;
    if (camera.m_orbit_distance < 0.3f) camera.m_orbit_distance = 0.3f;
}

static void cursor_position_callback(GLFWwindow* window, double pos_x, double pos_y) {
    if (dragging) {
        glm::vec2 current_mouse_pos = glm::vec2(pos_x, pos_y);
        
        glm::vec2 mouse_delta = last_mouse_pos - current_mouse_pos;
        glm::vec2 delta_angle = glm::vec2(2 * glm::pi<float>() / m_width, glm::pi<float>() / m_height);

        camera.m_orbit_rotation += glm::vec2(mouse_delta.x * delta_angle.x, mouse_delta.y * delta_angle.y);
        camera.m_orbit_rotation.y = glm::clamp(camera.m_orbit_rotation.y, -glm::pi<float>() / 2, glm::pi<float>() / 2);
        last_mouse_pos = current_mouse_pos;

        camera.Update();
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT ) {
        if (action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            last_mouse_pos = glm::vec2(x, y);
            dragging = true;
        }
        else if (action == GLFW_RELEASE) {
            dragging = false;
        }
    }
}

int main() {

    const std::string m_title = "";

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

    if (m_window == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(m_window);
    gladLoadGL();

    glfwSetCursorPosCallback(m_window, cursor_position_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetScrollCallback(m_window, scroll_callback);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    Mesh test_cube("data/models/cube.fbx");
    Shader test_shader("data/shaders/base.vert", "data/shaders/base.frag");

    camera.m_aspect_ratio = (float)m_width / (float)m_height ;

    while (glfwWindowShouldClose(m_window) == false) {
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_window, true);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Menu");
        ImGui::Text("");
        ImGui::End();

        ImGui::Render();

        glClearColor(0.176f, 0.294f, 0.463f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, m_width, m_height);

        camera.Update();

        test_cube.Render(&camera, &test_shader);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    
    return 0;
}
