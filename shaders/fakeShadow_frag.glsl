#version 430

//FragColor
layout (location=0) out vec4 FragColor;

//Matrices
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix;

//Light info
uniform vec3 lightPos;

// Vectors from the vertex shader.
in vec3 eyePos;
in vec3 FragNormal;
in vec4 FragmentPosition;

void main()
{
  vec3 n = normalize(FragNormal);
  vec3 transformedLightPos = lightPos * 100.f;
  float distance = length(transformedLightPos - eyePos.xyz);
  float attenuation = 1.0 / (distance * distance);
  vec3 s = normalize(transformedLightPos - eyePos.xyz);
  float diffuseIntensity = max(dot(s, n), 0.0);

  FragColor = vec4(vec3(0.f), (-diffuseIntensity + 1.f) * 0.5f);
}
