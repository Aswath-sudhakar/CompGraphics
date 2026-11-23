#version 410 core

const int MAX_LIGHTS = 8;

in vec3 worldPosition;
in vec3 worldNormal;
in vec3 diffuseColor;
in vec3 specularColor;
in float shininess;
in float blinnPhong;

uniform vec3 cameraPosition;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightIntensities[MAX_LIGHTS];
uniform int lightCount;

out vec4 fragmentColor;

void main()
{
  vec3 normal = normalize(worldNormal);
  vec3 viewDirection = normalize(cameraPosition - worldPosition);
  vec3 color = diffuseColor * 0.08;

  for (int index = 0; index < lightCount; ++index) {
    vec3 toLight = lightPositions[index] - worldPosition;
    float distanceToLight = length(toLight);
    vec3 lightDirection = toLight / max(distanceToLight, 0.0001);
    float attenuation = 1.0 / (1.0 + 0.04 * distanceToLight +
                               0.01 * distanceToLight * distanceToLight);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    color += diffuseColor * lightIntensities[index] * diffuse * attenuation;

    if (blinnPhong > 0.5 && diffuse > 0.0) {
      vec3 halfDirection = normalize(lightDirection + viewDirection);
      float specular = pow(max(dot(normal, halfDirection), 0.0),
                           max(shininess, 1.0));
      color += specularColor * lightIntensities[index] * specular * attenuation;
    }
  }

  fragmentColor = vec4(color, 1.0);
}
