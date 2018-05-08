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
uniform sampler2D logoMap;
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
in vec3 eyePos;
in vec3 FragNormal;
in vec4 FragmentPosition;
in vec2 FragmentTexCoord;

//Constants
const float small = 0.0001f;
const float pi = 3.1415926f;

vec3 specularComponent(vec3 _n, vec3 _v, vec3 _s, float _roughness, vec3 _fInc)
{
  float m = _roughness;
  float mSquared = pow(m, 2);

  //Beckmannn Distribution
  vec3 h = normalize(_v + _s);
  float NdotH = dot(_n, h);
  float VdotH = dot(_v, h);
  float NdotV = dot(_n, _v);
  float NdotL = dot(_n, _s);
  float r1  = 1.f / (4.f * mSquared * pow(NdotH, 4.f));
  float r2 = (NdotH * NdotH - 1.f) / (mSquared * NdotH * NdotH);
  float D = r1 * exp(r2);

  float denom = NdotH * NdotH * (mSquared - 1.f) + 1.f;
  D = mSquared / (pi * denom * denom);

  // Geometric attenuation
  float NH2 = 2.f * NdotH;
  float invVdotH = (VdotH > small)?(1.f / VdotH):1.f;
  float g1 = (NH2 * NdotV) * invVdotH;
  float g2 = (NH2 * NdotL) * invVdotH;
  float G = min(1.f, min(g1, g2));

  // Schlick approximation
  float F0 = 0.2; // Fresnel reflectance at normal incidence
  float F_r = pow(1.0 - VdotH, 5.0) * (1.0 - _fInc.r) + _fInc.r;
  float F_g = pow(1.0 - VdotH, 5.0) * (1.0 - _fInc.g) + _fInc.g;
  float F_b = pow(1.0 - VdotH, 5.0) * (1.0 - _fInc.b) + _fInc.b;
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

/******************************************************
  * The following simplex noise functions have been taken from WebGL-noise
  * https://github.com/stegu/webgl-noise/blob/master/src/noise2D.glsl
  *>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v) {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                + i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}
/*********** END REFERENCE ************************/

/***************************************************
  * This function is ported from
  * https://cmaher.github.io/posts/working-with-simplex-noise/
  ****************************************************/
float sumOctave(in vec2 pos,
                in int num_iterations,
                in float persistence,
                in float scale,
                in float low,
                in float high) {
    float maxAmp = 0.0f;
    float amp = 1.0f;
    float freq = scale;
    float noise = 0.0f;
    int i;

    for (i = 0; i < num_iterations; ++i) {
        noise += snoise(pos * freq) * amp;
        maxAmp += amp;
        amp *= persistence;
        freq *= 2.0f;
    }
    noise /= maxAmp;
    noise = noise*(high-low)*0.5f + (high+low)*0.5f;
    return noise;
}

float brushedNoise()
{
  vec2 PN = FragmentTexCoord;
  PN[0] *= 100;
  PN[1] *= 1;
  
  float grainNoise = sumOctave(PN, 10, 0.5f, 1.f, 0.5f, 0.6f);
  return grainNoise;
}

float quadratic(float _var, float _start, float _end)
{
  float big = pow((_end - _start), 4);
  float quadvar = pow(((_var - _start) * (_end - _start)), 2) / big;
  return quadvar;
}

float scratchGen(float _angle, float _start, float _end, vec2 _point, float _stretch)
{  
  float cosAngle = cos(_angle);
  float sinAngle = sin(_angle);
  _point[0] *= _stretch;
  _point[0] = (_point[0] * cosAngle) - (_point[0] * sinAngle);
  _point[1] = (_point[1] * sinAngle) + (_point[1] * cosAngle);

  float s = sumOctave(_point, 10, 0.5f, 1.f, 0.f, 1.f);
  s = quadratic(s, _start, _end);
  return s;
}

float scratchRoughness()
{
  float pi_3 = pi * 0.3333f;
  vec2 PS = FragmentTexCoord * 8.f;
  PS = PS + sumOctave(PS, 10, 0.5f, 1.f, 0.f, 1.f) * 0.4f;
  float s1 = scratchGen(pi_3, 0.f, 1.f, PS, 20.f);
  s1 = (s1 < 0.4f)?0.f:s1;
  float s2 = scratchGen(pi * 0.1666f, 0.f, 1.f, PS, 20.f);
  s2 = (s2 < 0.4f)?0.f:s2;

  PS = FragmentTexCoord * 10.f;
  float sMask1 = scratchGen(pi_3, 0.0f, 0.8f, PS, 1.f);

  PS = FragmentTexCoord * 10.f + 10.f;
  float sMask2 = scratchGen(0.f, 0.f, 0.8f, PS, 1.f);
  float scratches = sMask1 * sMask2 * min(s1 + s2, 1.f);
  return scratches;
}

vec3 over(vec4 _a, vec4 _b)
{
  return _a.rgb + _b.rgb * (1 - _a.a);
}

void main()
{
  vec3 n = normalize(FragNormal);
  vec3 v = vec3(0.0f, 0.0f, 1.0f);
  v = normalize(-eyePos);

  vec3 p = FragmentPosition.xyz / FragmentPosition.w;
  vec3 lookup = (reflect(-v,n));
  lookup.z *= -1;
  lookup.y *= -1;

  vec3 totalLight = vec3(0.f);

  float logo = texture(logoMap, FragmentTexCoord).a;
  vec3 diffuseColour = over(vec4(vec3(logo * 0.3), logo), vec4(materialDiff, 1.f));
  float rough = brushedNoise() - scratchRoughness() * 0.2;
  rough = over(vec4(vec3(logo * 0.6), logo), vec4(rough)).r;

  float fuzz = 0.005f;
  float topWear = ((smoothstep(0.07f - fuzz, 0.07f + fuzz, FragmentTexCoord.x) - smoothstep(0.105f - fuzz, 0.105f + fuzz, FragmentTexCoord.x)) +
                   (smoothstep(0.195f - fuzz, 0.195f + fuzz, FragmentTexCoord.x) - smoothstep(0.23f - fuzz, 0.23f + fuzz, FragmentTexCoord.x))) *
                   (smoothstep(0.87f - fuzz, 0.87f + fuzz, FragmentTexCoord.y) - (smoothstep(1.f - fuzz, 1.f + fuzz, FragmentTexCoord.y)));
  topWear = topWear * -0.2f + 1.f;
  rough *= topWear;

  n = rotate(n,v,rough * 1.2f);

  for(int i = 0; i < 14; i++)
  {
    vec3 transformedLightPos = lightPos[i] * 100.f;
    float distance = length(transformedLightPos - eyePos.xyz);
    float attenuation = 1.0 / (distance * distance);
    vec3 s = normalize(transformedLightPos - eyePos.xyz);

    vec3 specComponent = specularComponent(n, v, s, roughness, vec3(metallic, metallic, metallic));
    specComponent = max(min(specComponent, 1), 0);

    vec3 specularIntensity = textureLod(envMap, lookup, rough * envMapMaxLod).rgb;

    vec3 diffuseIntensity = lightCol[i] * max(dot(s, n), 0.0);

    totalLight += vec3((diffuseIntensity * diffAmount * diffuseColour) +
                       (specularIntensity * specComponent * specAmount * materialSpec));
  }

  FragColor = vec4(totalLight , alpha);
}
