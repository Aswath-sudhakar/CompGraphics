#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <png++/png.hpp>

#include "ISceneLoader.h"
#include "SceneParser_JSON.h"
#include "model_obj.h"

namespace {
constexpr int kMaxLights = 8;
constexpr int kSphereSegments = 36;
constexpr int kSphereRings = 24;
constexpr float kPi = 3.14159265358979323846f;
constexpr float kWindowAspect = 1200.0f / 800.0f;

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
  float blinnPhong;
};

struct Material {
  glm::vec3 diffuse{ 0.8f };
  glm::vec3 specular{ 0.0f };
  float shininess = 32.0f;
  float blinnPhong = 0.0f;
};

struct Camera {
  glm::vec3 position{ 0.0f };
  float yaw = -90.0f;
  float pitch = 0.0f;
  float verticalFov = 60.0f;

  glm::vec3 forward() const
  {
    const float yawRadians = glm::radians(yaw);
    const float pitchRadians = glm::radians(pitch);
    return glm::normalize(glm::vec3(
      std::cos(yawRadians) * std::cos(pitchRadians),
      std::sin(pitchRadians),
      std::sin(yawRadians) * std::cos(pitchRadians)));
  }

  glm::mat4 view() const
  {
    return glm::lookAt(position, position + forward(), glm::vec3(0, 1, 0));
  }
};

struct RasterScene {
  std::vector<Vertex> vertices;
  std::vector<glm::vec3> lightPositions;
  std::vector<glm::vec3> lightIntensities;
  glm::vec3 background{ 0.04f, 0.05f, 0.08f };
  Camera camera;
  bool hasCamera = false;
};

glm::vec3 toGlm(const ISceneLoader::vec &value)
{
  return { value.x, value.y, value.z };
}

class RasterSceneLoader final : public ISceneLoader {
public:
  explicit RasterSceneLoader(RasterScene &scene) : scene_(scene) {}

  void reserveCameras(size_t count) override
  {
    if (count > 1) throw std::runtime_error("The rasterizer supports one camera.");
  }
  void reserveLights(size_t count) override { scene_.lightPositions.reserve(count); }
  void reserveShapes(size_t) override {}
  void reserveShaders(size_t count) override { materials_.reserve(count); }
  void reserveTextures(size_t) override {}

  void setSceneParams(const SceneParams &params) override
  {
    scene_.background = toGlm(params.bgColor);
    if (!params.envMapFile.empty()) {
      std::cerr << "Warning: environment maps are not supported; ignoring '"
                << params.envMapFile << "'.\n";
    }
  }

  void addCamera(const std::string &, const std::string &type,
                 const vec &position, const vec &viewDirection,
                 float focalLength, float imagePlaneWidth) override
  {
    if (type != "perspective" && type != "perspective_camera") {
      throw std::runtime_error("Unsupported camera type: " + type);
    }
    if (focalLength <= 0.0f || imagePlaneWidth <= 0.0f ||
        (viewDirection.x == 0.0f && viewDirection.y == 0.0f && viewDirection.z == 0.0f)) {
      throw std::runtime_error("Camera dimensions and view direction must be nonzero.");
    }
    scene_.camera.position = toGlm(position);
    const glm::vec3 forward = glm::normalize(toGlm(viewDirection));
    scene_.camera.yaw = glm::degrees(std::atan2(forward.z, forward.x));
    scene_.camera.pitch = glm::degrees(std::asin(std::clamp(forward.y, -1.0f, 1.0f)));
    const float planeHeight = imagePlaneWidth / kWindowAspect;
    scene_.camera.verticalFov =
      glm::degrees(2.0f * std::atan(planeHeight / (2.0f * focalLength)));
    scene_.hasCamera = true;
  }

  void addPointLight(const vec &position, const vec &intensity) override
  {
    if (scene_.lightPositions.size() >= kMaxLights) {
      throw std::runtime_error("The rasterizer supports at most 8 point lights.");
    }
    scene_.lightPositions.push_back(toGlm(position));
    scene_.lightIntensities.push_back(toGlm(intensity));
  }

  void addAreaLight(const vec &position, const vec &color, const vec &,
                    float width, float length) override
  {
    addPointLight(position, { color.x * width * length,
                              color.y * width * length,
                              color.z * width * length });
    std::cerr << "Warning: area light approximated as a point light.\n";
  }

  void addShader(const ShaderDesc &shader) override
  {
    Material material;
    material.diffuse = toGlm(shader.diffuse.data);
    if (shader.type == "Lambertian" || shader.type == "Diffuse") {
      material.blinnPhong = 0.0f;
    } else if (shader.type == "BlinnPhong" || shader.type == "Phong") {
      material.specular = toGlm(shader.specular.data);
      material.shininess = std::max(shader.phongExp, 1.0f);
      material.blinnPhong = 1.0f;
    } else {
      throw std::runtime_error("Unsupported rasterizer shader: " + shader.type);
    }
    if (shader.diffuse.hasTexture || shader.specular.hasTexture) {
      std::cerr << "Warning: textures are not supported; using flat material colors.\n";
    }
    materials_[shader.name] = material;
  }

  void addShape(const ShapeDesc &shape) override
  {
    const auto material = materials_.find(shape.shaderNameReference);
    if (material == materials_.end()) {
      throw std::runtime_error("Shape '" + shape.name + "' references unknown shader '" +
                               shape.shaderNameReference + "'.");
    }
    if (shape.type == "sphere") {
      addSphere(shape, material->second);
    } else if (shape.type == "triangle") {
      addTriangle(shape, material->second, shape.v0, shape.v1, shape.v2);
    } else if (shape.type == "box") {
      addBox(shape, material->second);
    } else if (shape.type == "mesh") {
      addMesh(shape, material->second);
    } else {
      throw std::runtime_error("Unsupported rasterizer shape type: " + shape.type);
    }
  }

  void addTexture(const std::string &type, const std::string &name,
                  const std::string &source) override
  {
    std::cerr << "Warning: texture '" << name << "' (" << type
              << ") is not supported; ignoring '" << source << "'.\n";
  }

private:
  glm::vec3 transformPoint(const ShapeDesc &shape, const glm::vec3 &point) const
  {
    const glm::vec4 result = shape.localXForm * glm::vec4(point, 1.0f);
    if (std::abs(result.w) < 1e-8f) {
      throw std::runtime_error("Shape transform produced an invalid point.");
    }
    return glm::vec3(result) / result.w;
  }

  glm::vec3 transformNormal(const ShapeDesc &shape, const glm::vec3 &normal) const
  {
    const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(shape.localXForm)));
    const glm::vec3 transformed = normalMatrix * normal;
    if (glm::dot(transformed, transformed) < 1e-12f) {
      throw std::runtime_error("Shape transform produced an invalid normal.");
    }
    return glm::normalize(transformed);
  }

  void appendTriangle(const ShapeDesc &shape, const Material &material,
                      const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
  {
    const glm::vec3 p0 = transformPoint(shape, a);
    const glm::vec3 p1 = transformPoint(shape, b);
    const glm::vec3 p2 = transformPoint(shape, c);
    const glm::vec3 crossProduct = glm::cross(p1 - p0, p2 - p0);
    if (glm::dot(crossProduct, crossProduct) < 1e-16f) return;
    const glm::vec3 normal = glm::normalize(crossProduct);
    for (const glm::vec3 &position : { p0, p1, p2 }) {
      scene_.vertices.push_back({ position, normal, material.diffuse, material.specular,
                                  material.shininess, material.blinnPhong });
    }
  }

  void addTriangle(const ShapeDesc &shape, const Material &material,
                   const vec &a, const vec &b, const vec &c)
  {
    appendTriangle(shape, material, toGlm(a), toGlm(b), toGlm(c));
  }

  void addSphere(const ShapeDesc &shape, const Material &material)
  {
    const float radius = shape.radius;
    for (int ring = 0; ring < kSphereRings; ++ring) {
      const float theta0 = kPi * static_cast<float>(ring) / kSphereRings;
      const float theta1 = kPi * static_cast<float>(ring + 1) / kSphereRings;
      for (int segment = 0; segment < kSphereSegments; ++segment) {
        const float phi0 = 2.0f * kPi * static_cast<float>(segment) / kSphereSegments;
        const float phi1 = 2.0f * kPi * static_cast<float>(segment + 1) / kSphereSegments;
        const auto point = [=](float theta, float phi) {
          return toGlm(shape.center) + radius * glm::vec3(
            std::sin(theta) * std::cos(phi), std::cos(theta),
            std::sin(theta) * std::sin(phi));
        };
        const glm::vec3 p00 = point(theta0, phi0);
        const glm::vec3 p01 = point(theta0, phi1);
        const glm::vec3 p10 = point(theta1, phi0);
        const glm::vec3 p11 = point(theta1, phi1);
        appendSphereTriangle(shape, material, p00, p01, p10, radius);
        appendSphereTriangle(shape, material, p01, p11, p10, radius);
      }
    }
  }

  void appendSphereTriangle(const ShapeDesc &shape, const Material &material,
                            const glm::vec3 &a, const glm::vec3 &b,
                            const glm::vec3 &c, float radius)
  {
    for (const glm::vec3 &point : { a, b, c }) {
      const glm::vec3 position = transformPoint(shape, point);
      const glm::vec3 normal = transformNormal(shape, (point - toGlm(shape.center)) / radius);
      scene_.vertices.push_back({ position, normal, material.diffuse, material.specular,
                                  material.shininess, material.blinnPhong });
    }
  }

  void addBox(const ShapeDesc &shape, const Material &material)
  {
    const glm::vec3 minimum = toGlm(shape.minPt);
    const glm::vec3 maximum = toGlm(shape.maxPt);
    const glm::vec3 corners[8] = {
      { minimum.x, minimum.y, minimum.z }, { maximum.x, minimum.y, minimum.z },
      { maximum.x, maximum.y, minimum.z }, { minimum.x, maximum.y, minimum.z },
      { minimum.x, minimum.y, maximum.z }, { maximum.x, minimum.y, maximum.z },
      { maximum.x, maximum.y, maximum.z }, { minimum.x, maximum.y, maximum.z }
    };
    const int faces[12][3] = {
      { 0, 2, 1 }, { 0, 3, 2 }, { 4, 5, 6 }, { 4, 6, 7 },
      { 0, 1, 5 }, { 0, 5, 4 }, { 3, 7, 6 }, { 3, 6, 2 },
      { 0, 4, 7 }, { 0, 7, 3 }, { 1, 2, 6 }, { 1, 6, 5 }
    };
    for (const auto &face : faces) {
      appendTriangle(shape, material, corners[face[0]], corners[face[1]], corners[face[2]]);
    }
  }

  void addMesh(const ShapeDesc &shape, const Material &material)
  {
    ModelOBJ mesh;
    if (shape.meshFilePath.empty() || !mesh.import(shape.meshFilePath.c_str(), true)) {
      throw std::runtime_error("Unable to load mesh for shape '" + shape.name +
                               "': " + shape.meshFilePath);
    }
    if (mesh.getNumberOfIndices() == 0) {
      throw std::runtime_error("Mesh has no triangles: " + shape.meshFilePath);
    }
    const int *indices = mesh.getIndexBuffer();
    for (int index = 0; index < mesh.getNumberOfIndices(); index += 3) {
      const auto &a = mesh.getVertex(indices[index]).position;
      const auto &b = mesh.getVertex(indices[index + 1]).position;
      const auto &c = mesh.getVertex(indices[index + 2]).position;
      appendTriangle(shape, material, { a[0], a[1], a[2] },
                     { b[0], b[1], b[2] }, { c[0], c[1], c[2] });
    }
  }

  RasterScene &scene_;
  std::unordered_map<std::string, Material> materials_;
};

GLuint compileShader(GLenum type, const std::string &path)
{
  FILE *file = nullptr;
  if (fopen_s(&file, path.c_str(), "rb") != 0 || !file) {
    throw std::runtime_error("Unable to open shader: " + path);
  }
  std::string source;
  char buffer[4096];
  size_t count = 0;
  while ((count = std::fread(buffer, 1, sizeof(buffer), file)) > 0) {
    source.append(buffer, count);
  }
  std::fclose(file);

  const char *sourcePointer = source.c_str();
  const GLint sourceLength = static_cast<GLint>(source.size());
  const GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &sourcePointer, &sourceLength);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled != GL_TRUE) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<size_t>(std::max(logLength, 1)), '\0');
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error("Shader compilation failed (" + path + "):\n" + log);
  }
  return shader;
}

GLuint createProgram()
{
  const std::string assetDirectory = OPENGL_ASSET_DIR;
  const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, assetDirectory + "/rasterizer.vert");
  const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, assetDirectory + "/rasterizer.frag");
  const GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  GLint linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked != GL_TRUE) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(static_cast<size_t>(std::max(logLength, 1)), '\0');
    glGetProgramInfoLog(program, logLength, nullptr, log.data());
    glDeleteProgram(program);
    throw std::runtime_error("Shader program link failed:\n" + log);
  }
  return program;
}

void framebufferSizeCallback(GLFWwindow *, int width, int height)
{
  glViewport(0, 0, width, height);
}

struct MouseState {
  bool first = true;
  double lastX = 0.0;
  double lastY = 0.0;
  Camera *camera = nullptr;
};

void mouseCallback(GLFWwindow *window, double x, double y)
{
  auto *state = static_cast<MouseState *>(glfwGetWindowUserPointer(window));
  if (!state || !state->camera) return;
  if (state->first) {
    state->lastX = x;
    state->lastY = y;
    state->first = false;
  }
  const float xOffset = static_cast<float>(x - state->lastX);
  const float yOffset = static_cast<float>(state->lastY - y);
  state->lastX = x;
  state->lastY = y;
  constexpr float sensitivity = 0.12f;
  state->camera->yaw += xOffset * sensitivity;
  state->camera->pitch = std::clamp(state->camera->pitch + yOffset * sensitivity,
                                    -89.0f, 89.0f);
}

void setVec3Array(GLuint program, const char *name,
                  const std::vector<glm::vec3> &values)
{
  for (size_t index = 0; index < values.size(); ++index) {
    const std::string uniformName = std::string(name) + "[" + std::to_string(index) + "]";
    glUniform3fv(glGetUniformLocation(program, uniformName.c_str()), 1,
                 glm::value_ptr(values[index]));
  }
}

int run(const std::string &sceneFile, bool validateOnly)
{
  RasterScene scene;
  auto loader = std::make_shared<RasterSceneLoader>(scene);
  SceneParser_JSON parser(loader);
  parser.parseFileData(sceneFile);
  if (!scene.hasCamera) {
    throw std::runtime_error("The scene must provide a perspective camera.");
  }
  if (scene.vertices.empty()) throw std::runtime_error("The scene contains no renderable triangles.");
  if (validateOnly) {
    std::cout << "Scene loaded: " << scene.vertices.size() / 3 << " triangles, "
              << scene.lightPositions.size() << " point lights.\n";
    return 0;
  }

  if (!glfwInit()) throw std::runtime_error("GLFW initialization failed.");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  GLFWwindow *window = glfwCreateWindow(1200, 800, "OpenGL Real-time Rasterizer", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("GLFW could not create an OpenGL 4.1 window.");
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    glfwDestroyWindow(window);
    glfwTerminate();
    throw std::runtime_error("GLEW initialization failed.");
  }
  glGetError();
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  MouseState mouseState;
  mouseState.camera = &scene.camera;
  glfwSetWindowUserPointer(window, &mouseState);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  GLuint program = 0, vao = 0, vbo = 0;
  try {
    program = createProgram();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(scene.vertices.size() * sizeof(Vertex)),
                 scene.vertices.data(), GL_STATIC_DRAW);
    const GLsizei stride = static_cast<GLsizei>(sizeof(Vertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(Vertex, diffuse)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(Vertex, specular)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(Vertex, shininess)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(Vertex, blinnPhong)));
    glBindVertexArray(0);

    const GLint cameraLocation = glGetUniformLocation(program, "cameraPosition");
    const GLint lightCountLocation = glGetUniformLocation(program, "lightCount");
    const GLint viewMatrixLocation = glGetUniformLocation(program, "viewMatrix");
    const GLint projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");

    double previousTime = glfwGetTime();
    bool screenshotWasPressed = false;
    while (!glfwWindowShouldClose(window)) {
      const double now = glfwGetTime();
      const float deltaTime = static_cast<float>(std::min(now - previousTime, 0.1));
      previousTime = now;
      const float movement = 3.0f * deltaTime;
      const glm::vec3 forward = scene.camera.forward();
      const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) scene.camera.position += forward * movement;
      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) scene.camera.position -= forward * movement;
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) scene.camera.position -= right * movement;
      if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) scene.camera.position += right * movement;
      if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) scene.camera.position.y += movement;
      if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) scene.camera.position.y -= movement;
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

      int width = 0, height = 0;
      glfwGetFramebufferSize(window, &width, &height);
      if (height <= 0 || width <= 0) {
        glfwPollEvents();
        continue;
      }
      glClearColor(scene.background.r, scene.background.g, scene.background.b, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glUseProgram(program);
      const glm::mat4 view = scene.camera.view();
      const glm::mat4 projection = glm::perspective(
        glm::radians(scene.camera.verticalFov), static_cast<float>(width) / height, 0.05f, 500.0f);
      glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));
      glUniform3fv(cameraLocation, 1, glm::value_ptr(scene.camera.position));
      glUniform1i(lightCountLocation, static_cast<GLint>(scene.lightPositions.size()));
      setVec3Array(program, "lightPositions", scene.lightPositions);
      setVec3Array(program, "lightIntensities", scene.lightIntensities);
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(scene.vertices.size()));
      glBindVertexArray(0);
      const bool screenshotPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
      if (screenshotPressed && !screenshotWasPressed) {
        std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        png::image<png::rgb_pixel> screenshot(width, height);
        for (int row = 0; row < height; ++row) {
          for (int column = 0; column < width; ++column) {
            const size_t pixelIndex =
              (static_cast<size_t>(height - row - 1) * width + column) * 3;
            screenshot[row][column] = png::rgb_pixel(
              pixels[pixelIndex], pixels[pixelIndex + 1], pixels[pixelIndex + 2]);
          }
        }
        screenshot.write("rasterized.png");
        std::cout << "Saved rasterized.png\n";
      }
      screenshotWasPressed = screenshotPressed;
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  } catch (...) {
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (program) glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    throw;
  }
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
} // namespace

int main(int argc, char **argv)
{
  bool validateOnly = false;
  std::string sceneFile = std::string(OPENGL_ASSET_DIR) + "/creative_scene.json";
  bool sceneProvided = false;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--validate") {
      validateOnly = true;
    } else if (!sceneProvided) {
      sceneFile = argument;
      sceneProvided = true;
    } else {
      std::cerr << "Usage: rasterizer [--validate] [scene.json]\n";
      return EXIT_FAILURE;
    }
  }
  if (sceneFile.empty()) {
    std::cerr << "Usage: rasterizer [--validate] [scene.json]\n";
    return EXIT_FAILURE;
  }
  try {
    return run(sceneFile, validateOnly);
  } catch (const std::exception &error) {
    std::cerr << "Rasterizer error: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
