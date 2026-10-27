#pragma once

#include "Pcamera.h"
#include "light.h"
#include "shape_list.h"
#include "sphere.h"
#include "triangle.h"
#include "ISceneLoader.h"
#include "model_obj.h"

#include <cmath>
#include <cctype>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

struct RenderScene
{
  std::shared_ptr<perspective_camera> camera;
  shape_list shapes;
  std::vector<point_light> lights;
  vec3 background{ 0.0, 0.0, 0.0 };
};

class SceneLoader : public ISceneLoader
{
private:
  struct Material
  {
    vec3 color;
    Shading shading;
    double specularStrength = 0.0;
    double shininess = 32.0;
    double reflectivity = 0.0;
  };

  RenderScene &m_targetScene;
  int m_imageWidth, m_imageHeight;

  std::unordered_map<std::string, Material> m_shaders;

  static vec3 toVec3(const ISceneLoader::vec &value)
  {
    return vec3(value.x, value.y, value.z);
  }

  glm::vec3 transformed(const ISceneLoader::ShapeDesc &shape, const ISceneLoader::vec &point) const
  {
    const glm::vec4 result = shape.localXForm * glm::vec4(point.x, point.y, point.z, 1.0f);
    if (std::abs(result.w) < 1e-8f) {
      throw std::runtime_error("Shape transform produced an invalid point.");
    }
    return glm::vec3(result) / result.w;
  }


public:
  SceneLoader(RenderScene &sceneToPopulate, int imageWidth, int imageHeight)
    : m_targetScene(sceneToPopulate), m_imageWidth(imageWidth), m_imageHeight(imageHeight) {}

  void reserveCameras(size_t count) override
  {
    if (count > 1) {
      throw std::runtime_error("This render supports only one camera for now");
    }
  }

  void reserveLights(size_t count) override
  {
    m_targetScene.lights.reserve(count);
  }

  void reserveShapes(size_t count) override
  {
    (void)count;
  }

  void reserveShaders(size_t count) override { m_shaders.reserve(count); }
  void reserveTextures(size_t count) override { (void)count; }

  void setSceneParams(const SceneParams &sparams) override
  {
    m_targetScene.background = toVec3(sparams.bgColor);
    if (!sparams.envMapFile.empty()) {
      std::cerr << "Warning: environment maps are not supported; ignoring '" << sparams.envMapFile << "'.\n";
    }
  }

  void addCamera(
    const std::string &name,
    const std::string &type,
    const ISceneLoader::vec &pos,
    const ISceneLoader::vec &viewDir,
    float focalLength,
    float imagePlaneWidth) override
  {
    (void)name;
    std::string normalizedType = type;
    for (char &character : normalizedType) {
      character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    if (normalizedType != "perspective" && normalizedType != "perspective_camera") {
      throw std::runtime_error("Unsupported camera type: " + type);
    }
    if (m_imageWidth <= 0 || m_imageHeight <= 0 || imagePlaneWidth <= 0.0f || focalLength <= 0.0f) {
      throw std::runtime_error("Camera dimensions and focal length must be positive.");
    }
    const double planeHeight = imagePlaneWidth * m_imageHeight / m_imageWidth;
    m_targetScene.camera = std::make_shared<perspective_camera>(
      point3(pos.x, pos.y, pos.z), toVec3(viewDir), imagePlaneWidth, planeHeight, focalLength);
  }


  void addPointLight(
    const ISceneLoader::vec &pos,
    const ISceneLoader::vec &intensity) override
  {
    m_targetScene.lights.push_back({ point3(pos.x, pos.y, pos.z), toVec3(intensity) });
  }


  void addAreaLight(
    const ISceneLoader::vec &pos,
    const ISceneLoader::vec &color,
    const ISceneLoader::vec &normal,
    float width,
    float length) override
  {
    (void)normal;
    m_targetScene.lights.push_back({ point3(pos.x, pos.y, pos.z), width * length * toVec3(color) });
    std::cerr << "Warning: area light approximated as a point light.\n";
  }


  void addShader(const ISceneLoader::ShaderDesc &shaderDesc) override
  {
    Material material{ toVec3(shaderDesc.diffuse.data), LAMBERT };
    if (shaderDesc.type == "Lambertian" || shaderDesc.type == "Diffuse") {
      material.shading = LAMBERT;
    } else if (shaderDesc.type == "BlinnPhong" || shaderDesc.type == "Phong") {
      material.shading = BLINN;
      material.specularStrength = (shaderDesc.specular.data.x + shaderDesc.specular.data.y + shaderDesc.specular.data.z) / 3.0;
      material.shininess = shaderDesc.phongExp;
    } else if (shaderDesc.type == "Mirror") {
      material.color = vec3(1.0, 1.0, 1.0);
      material.shading = MIRROR;
    } else if (shaderDesc.type == "Glaze" || shaderDesc.type == "BlinnPhongMirrored") {
      material.shading = BLINN;
      material.specularStrength = (shaderDesc.specular.data.x + shaderDesc.specular.data.y + shaderDesc.specular.data.z) / 3.0;
      material.shininess = shaderDesc.phongExp;
      material.reflectivity = shaderDesc.mirrorCoef;
    } else if (shaderDesc.type == "CoolToWarm") {
      material.shading = LAMBERT;
    } else {
      throw std::runtime_error("Unsupported shader type: " + shaderDesc.type);
    }

    if (shaderDesc.diffuse.hasTexture || shaderDesc.specular.hasTexture) {
      std::cerr << "Warning: shader textures are not supported; using the supplied flat color.\n";
    }
    m_shaders[shaderDesc.name] = material;
  }

  void addShape(const ISceneLoader::ShapeDesc &shapeDesc) override
  {
    const auto shaderIt = m_shaders.find(shapeDesc.shaderNameReference);

    if (shaderIt == m_shaders.end()) {
      throw std::runtime_error("Shape '" + shapeDesc.name + "' references unknown shader '" + shapeDesc.shaderNameReference + "'.");
    }

    const Material &material = shaderIt->second;
    const auto addTriangle = [&](const ISceneLoader::vec &v0, const ISceneLoader::vec &v1, const ISceneLoader::vec &v2) {
      const glm::vec3 a = transformed(shapeDesc, v0);
      const glm::vec3 b = transformed(shapeDesc, v1);
      const glm::vec3 c = transformed(shapeDesc, v2);
      m_targetScene.shapes.add(std::make_shared<triangle>(
        point3(a.x, a.y, a.z), point3(b.x, b.y, b.z), point3(c.x, c.y, c.z), material.color, material.shading, material.specularStrength, material.shininess, material.reflectivity));
    };

    if (shapeDesc.type == "sphere") {
      const glm::vec3 center = transformed(shapeDesc, shapeDesc.center);
      const double scaleX = shapeDesc.hasXForm ? glm::length(glm::vec3(shapeDesc.localXForm[0])) : 1.0;
      const double scaleY = shapeDesc.hasXForm ? glm::length(glm::vec3(shapeDesc.localXForm[1])) : 1.0;
      const double scaleZ = shapeDesc.hasXForm ? glm::length(glm::vec3(shapeDesc.localXForm[2])) : 1.0;
      if (std::abs(scaleX - scaleY) > 1e-5 || std::abs(scaleX - scaleZ) > 1e-5) {
        throw std::runtime_error("Sphere transforms must use uniform scaling.");
      }
      m_targetScene.shapes.add(std::make_shared<sphere>(
        point3(center.x, center.y, center.z), shapeDesc.radius * scaleX, material.color, material.shading, material.specularStrength, material.shininess, material.reflectivity));
      return;
    }

    if (shapeDesc.type == "triangle") {
      addTriangle(shapeDesc.v0, shapeDesc.v1, shapeDesc.v2);
      return;
    }

    if (shapeDesc.type == "box") {
      const auto &min = shapeDesc.minPt;
      const auto &max = shapeDesc.maxPt;
      const ISceneLoader::vec corners[8] = {
        { min.x, min.y, min.z }, { max.x, min.y, min.z }, { max.x, max.y, min.z }, { min.x, max.y, min.z }, { min.x, min.y, max.z }, { max.x, min.y, max.z }, { max.x, max.y, max.z }, { min.x, max.y, max.z }
      };
      const int faces[12][3] = {
        { 0, 2, 1 }, { 0, 3, 2 }, { 4, 5, 6 }, { 4, 6, 7 }, { 0, 1, 5 }, { 0, 5, 4 }, { 3, 7, 6 }, { 3, 6, 2 }, { 0, 4, 7 }, { 0, 7, 3 }, { 1, 2, 6 }, { 1, 6, 5 }
      };
      for (const auto &face : faces) {
        addTriangle(corners[face[0]], corners[face[1]], corners[face[2]]);
      }
      return;
    }

    if (shapeDesc.type == "mesh") {
      ModelOBJ mesh;
      if (shapeDesc.meshFilePath.empty() || !mesh.import(shapeDesc.meshFilePath.c_str(), true)) {
        throw std::runtime_error("Unable to load mesh for shape '" + shapeDesc.name + "': " + shapeDesc.meshFilePath);
      }
      if (mesh.getNumberOfIndices() == 0) {
        throw std::runtime_error("Mesh has no triangles: " + shapeDesc.meshFilePath);
      }
      const int *indices = mesh.getIndexBuffer();
      for (int index = 0; index < mesh.getNumberOfIndices(); index += 3) {
        const auto &v0 = mesh.getVertex(indices[index]).position;
        const auto &v1 = mesh.getVertex(indices[index + 1]).position;
        const auto &v2 = mesh.getVertex(indices[index + 2]).position;
        addTriangle({ v0[0], v0[1], v0[2] }, { v1[0], v1[1], v1[2] }, { v2[0], v2[1], v2[2] });
      }
      return;
    }

    throw std::runtime_error("Unsupported shape type: " + shapeDesc.type);
  }

  void addTexture(
    const std::string &type,
    const std::string &name,
    const std::string &sourceFile) override
  {
    std::cerr << "Warning: texture '" << name << "' (" << type << ") is not supported; ignoring '" << sourceFile << "'.\n";
  }
};
