#version 430

// The texture to be mapped
uniform sampler2D fboTex;
uniform sampler2D fboDepthTex;
uniform sampler2D shadowMap;

// The depth at which we want to focus
uniform float focalDepth = 0.5;

// A scale factor for the radius of blur
uniform float blurRadius = 0.008;

// The output colour. At location 0 it will be sent to the screen.
layout (location=0) out vec4 FragColour;

// The fragment UV coordinates
in vec2 FragmentUV;

/***************************************************************************************************
 * Gaussian Blur functions and constants
 ***************************************************************************************************/
// Gaussian coefficients
const float G5x5[25] = {0.0035,    0.0123,    0.0210,    0.0123,    0.0035,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0210,    0.0911,    0.2224,    0.0911,    0.0210,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0035,    0.0123,    0.0210,    0.0123,    0.0035};

const float G9x9[81] = {0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0273,    0.0430,    0.0469,    0.0430,    0.0273,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0};

// These define which Gaussian kernel and the size to use (G5x5 and 5 also possible)
#define SZ 9
#define G  G9x9


/***************************************************************************************************
 * Poisson Blur functions and constants
 ***************************************************************************************************/
// Tap locations for 13 sample Poisson unit disc
const vec2 PoissonDisc[12] = {vec2(-0.326212,-0.40581),vec2(-0.840144,-0.07358),
                              vec2(-0.695914,0.457137),vec2(-0.203345,0.620716),
                              vec2(0.96234,-0.194983),vec2(0.473434,-0.480026),
                              vec2(0.519456,0.767022),vec2(0.185461,-0.893124),
                              vec2(0.507431,0.064425),vec2(0.89642,0.412458),
                              vec2(-0.32194,-0.932615),vec2(-0.791559,-0.59771)};

float poissonWeights[12];

// A pseudorandom generator for the Poisson disk rotation
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 poissBlur(vec2 _fragmentUV, float _adjBRadius)
{
    vec4 poiss = vec4(0, 0, 0, 0);
    for(int i = 0; i < 12; i++)
    {
        poiss += texture(fboTex, (FragmentUV + PoissonDisc[i] * _adjBRadius)) * poissonWeights[i];
    }
    return poiss;
}

void main()
{
    float sigInvLength = 0;
    for(int i = 0; i < 12; i++)
    {
        poissonWeights[i] = 1/length(PoissonDisc[i]);
        sigInvLength += poissonWeights[i];
    }

    for(int i = 0; i < 12; i++)
    {
        poissonWeights[i] /= sigInvLength;
    }

    vec2 texpos = FragmentUV;

    float adjBRadius = blurRadius * abs(texture(fboDepthTex, FragmentUV).r - focalDepth);
    vec4 blurred = poissBlur(FragmentUV, adjBRadius);

    vec4 testCol = texture(fboTex, FragmentUV);
    // Determine sigma, the blur radius of this pixel here!
    // Call your blurFunction on the input texture based on the blur radius here to return a texture

    // Output a bit of the depth texture in this image for debugging
    //FragColour = (texpos.x < 0.5)?texture(fboTex, texpos):texture(fboDepthTex, texpos).rrrr;
    //FragColour = (texpos.x < 0.5)?maxGauss:texture(fboDepthTex, texpos).rrrr;
    FragColour = blurred;
}

