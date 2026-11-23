#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 vertexDiffuse;
layout(location = 3) in vec3 vertexSpecular;
layout(location = 4) in float vertexShininess;
layout(location = 5) in float vertexBlinnPhong;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 worldPosition;
out vec3 worldNormal;
out vec3 diffuseColor;
out vec3 specularColor;
out float shininess;
out float blinnPhong;

void main()
{
  worldPosition = vertexPosition;
  worldNormal = normalize(vertexNormal);
  diffuseColor = vertexDiffuse;
  specularColor = vertexSpecular;
  shininess = vertexShininess;
  blinnPhong = vertexBlinnPhong;
  gl_Position = projectionMatrix * viewMatrix * vec4(vertexPosition, 1.0);
}
