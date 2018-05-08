#version 430

//Matrices
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix; // This is the inverse transpose of the MV matrix

//Mesh data
layout (location=0) in vec3 VertexPosition;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 VertexNormal;


out vec4 FragmentPosition;
out vec3 FragNormal;
out vec2 FragmentTexCoord;
out vec3 eyePos;

/************************************************************************************/
void main()
{
  // Set the position of the current vertex
  gl_Position = MVP * vec4(VertexPosition, 1.0);
  eyePos = vec3(MV * vec4(VertexPosition, 1.0) );
  FragmentPosition = gl_Position;
  FragNormal = normalize(normalMatrix * VertexNormal);
  FragmentTexCoord = TexCoord;
}
