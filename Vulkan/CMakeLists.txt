include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable (basicCode
  basicCode.cpp
)
target_link_libraries (basicCode GLEW::GLEW)
target_link_libraries (basicCode glfw)
target_link_libraries (basicCode Vulkan::Vulkan)
target_link_libraries (basicCode PNG::PNG)
target_link_libraries (basicCode ZLIB::ZLIB)

add_executable (glfwVulkan
  glfwVulkan.cpp
)
target_link_libraries (glfwVulkan GLEW::GLEW)
target_link_libraries (glfwVulkan glfw)
target_link_libraries (glfwVulkan Vulkan::Vulkan)
target_link_libraries (glfwVulkan PNG::PNG)
target_link_libraries (glfwVulkan ZLIB::ZLIB)
