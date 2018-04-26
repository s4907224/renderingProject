#version 430

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix; // This is the inverse transpose of the MV matrix
uniform mat4 invV;
uniform mat4 M;

// The vertex position attribute
layout (location=0) in vec3 VertexPosition;

// The texture coordinate attribute
layout (location=1) in vec2 TexCoord;

// The vertex normal attribute
layout (location=2) in vec3 VertexNormal;

out vec4 FragmentPosition;
out vec3 FragNormal;
out vec3 wsFragmentPosition;
out vec2 FragmentTexCoord;
out vec3 worldPos;

/************************************************************************************/
void main() {
    // Set the position of the current vertex
    gl_Position = MVP * vec4(VertexPosition, 1.0);
    worldPos = vec3(MV * vec4(VertexPosition, 1.0) );
    wsFragmentPosition = VertexPosition;
    FragmentPosition = gl_Position;
    FragNormal = normalize(normalMatrix * VertexNormal);
    FragmentTexCoord = TexCoord;
}
