#version 430

//FragColor
layout (location=0) out vec4 FragColor;

//Matrices
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix;

//Light info
uniform vec3 lightPos[14];
uniform vec3 lightCol[14];

//Textures
uniform samplerCube envMap;
uniform sampler2D normalMap;
uniform int envMapMaxLod;

//Shader parameters
uniform float roughness;
uniform float metallic;
uniform float diffAmount;
uniform float specAmount;
uniform vec3 materialDiff;
uniform vec3 materialSpec;
uniform float alpha;

// Vectors from the vertex shader.
in vec3 worldPos;
in vec3 FragNormal;
in vec4 FragmentPosition;
in vec2 FragmentTexCoord;

void main()
{
  vec3 n = normalize(FragNormal);
  vec3 transformedLightPos = lightPos[0] * 100.f;
  float distance = length(transformedLightPos - worldPos.xyz);
  float attenuation = 1.0 / (distance * distance);
  vec3 s = normalize(transformedLightPos - worldPos.xyz);
  float diffuseIntensity = max(dot(s, n), 0.0);

  FragColor = vec4(vec3(0.f), (-diffuseIntensity + 1.f) * 0.5f);
}
