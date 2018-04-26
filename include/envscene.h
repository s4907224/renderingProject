#ifndef ENVSCENE_H
#define ENVSCENE_H

#include <ngl/Obj.h>
#include "scene.h"

class EnvScene : public Scene
{
public:
    EnvScene();

    /// Called when the scene needs to be painted
    void paintGL() noexcept;

    /// Called when the scene is to be initialised
    void initGL() noexcept;

    float m_roughness;

private:
    /// The ID of our environment texture
    GLuint m_envTex, m_glossMapTex;

    /// Initialise the entire environment map
    void initEnvironment();

    /// Utility function for loading up a 2D texture
    void initTexture(const GLuint&, GLuint &, const char *);

    /// Initialise a single side of the environment map
    void initEnvironmentSide(GLenum /*target*/, const char* /*filename*/);

    void initFBO();
    bool m_isFBODirty = true;
    GLuint m_fboID, m_fboTextureID, m_fboDepthID;

    ngl::Obj *m_mesh;

    std::array<ngl::Vec3, 14> m_lightPos = {ngl::Vec3( 1.506f, 0.815f, 0.041f),
                                            ngl::Vec3( 0.079f, 0.609f,-1.026f),
                                            ngl::Vec3( 3.534f, 0.432f, 3.913f),
                                            ngl::Vec3( 1.254f, 0.453f, 4.827f),
                                            ngl::Vec3( 0.783f, 0.837f,-0.679f),
                                            ngl::Vec3( 0.783f, 0.239f,-0.679f),
                                            ngl::Vec3( 1.568f, 0.246f, 0.037f),
                                            ngl::Vec3(-0.069f, 0.031f,-1.023f),
                                            ngl::Vec3(-3.215f, 0.069f, 1.761f),
                                            ngl::Vec3(-1.340f, 0.217f,-1.599f),
                                            ngl::Vec3( 0.073f,-0.432f, 0.045f),
                                            ngl::Vec3( 0.419f,-0.106f, 0.675f),
                                            ngl::Vec3( 0.091f, 0.822f, 1.050f),
                                            ngl::Vec3(-0.231f,-0.938f, 1.825f)};

    std::array<ngl::Vec3, 14> m_lightCol = {ngl::Vec3(0.551f, 0.887f, 1.000f),
                                            ngl::Vec3(0.432f, 0.795f, 0.995f),
                                            ngl::Vec3(0.485f, 0.542f, 0.562f),
                                            ngl::Vec3(0.314f, 0.384f, 0.296f),
                                            ngl::Vec3(0.390f, 0.735f, 0.999f),
                                            ngl::Vec3(0.817f, 0.915f, 1.000f),
                                            ngl::Vec3(0.443f, 0.575f, 0.739f),
                                            ngl::Vec3(0.187f, 0.249f, 0.317f),
                                            ngl::Vec3(0.152f, 0.197f, 0.237f),
                                            ngl::Vec3(0.270f, 0.238f, 0.158f),
                                            ngl::Vec3(0.084f, 0.098f, 0.045f),
                                            ngl::Vec3(0.223f, 0.331f, 0.407f),
                                            ngl::Vec3(0.209f, 0.188f, 0.150f),
                                            ngl::Vec3(0.441f, 0.376f, 0.276f)};
};

#endif // MYSCENE_H
