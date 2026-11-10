#ifndef OPENGL_CAMERA_H
#define OPENGL_CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class OpenGLCamera
{
public:
  OpenGLCamera(float aspectRatio)
    : position(0.0f, 0.0f, 0.0f),
      right(1.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      backward(0.0f, 0.0f, 1.0f),
      aspectRatio(aspectRatio)
  {
  }

  glm::mat4 getViewMatrix() const
  {
    return glm::lookAt(position, position - backward, up);
  }

  glm::mat4 getProjectionMatrix() const
  {
    return glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);
  }

  void moveForward(float distance)
  {
    position -= backward * distance;
  }

  void moveBackward(float distance)
  {
    position += backward * distance;
  }

  void moveLeft(float distance)
  {
    position -= right * distance;
  }

  void moveRight(float distance)
  {
    position += right * distance;
  }

private:
  glm::vec3 position;
  glm::vec3 right;
  glm::vec3 up;
  glm::vec3 backward;
  float aspectRatio;
};

#endif
