#version 430

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix; // This is the inverse transpose of the MV matrix

// The fragment position attribute
layout (location=0) out vec4 FragColor;

struct LightInfo
{
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene with some default values
uniform LightInfo Light = LightInfo
  (
    vec4(2.0, 2.0, 10.0, 1.0),   // position
    vec3(0.2, 0.2, 0.2),        // La
    vec3(1.0, 1.0, 1.0),        // Ld
    vec3(1.0, 1.0, 1.0)         // Ls
  );

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


void main()
{
    vec3 n = normalize(FragNormal);
    vec3 p = FragmentPosition.xyz / FragmentPosition.w;
    vec3 s = normalize( vec3(Light.Position) - p); //this is s for the h equation
    vec3 v = vec3(0.0f,0.0f,1.0f);
    vec3 r = reflect( -s, n );

    //Half vector
    vec3 h = normalize(v+s);

    // Distribution function
    float m = 0.8f;
    float mSquared = m*m;
    float NdotH = dot(n, h); //dot product of surface and light position
    float VdotH = dot(v, h); //dot product of surface and light position
    float NdotV = dot(n, v); //dot product of surface and light position
    float NdotL = dot(n, s); //dot product of surface and light position

    float r1  = 1.0 / (4.0f * mSquared * pow(NdotH, 4.0f));
    float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
    float D = r1 * exp(r2);

    // Geometric attenuation
    float NH2 = 2.0 * NdotH;
    float eps = 0.0001f;
    float invVdotH = (VdotH > eps)?(1.0 / VdotH):1.0;
    float g1 = (NH2 * NdotV) * invVdotH;
    float g2 = (NH2 * NdotL) * invVdotH;
    float G = min(1.0, min(g1, g2));

    // Schlick approximation
    float F0 = 0.1; // Fresnel reflectance at normal incidence
    float F_r = pow(1.0 - VdotH, 5.0) * (1.0 - F0) + F0;
    F0 = 0.1; // Fresnel reflectance at normal incidence
    float F_g = pow(1.0 - VdotH, 5.0) * (1.0 - F0) + F0;
    F0 = 0.1; // Fresnel reflectance at normal incidence
    float F_b = pow(1.0 - VdotH, 5.0) * (1.0 - F0) + F0;

    // Compute the light from the ambient, diffuse and specular components
    vec3 spec = G * vec3(F_r, F_g, F_b) * D / NdotV;
    //float spec = G * F * D / NdotV;
    vec3 LightIntensity = (
            Light.La * Material.Ka +
            Light.Ld * Material.Kd * max( dot(s, n), 0.0 ) +
            Light.Ls * Material.Ks * spec);

    FragColor = vec4(LightIntensity,1.0);
}
