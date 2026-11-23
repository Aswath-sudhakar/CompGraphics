include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable (glfwExample
  glfwExample.cpp 
  GLSL.cpp GLSL.h
)

target_link_libraries (glfwExample PRIVATE GLEW::GLEW)
target_link_libraries (glfwExample PRIVATE glfw)

if(APPLE)
    target_link_libraries(glfwExample PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
else()
    target_link_libraries(glfwExample PRIVATE OpenGL::GL)
endif()

target_link_libraries (glfwExample PRIVATE PNG::PNG)
target_link_libraries (glfwExample PRIVATE ZLIB::ZLIB)

find_package(nlohmann_json CONFIG REQUIRED)
add_executable(rasterizer
  rasterizer.cpp
  ../src/SceneParser_JSON.cpp
)
target_include_directories(rasterizer PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(rasterizer PRIVATE
  OPENGL_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(rasterizer PRIVATE
  cs4212-util
  nlohmann_json::nlohmann_json
  GLEW::GLEW
  glfw
  OpenGL::GL
  glm::glm
)
