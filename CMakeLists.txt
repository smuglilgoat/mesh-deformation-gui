cmake_minimum_required(VERSION 3.0)
project(Glitter)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(Glitter/Vendor/glfw)

set(CGAL_DIR "C://local//cgal//")
set(GMP_INCLUDE_DIR "C://local//cgal//auxiliary//gmp//include")
set(GMP_LIBRARIES "C://local//cgal//auxiliary//gmp//lib//libgmp-10.lib")
set(MPFR_INCLUDE_DIR "C://local//cgal//auxiliary//gmp//include")
set(MPFR_LIBRARIES "C://local//cgal//auxiliary//gmp//lib//libmpfr-4.lib")

find_package(CGAL QUIET COMPONENTS Core)
if(CGAL_FOUND)
  include(${CGAL_USE_FILE})
  include(CGAL_CreateSingleSourceCGALProgram)
else()
  message(WARNING "This program requires the CGAL library, and will not be compiled.")
endif()


option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(Glitter/Vendor/assimp)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(Glitter/Headers/
                    Glitter/Vendor/assimp/include/
                    Glitter/Vendor/glad/include/
                    Glitter/Vendor/glfw/include/
                    Glitter/Vendor/glm/
                    Glitter/Vendor/stb/
                    Glitter/Vendor/imgui/
                    Glitter/Vendor/imgui/backends/
			        Glitter/Vendor/imguifiledialog
                    )

add_subdirectory(Glitter/Vendor/nativefiledialog-extended)

file(GLOB VENDORS_SOURCES Glitter/Vendor/glad/src/glad.c
						  Glitter/Vendor/imgui/*.cpp
						  Glitter/Vendor/imguifiledialog/*.cpp)
file(GLOB VENDORS_SOURCES Glitter/Vendor/glad/src/glad.c
						  Glitter/Vendor/imgui/*.cpp
						  Glitter/Vendor/imguifiledialog/*.cpp)
file(GLOB PROJECT_HEADERS Glitter/Vendor/imgui/backends/imgui_impl_opengl3.h)
file(GLOB PROJECT_HEADERS Glitter/Vendor/imgui/backends/imgui_impl_opengl3_loader.h)
file(GLOB PROJECT_HEADERS Glitter/Vendor/imgui/backends/imgui_impl_glfw.h)
file(GLOB PROJECT_HEADERS Glitter/Vendor/imguifiledialog/ImGuiFileDialog.h)
file(GLOB PROJECT_HEADERS Glitter/Headers/*.hpp)
file(GLOB PROJECT_HEADERS Glitter/Headers/*.h)
file(GLOB PROJECT_SOURCES Glitter/Sources/*.cpp)
file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Vendors" FILES ${VENDORS_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_CONFIGS}
                               ${VENDORS_SOURCES})
target_link_libraries(${PROJECT_NAME} assimp glfw nfd
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})