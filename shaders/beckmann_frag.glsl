#version 430

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix; // This is the inverse transpose of the MV matrix
uniform mat4 invV;

// The fragment position attribute
layout (location=0) out vec4 FragColor;

// The material properties of our object
struct MaterialInfo
{
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
};

// The object has a material
uniform MaterialInfo Material = MaterialInfo
(
  vec3(0.1, 0.1, 0.1),    // Ka
  vec3(1.0, 1.0, 1.0),    // Kd
  vec3(1.0, 1.0, 1.0),    // Ks
  10.0                  // Shininess
);

// This is passed on from the vertex shader
in vec3 FragNormal;
in vec4 FragmentPosition;
in vec2 FragmentTexCoord;
in vec3 wsFragmentPosition;
uniform samplerCube envMap;
uniform sampler2D normalMap;
uniform float roughness;

vec3 beckmann(vec3 _n, vec3 _v, vec3 _s, float _roughness)
{
  //Half vector
  vec3 h = normalize(_v + _s);
  // Distribution function
  float m = _roughness;
  float mSquared = m * m;
  float NdotH = dot(_n, h); //dot product of surface and light position
  float VdotH = dot(_v, h); //dot product of surface and light position
  float NdotV = dot(_n, _v); //dot product of surface and light position
  float NdotL = dot(_n, _s); //dot product of surface and light position
  float r1  = 1.f / (4.f * mSquared * pow(NdotH, 4.f));
  float r2 = (NdotH * NdotH - 1.f) / (mSquared * NdotH * NdotH);
  float D = r1 * exp(r2);

  // Geometric attenuation
  float NH2 = 2.f * NdotH;
  float eps = 0.0001f;
  float invVdotH = (VdotH > eps)?(1.f / VdotH):1.f;
  float g1 = (NH2 * NdotV) * invVdotH;
  float g2 = (NH2 * NdotL) * invVdotH;
  float G = min(1.f, min(g1, g2));

  // Schlick approximation
  float F0 = 0.01; // Fresnel reflectance at normal incidence
  float F_r = pow(1.0 - VdotH, 5.0) * (1.0 - F0) + F0;
  F0 = 0.01; // Fresnel reflectance at normal incidence
  float F_g = pow(1.0 - VdotH, 5.0) * (1.0 - F0) + F0;
  F0 = 0.01; // Fresnel reflectance at normal incidence
  float F_b = pow(1.0 - VdotH, 5.0) * (1.0 - F0) + F0;
  vec3 F = vec3(F_r, F_g, F_b);

  // Compute the light from the ambient, diffuse and specular components
  return F * D / NdotV;
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec3 rotate(vec3 v, vec3 axis, float angle)
{
        mat4 m = rotationMatrix(axis, angle);
        return (m * vec4(v, 1.0)).xyz;
}

void main()
{
  vec3 n = normalize(FragNormal);
  vec3 v = vec3(0.0f, 0.0f, 1.0f);
  //v = normalize(FragmentPosition.xyz);

  vec3 normalValue = normalize(texture(normalMap, FragmentTexCoord).xyz);
  float cosAngle = dot(n, normalValue);
  float angle = acos(cosAngle);
  //n = rotate(n,v,angle);

  vec3 p = FragmentPosition.xyz / FragmentPosition.w;
  vec3 s = normalize(-reflect(v, n));
  //s = normalize(Light1.Position).xyz;
  vec3 lookup = -s;
  lookup.x *= -1;
  //lookup.z *= -1;
  //lookup = (vec4(lookup, 1)).xyz;
  vec3 brdf = beckmann(n, v, s, roughness);
  vec3 specular = textureLod(envMap, lookup, 16 * roughness).rgb;
  brdf = max(brdf, 0);
  brdf = min(brdf, 1);
  specular *= brdf;

  vec3 indirectDiffuse = textureLod(envMap, lookup, 16).rgb;

  vec3 lightSig =  indirectDiffuse + (specular) + Material.Ka;
  //lightSig *= vec3(0.878431372549, 0.349019607843, 0.56862745098);


  FragColor = vec4(specular , 1.f);
  FragColor = vec4(indirectDiffuse, 1.f);
}
