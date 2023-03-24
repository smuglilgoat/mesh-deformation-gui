#define NOMINMAX
#include "Vendors.h"
#include "Mesh.h"
#include "Camera.h"
#include "imgui_stdlib.h"
#include "csv.hpp"

#include <nfd.h>
#include <ImGuiFileDialog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/io.hpp>

////////////// CGAL ///////////////
#include <CGAL/Barycentric_coordinates_3/Mean_value_coordinates_3.h>
#include <CGAL/Barycentric_coordinates_3/Wachspress_coordinates_3.h>
#include <CGAL/Barycentric_coordinates_3/Discrete_harmonic_coordinates_3.h>
#include <CGAL/Barycentric_coordinates_3/tetrahedron_coordinates.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/IO/OBJ_reader.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>

using namespace csv;
using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Tr = CGAL::Surface_mesh_default_triangulation_3;
using C2t3 = CGAL::Complex_2_in_triangulation_3<Tr>;
using Point_3 = Kernel::Point_3;
using FT = Kernel::FT;
typedef FT(*Function)(Point_3);
using Surface_3 = CGAL::Implicit_surface_3<Kernel, Function>;
using Surface_mesh = CGAL::Surface_mesh<Point_3>;
namespace PMP = CGAL::Polygon_mesh_processing;

////////////// CGAL ///////////////

glm::vec2 last_mouse_pos = glm::vec2(0, 0);
bool dragging = false;
Camera camera;
int m_width = 1500;
int m_height = 1000;
bool wf = true;

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
    if (!io.WantCaptureMouse) {
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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == 90 && action == GLFW_PRESS)
        wf = true;
    if (key == 59 && action == GLFW_PRESS)
        wf = false;
}

int main() {
    NFD_Init();

    nfdchar_t* outPath;
    nfdfilteritem_t filterItem[1] = { { "Wavefront", "obj" } };
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
    if (result == NFD_OKAY)
    {
        puts("Opening File!");
        puts(outPath);

        const std::string m_title = "Coord Bary Gen";
        std::string file{ outPath };
        const char* methods[] = { "Mean_value", "Wachspress", "Discrete_harmonic"};
        static int methods_current = 0;

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

        printf("***** GPU INFO *****\n");
        printf("Graphic card vendor: %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("GL version: %s\n", glGetString(GL_VERSION));
        printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

        glfwSetCursorPosCallback(m_window, cursor_position_callback);
        glfwSetMouseButtonCallback(m_window, mouse_button_callback);
        glfwSetScrollCallback(m_window, scroll_callback);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 150");

        Shader lit_shader("data/shaders/base.vert", "data/shaders/base.frag");
        Shader unlit_shader("data/shaders/unlit.vert", "data/shaders/unlit.frag");

        camera.m_aspect_ratio = (float)m_width / (float)m_height;

        Mesh model_to_deform(file);

        Point_3 p0(2, -2, -2), p0_new(2, -2, -2);
        Point_3 p1(2, 2, -2), p1_new(2, 2, -2);
        Point_3 p2(-2, 2, -2), p2_new(-2, 2, -2);
        Point_3 p3(-2, -2, -2), p3_new(-2, -2, -2);
        Point_3 p4(-2, -2, 2), p4_new(-2, -2, 2);
        Point_3 p5(2, -2, 2), p5_new(2, -2, 2);
        Point_3 p6(2, 2, 2), p6_new(2, 2, 2);
        Point_3 p7(-2, 2, 2), p7_new(-2, 2, 2);

        glm::vec3 p0_glm = glm::vec3(2, -2, -2);
        glm::vec3 p1_glm = glm::vec3(2, 2, -2);
        glm::vec3 p2_glm = glm::vec3(-2, 2, -2);
        glm::vec3 p3_glm = glm::vec3(-2, -2, -2);
        glm::vec3 p4_glm = glm::vec3(-2, -2, 2);
        glm::vec3 p5_glm = glm::vec3(2, -2, 2);
        glm::vec3 p6_glm = glm::vec3(2, 2, 2);
        glm::vec3 p7_glm = glm::vec3(-2, 2, 2);

        std::vector<glm::vec3> glm_vertices = {
            p0_glm,
            p1_glm,
            p2_glm,
            p3_glm,
            p4_glm,
            p5_glm,
            p6_glm,
            p7_glm
        };

        std::vector<Vertex> cage_vertices = {
            { p0_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)},
            { p1_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)},
            { p2_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)},
            { p3_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)},
            { p4_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)},
            { p5_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)},
            { p6_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)},
            { p7_glm, glm::vec3(0, 0, 0), glm::vec2(0, 0)}
        };

        std::vector<unsigned int> cage_indices = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            2, 6, 7, 2, 7, 3,
            0, 4, 5, 0, 5, 1,
            1, 5, 6, 1, 6, 2,
            3, 4, 7
        };
        Mesh cage(cage_vertices, cage_indices);
        cage.m_wireframe = true;

        Surface_mesh quad_cage;
        CGAL::make_hexahedron(p0, p1, p2, p3, p4, p5, p6, p7, quad_cage);
        PMP::triangulate_faces(faces(quad_cage), quad_cage);
        CGAL::Barycentric_coordinates::Mean_value_coordinates_3<Surface_mesh, Kernel> deform_func(quad_cage);

        auto vertex_to_point_map = get_property_map(CGAL::vertex_point, model_to_deform.m_surface_mesh);
        std::vector<FT> coords;
        std::vector<Point_3> target_cube{ p0_new, p1_new, p2_new, p3_new,
                                          p4_new, p5_new, p6_new, p7_new };

        for (auto& v : vertices(model_to_deform.m_surface_mesh)) {

            const Point_3 vertex_val = get(vertex_to_point_map, v);
            coords.clear();
            deform_func(vertex_val, std::back_inserter(coords));

            FT x = FT(0), y = FT(0), z = FT(0);
            for (std::size_t i = 0; i < 8; i++) {

                x += target_cube[i].x() * coords[i];
                y += target_cube[i].y() * coords[i];
                z += target_cube[i].z() * coords[i];
            }

            put(vertex_to_point_map, v, Point_3(x, y, z));
        }

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
            ImGui::Spacing();
            ImGui::Text("Currently Used Method:");
            bool cage_change = false;
            if (ImGui::ListBox("(single select)", &methods_current, methods, IM_ARRAYSIZE(methods), 3)) {
                cage_change = true;
            }
            ImGui::Spacing();
            // open Dialog Simple
            if (ImGui::Button("Import")) {
                NFD_Init();

                nfdchar_t* outPath;
                nfdfilteritem_t filterItem[1] = { { "CSV file", "csv" } };
                nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
                if (result == NFD_OKAY)
                {
                    puts("Importing Control Points From:");
                    puts(outPath);

                    CSVReader reader(outPath);
                    CSVRow row;
                    for (int i = 0; i < 8; i++) {
                        reader.read_row(row);
                        glm_vertices.at(i).x = row[0].get<float>();
                        glm_vertices.at(i).y = row[1].get<float>();
                        glm_vertices.at(i).z = row[2].get<float>();
                        std::cout << glm_vertices.at(i) << i << "\n";
                    }
                    p0_glm = glm::vec3(glm_vertices.at(0).x, glm_vertices.at(0).y, glm_vertices.at(0).z);
                    p1_glm = glm::vec3(glm_vertices.at(1).x, glm_vertices.at(1).y, glm_vertices.at(1).z);
                    p2_glm = glm::vec3(glm_vertices.at(2).x, glm_vertices.at(2).y, glm_vertices.at(2).z);
                    p3_glm = glm::vec3(glm_vertices.at(3).x, glm_vertices.at(3).y, glm_vertices.at(3).z);
                    p4_glm = glm::vec3(glm_vertices.at(4).x, glm_vertices.at(4).y, glm_vertices.at(4).z);
                    p5_glm = glm::vec3(glm_vertices.at(5).x, glm_vertices.at(5).y, glm_vertices.at(5).z);
                    p6_glm = glm::vec3(glm_vertices.at(6).x, glm_vertices.at(6).y, glm_vertices.at(6).z);
                    p7_glm = glm::vec3(glm_vertices.at(7).x, glm_vertices.at(7).y, glm_vertices.at(7).z);
                    cage_change = true;

                    NFD_FreePath(outPath);
                }
                else if (result == NFD_CANCEL)
                {
                    puts("User pressed cancel.");
                }
                else
                {
                    printf("Error: %s\n", NFD_GetError());
                }

                NFD_Quit();
            }

            // display
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
            {
                // action if OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                    CSVReader reader(filePathName);
                }

                // close
                ImGuiFileDialog::Instance()->Close();
            }
            if (ImGui::Button("Export"))
            {
                NFD_Init();

                nfdchar_t* outPath;
                nfdfilteritem_t filterItem[1] = { { "CSV file", "csv" } };
                nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, NULL, NULL);
                if (result == NFD_OKAY)
                {
                    puts("Exporting Control Points To:");
                    puts(outPath);

                    std::ofstream outfile(outPath);

                    CSVWriter<std::ofstream> writer(outfile);
                    writer << std::vector<std::string>({ "X", "Y", "Z" });

                    writer << std::vector<float>({ p0_glm.x, p0_glm.y, p0_glm.z });
                    writer << std::vector<float>({ p1_glm.x, p1_glm.y, p1_glm.z });
                    writer << std::vector<float>({ p2_glm.x, p2_glm.y, p2_glm.z });
                    writer << std::vector<float>({ p3_glm.x, p3_glm.y, p3_glm.z });
                    writer << std::vector<float>({ p4_glm.x, p4_glm.y, p4_glm.z });
                    writer << std::vector<float>({ p5_glm.x, p5_glm.y, p5_glm.z });
                    writer << std::vector<float>({ p6_glm.x, p6_glm.y, p6_glm.z });
                    writer << std::vector<float>({ p7_glm.x, p7_glm.y, p7_glm.z });

                    NFD_FreePath(outPath);
                }
                else if (result == NFD_CANCEL)
                {
                    puts("User pressed cancel.");
                }
                else
                {
                    printf("Error: %s\n", NFD_GetError());
                }

                NFD_Quit();
            }
            cage_change |= ImGui::SliderFloat3("p0", (float*)&p0_glm, -5.0f, 5.0f);
            cage_change |= ImGui::SliderFloat3("p1", (float*)&p1_glm, -5.0f, 5.0f);
            cage_change |= ImGui::SliderFloat3("p2", (float*)&p2_glm, -5.0f, 5.0f);
            cage_change |= ImGui::SliderFloat3("p3", (float*)&p3_glm, -5.0f, 5.0f);
            cage_change |= ImGui::SliderFloat3("p4", (float*)&p4_glm, -5.0f, 5.0f);
            cage_change |= ImGui::SliderFloat3("p5", (float*)&p5_glm, -5.0f, 5.0f);
            cage_change |= ImGui::SliderFloat3("p6", (float*)&p6_glm, -5.0f, 5.0f);
            cage_change |= ImGui::SliderFloat3("p7", (float*)&p7_glm, -5.0f, 5.0f);
            ImGui::Spacing();
            

            if (cage_change) {
                // update cage viualization
                cage.UpdateVertexBuffer({
                    { p0_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)},
                    { p1_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)},
                    { p2_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)},
                    { p3_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)},
                    { p4_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)},
                    { p5_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)},
                    { p6_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)},
                    { p7_glm,  glm::vec3(0, 0, 0),  glm::vec2(0, 0)}
                    });

                // update CGAL cage
                p0_new = Point_3(p0_glm.x, p0_glm.y, p0_glm.z);
                p1_new = Point_3(p1_glm.x, p1_glm.y, p1_glm.z);
                p2_new = Point_3(p2_glm.x, p2_glm.y, p2_glm.z);
                p3_new = Point_3(p3_glm.x, p3_glm.y, p3_glm.z);
                p4_new = Point_3(p4_glm.x, p4_glm.y, p4_glm.z);
                p5_new = Point_3(p5_glm.x, p5_glm.y, p5_glm.z);
                p6_new = Point_3(p6_glm.x, p6_glm.y, p6_glm.z);
                p7_new = Point_3(p7_glm.x, p7_glm.y, p7_glm.z);
                target_cube = { p0_new, p1_new, p2_new, p3_new,
                                p4_new, p5_new, p6_new, p7_new
                };

                

                model_to_deform.ResetSurfaceMesh();
                Surface_mesh quad_cage;
                CGAL::make_hexahedron(p0, p1, p2, p3, p4, p5, p6, p7, quad_cage);
                PMP::triangulate_faces(faces(quad_cage), quad_cage);
                if (methods_current == 0) {
                    CGAL::Barycentric_coordinates::Mean_value_coordinates_3<Surface_mesh, Kernel> deform_func(quad_cage);
                    auto vertex_to_point_map = get_property_map(CGAL::vertex_point, model_to_deform.m_surface_mesh);
                    std::vector<FT> coords;
                    std::vector<Point_3> target_cube{ p0_new, p1_new, p2_new, p3_new,
                                                      p4_new, p5_new, p6_new, p7_new };

                    for (auto& v : vertices(model_to_deform.m_surface_mesh)) {

                        const Point_3 vertex_val = get(vertex_to_point_map, v);
                        coords.clear();
                        deform_func(vertex_val, std::back_inserter(coords));

                        FT x = FT(0), y = FT(0), z = FT(0);
                        for (std::size_t i = 0; i < 8; i++) {

                            x += target_cube[i].x() * coords[i];
                            y += target_cube[i].y() * coords[i];
                            z += target_cube[i].z() * coords[i];
                        }

                        put(vertex_to_point_map, v, Point_3(x, y, z));
                    }
                }
                else if (methods_current == 2) {
                    CGAL::Barycentric_coordinates::Discrete_harmonic_coordinates_3<Surface_mesh, Kernel> deform_func(quad_cage);
                    auto vertex_to_point_map = get_property_map(CGAL::vertex_point, model_to_deform.m_surface_mesh);
                    std::vector<FT> coords;
                    std::vector<Point_3> target_cube{ p0_new, p1_new, p2_new, p3_new,
                                                      p4_new, p5_new, p6_new, p7_new };

                    for (auto& v : vertices(model_to_deform.m_surface_mesh)) {

                        const Point_3 vertex_val = get(vertex_to_point_map, v);
                        coords.clear();
                        deform_func(vertex_val, std::back_inserter(coords));

                        FT x = FT(0), y = FT(0), z = FT(0);
                        for (std::size_t i = 0; i < 8; i++) {

                            x += target_cube[i].x() * coords[i];
                            y += target_cube[i].y() * coords[i];
                            z += target_cube[i].z() * coords[i];
                        }

                        put(vertex_to_point_map, v, Point_3(x, y, z));
                    }
                }
                else if (methods_current == 1) {
                    CGAL::Barycentric_coordinates::Wachspress_coordinates_3<Surface_mesh, Kernel> deform_func(quad_cage);
                    auto vertex_to_point_map = get_property_map(CGAL::vertex_point, model_to_deform.m_surface_mesh);
                    std::vector<FT> coords;
                    std::vector<Point_3> target_cube{ p0_new, p1_new, p2_new, p3_new,
                                                      p4_new, p5_new, p6_new, p7_new };

                    for (auto& v : vertices(model_to_deform.m_surface_mesh)) {

                        const Point_3 vertex_val = get(vertex_to_point_map, v);
                        coords.clear();
                        deform_func(vertex_val, std::back_inserter(coords));

                        FT x = FT(0), y = FT(0), z = FT(0);
                        for (std::size_t i = 0; i < 8; i++) {

                            x += target_cube[i].x() * coords[i];
                            y += target_cube[i].y() * coords[i];
                            z += target_cube[i].z() * coords[i];
                        }

                        put(vertex_to_point_map, v, Point_3(x, y, z));
                    }
                }

                model_to_deform.UpdateMeshFromCGAL();
            }

            ImGui::End();

            ImGui::Render();
            
            glfwSetKeyCallback(m_window, key_callback);

            glClearColor(0.176f, 0.294f, 0.463f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, m_width, m_height);

            camera.Update();

            model_to_deform.m_wireframe = wf;
            model_to_deform.Render(&camera, &lit_shader);

            cage.Render(&camera, &unlit_shader);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }

        glfwTerminate();
        NFD_FreePath(outPath);
        return 0;
    }
    else if (result == NFD_CANCEL)
    {
        puts("User pressed cancel.");
    }
    else
    {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
    return 0;
}
