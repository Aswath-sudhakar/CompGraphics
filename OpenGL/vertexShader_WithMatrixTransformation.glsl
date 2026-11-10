#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 interpolatedColor;

void main(void)
{
  gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
  interpolatedColor = vertexColor;
}
