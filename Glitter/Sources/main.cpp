#define NOMINMAX
#include "Vendors.h"
#include "Mesh.h"
#include "Camera.h"
#include "imgui_stdlib.h"

#include <ImGuiFileDialog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

////////////// CGAL TEST ///////////////
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Barycentric_coordinates_3/Wachspress_coordinates_3.h>
#include <CGAL/Barycentric_coordinates_3/Mean_value_coordinates_3.h>

using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;

// default triangulation for Surface_mesher
using Tr = CGAL::Surface_mesh_default_triangulation_3;

using C2t3 = CGAL::Complex_2_in_triangulation_3<Tr>;
using Sphere_3 = Kernel::Sphere_3;
using Point_3 = Kernel::Point_3;
using FT = Kernel::FT;

typedef FT(*Function)(Point_3);
using  Surface_3 = CGAL::Implicit_surface_3<Kernel, Function>;
using Surface_mesh = CGAL::Surface_mesh<Point_3>;
namespace PMP = CGAL::Polygon_mesh_processing;

FT sphere_function(Point_3 p) {
    const FT x2 = p.x() * p.x(), y2 = p.y() * p.y(), z2 = p.z() * p.z();
    return x2 + y2 + z2 - 1;
}
////////////// CGAL TEST ///////////////


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
    ImGuiIO& io = ImGui::GetIO();
    // (2) ONLY forward mouse data to your underlying app/game.
    if (!io.WantCaptureMouse)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
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
}

int main() {



    ////////////// CGAL TEST ///////////////
    Tr tr;
    C2t3 c2t3(tr);


    // remplace ca par ton mesh, idk how cherche
    Surface_3 surface(sphere_function, Sphere_3(CGAL::ORIGIN, 2.));
    CGAL::Surface_mesh_default_criteria_3<Tr> criteria(30., 0.1, 0.1);
    CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());

    Surface_mesh sm;
    Surface_mesh deformed;
    CGAL::facets_in_complex_2_to_triangle_mesh(c2t3, sm);
    deformed = sm;

    Surface_mesh quad_cage;

    // afficher les points de controles, pouvoir les changer avec imgui
    const Point_3 p0(2, -2, -2), p0_new(5, -5, -5);
    const Point_3 p1(2, 2, -2), p1_new(3, 3, -3);
    const Point_3 p2(-2, 2, -2), p2_new(-2, 2, -2);
    const Point_3 p3(-2, -2, -2), p3_new(-3, -3, -3);

    const Point_3 p4(-2, -2, 2), p4_new(-3, -3, 3);
    const Point_3 p5(2, -2, 2), p5_new(4, -4, 4);
    const Point_3 p6(2, 2, 2), p6_new(2, 2, 3);
    const Point_3 p7(-2, 2, 2), p7_new(-3, 3, 3);

    CGAL::make_hexahedron(p0, p1, p2, p3, p4, p5, p6, p7, quad_cage);
    PMP::triangulate_faces(faces(quad_cage), quad_cage);

    // tu peux faire ca : mean value
    CGAL::Barycentric_coordinates::Mean_value_coordinates_3<Surface_mesh, Kernel> mv(quad_cage);
    // ou ca wachspress
    CGAL::Barycentric_coordinates::Wachspress_coordinates_3<Surface_mesh, Kernel> mv2(quad_cage);

    auto vertex_to_point_map = get_property_map(CGAL::vertex_point, deformed);

    std::vector<FT> coords;
    std::vector<Point_3> target_cube{ p0_new, p1_new, p2_new, p3_new,
                                     p4_new, p5_new, p6_new, p7_new };

    for (auto& v : vertices(deformed)) {

        const Point_3 vertex_val = get(vertex_to_point_map, v);
        coords.clear();
        mv(vertex_val, std::back_inserter(coords));

        FT x = FT(0), y = FT(0), z = FT(0);
        for (std::size_t i = 0; i < 8; i++) {

            x += target_cube[i].x() * coords[i];
            y += target_cube[i].y() * coords[i];
            z += target_cube[i].z() * coords[i];
        }

        put(vertex_to_point_map, v, Point_3(x, y, z));
    }

    std::cout << "cgal works" << std::endl;
    ////////////// CGAL TEST ///////////////

    const std::string m_title = "Coord Bary Gen";
    std::string file{"data/models/cube.obj"};

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

    Mesh test_cube(file);
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
        ImGui::Text("Currently Opened Model:");
        ImGui::Text(file.c_str());

        // open Dialog Simple
        if (ImGui::Button("Open Model"))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".fbx", ".");

        // display
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                file = filePathName;
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }
        Mesh test_cube(file);

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
