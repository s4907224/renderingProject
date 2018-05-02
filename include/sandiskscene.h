#ifndef SANDISKSCENE_H
#define SANDISKSCENE_H

#include <ngl/Obj.h>
#include "scene.h"

class EnvScene : public Scene
{
  public:
    /// @brief Empty ctor.
    EnvScene();
    /// @brief Initialisation method for GL setup and shaders.
    void initGL() noexcept;
    /// @brief GL Draw method.
    void paintGL() noexcept;
    /// @brief Currently public for debugging roughness by changing it in main.
    float m_roughness;
  private:
    /// @brief ID for the environment texture.
    GLuint m_envTex;
    /// @brief IDs for FBO.
    GLuint m_fboID, m_fboTextureID, m_fboDepthID;
    /// @brief Pointer to the OBJ.
    ngl::Obj *m_mesh;
    /// @brief Array of light positions.
    std::array<glm::vec3, 14> m_lightPos = {glm::vec3( 1.506f, 0.815f, 0.041f),
                                            glm::vec3( 0.079f, 0.609f,-1.026f),
                                            glm::vec3( 3.534f, 0.432f, 3.913f),
                                            glm::vec3( 1.254f, 0.453f, 4.827f),
                                            glm::vec3( 0.783f, 0.837f,-0.679f),
                                            glm::vec3( 0.783f, 0.239f,-0.679f),
                                            glm::vec3( 1.568f, 0.246f, 0.037f),
                                            glm::vec3(-0.069f, 0.031f,-1.023f),
                                            glm::vec3(-3.215f, 0.069f, 1.761f),
                                            glm::vec3(-1.340f, 0.217f,-1.599f),
                                            glm::vec3( 0.073f,-0.432f, 0.045f),
                                            glm::vec3( 0.419f,-0.106f, 0.675f),
                                            glm::vec3( 0.091f, 0.822f, 1.050f),
                                            glm::vec3(-0.231f,-0.938f, 1.825f)};
    /// @brief Array of light colours.
    std::array<glm::vec3, 14> m_lightCol = {glm::vec3(0.551f, 0.887f, 1.000f),
                                            glm::vec3(0.432f, 0.795f, 0.995f),
                                            glm::vec3(0.485f, 0.542f, 0.562f),
                                            glm::vec3(0.314f, 0.384f, 0.296f),
                                            glm::vec3(0.390f, 0.735f, 0.999f),
                                            glm::vec3(0.817f, 0.915f, 1.000f),
                                            glm::vec3(0.443f, 0.575f, 0.739f),
                                            glm::vec3(0.187f, 0.249f, 0.317f),
                                            glm::vec3(0.152f, 0.197f, 0.237f),
                                            glm::vec3(0.270f, 0.238f, 0.158f),
                                            glm::vec3(0.084f, 0.098f, 0.045f),
                                            glm::vec3(0.223f, 0.331f, 0.407f),
                                            glm::vec3(0.209f, 0.188f, 0.150f),
                                            glm::vec3(0.441f, 0.376f, 0.276f)};
    /// @brief Method to set the environment shader active and pass over the uniforms.
    void loadEnvironmentUniforms();
    /// @brief Method to set the beckmann shader active and pass over the uniforms.
    void loadMemoryStickUniforms();
    /// @brief Initialises the cubemap.
    void initEnvironment();
    /// @brief Intialises any given 2D texture.
    /// @param _texUnit the texture unit the texture should be stored in.
    /// @param _texID the texture ID.
    /// @param _filename the path to the image to be loaded.
    void initTexture(const GLuint &_texUnit, GLuint &_texID, const char *_filename);
    /// @brief Initialises a given side of a cubemap.
    /// @param _target the side of the cube that is being initialised
    /// @param _filename the path to the image to be loaded.
    void initEnvironmentSide(GLenum _target, const char* _filename);
};
#endif
