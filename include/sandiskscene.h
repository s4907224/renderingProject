#ifndef SANDISKSCENE_H
#define SANDISKSCENE_H

#include <ngl/Obj.h>
#include "scene.h"
#include "usb.h"

class EnvScene : public Scene
{
  public:
    /// @brief Empty ctor.
    EnvScene();
    /// @brief Initialisation method for GL setup and shaders.
    void initGL() noexcept;
    /// @brief Called to resize both the window and set the FBO to be reinitialised.
    void resizeGL(GLint width, GLint height) noexcept;
    /// @brief GL Draw method.
    void paintGL() noexcept;
    /// @brief Currently public for debugging roughness by changing it in main.
    float m_roughness;
  private:
    /// @brief ID for the environment texture.
    GLuint m_envTex, m_metalDispMap, m_logoMap, m_textMap;
    /// @brief IDs for FBO.
    GLuint m_fbo1ID, m_fbo1TexID, m_fbo1DepthTexID, m_fbo2ID, m_fbo2TexID, m_fbo2DepthTexID;
    /// @brief Returns true when window has been resized.
    bool m_fboIsDirty;
    /// @brief IDs for shaders.
    GLint m_environmentID, m_beckmannID, m_brushedMetalID, m_translucentPlasticID, m_dofID, m_fakeShadowID;
    /// @brief std array containing the 5 meshes that make up the object.
    std::array<USBmesh, 6> m_usbMeshes = {
                                          //Black Plastic
                                          USBmesh(
                                                  0.7f, //Roughness
                                                  0.4f, //Metallic
                                                  0.5f, //Diffuse Intensity
                                                  0.1f, //Spec Intensity
                                                  glm::vec3(0.1f, 0.1f, 0.1f), //Diffuse Colour
                                                  glm::vec3(1.f, 1.f, 1.f), //Spec Colour
                                                  1.f //Alpha
                                                  ),

                                          //Blue Plastic
                                          USBmesh(
                                                  0.7f,
                                                  0.1f,
                                                  0.4f,
                                                  0.2f,
                                                  glm::vec3(0.f, 0.25f, 0.61f),
                                                  glm::vec3(1.f, 1.f, 1.f),
                                                  1.f
                                                  ),

                                          //Gold
                                          USBmesh(
                                                  0.6f,
                                                  1.f,
                                                  0.5f,
                                                  0.1f,
                                                  glm::vec3(),
                                                  glm::vec3(1.f, 0.81f, 0.29f),
                                                  1.f
                                                  ),

                                          //Metal
                                          USBmesh(
                                                  0.6f,
                                                  0.7f,
                                                  0.5f,
                                                  0.4f,
                                                  glm::vec3(0.2f, 0.2f, 0.2f),
                                                  glm::vec3(0.4f, 0.4f, 0.4f),
                                                  1.f
                                                  ),

                                          //Translucent Plastic
                                          USBmesh(
                                                  0.1f,
                                                  0.1f,
                                                  0.1f,
                                                  0.4f,
                                                  glm::vec3(0.7f, 0.8f, 0.75f),
                                                  glm::vec3(0.6f, 0.8f, 0.8f),
                                                  0.85f
                                                  ),

                                          //Faceplate
                                          USBmesh(
                                                  1.0f,
                                                  0.1f,
                                                  1.0f,
                                                  0.0f,
                                                  glm::vec3(),
                                                  glm::vec3(),
                                                  0.5f
                                                  )
                                         };
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

    /// @brief Takes the rendered image and applies a depth of field effect.
    void dof();
    /// @brief Sets up GL to render a depth texture from the perspective of the main light.
    void loadShadowUniforms();
    /// @brief Method to set the environment shader active and pass over the uniforms.
    void loadEnvironmentUniforms();
    /// @brief Method to set the beckmann shader active and pass over the uniforms.
    void loadMemoryStickUniforms(USBmesh _mesh);
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
    /// @brief Initialises the Framebuffer for Dof
    void initDepthFBO();
    /// @brief Initialises the Framebuffer for shadow mapping.
    void initShadowFBO();
};
#endif
