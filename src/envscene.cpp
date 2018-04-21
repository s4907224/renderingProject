#include "envscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>

EnvScene::EnvScene() : Scene() {}

void EnvScene::initGL() noexcept {
    // Fire up the NGL machinary (not doing this will make it crash)
    ngl::NGLInit::instance();
    // Set background colour
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);
    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);
    // Create and compile the vertex and fragment shader
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->loadShader("EnvironmentProgram",
                       "shaders/env_vert.glsl",
                       "shaders/env_frag.glsl");
    // Initialise our environment map here

    // Initialise our gloss texture map here

    shader->loadShader("BeckmannProgram",
                       "shaders/beckmann_vert.glsl",
                       "shaders/beckmann_frag.glsl");
    shader->use("BeckmannProgram");

    initTexture(3, m_glossMapTex, "images/bump.jpg");
    shader->setUniform("normalMap", 3);
    initEnvironment();
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createTrianglePlane("plane",2,2,1,1,ngl::Vec3(0,1,0));
    prim->createTorus("torus", 0.1f, 0.2f, 200, 200, false);
    prim->createSphere("sphere", 1.f, 30);
    m_roughness = 0.5f;

    m_mesh = new ngl::Obj("models/usbtri.obj");
    m_mesh->createVAO();
}

void EnvScene::paintGL() noexcept
{
  if (m_isFBODirty)
  {
    initFBO();
    m_isFBODirty = false;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["EnvironmentProgram"]->use();
  GLint envID = shader->getProgramID("EnvironmentProgram");
  glm::mat4 M, MV, MVP;
  glm::mat3 normalMatrix;

  M = glm::scale(M, glm::vec3(50.f, 50.f, 50.f));
  MV = m_V2 * M;
  MVP = m_P * MV;
  normalMatrix = glm::inverse(glm::mat3(MV));

  // Set this MVP on the GPU
  glUniformMatrix4fv(glGetUniformLocation(envID, "MVP"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MVP)); // a raw pointer to the data
  glUniformMatrix4fv(glGetUniformLocation(envID, "MV"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MV)); // a raw pointer to the data
  glUniformMatrix4fv(glGetUniformLocation(envID, "normalMatrix"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(normalMatrix)); // a raw pointer to the data
  glUniformMatrix4fv(glGetUniformLocation(envID, "invV"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(glm::inverse(m_V))); // a raw pointer to the data

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->draw("cube");

  glm::mat3 N;
  M = glm::mat4();
  //M = glm::scale(M, glm::vec3(1.8f, 1.f, 1.8f));
  //M = glm::scale(M, glm::vec3(0.1f, 0.1f, 0.1f));
  //M = glm::translate(M, glm::vec3(0.f, -5.f, 0.f));
  //M = glm::rotate(M, float(M_PI/2.f), glm::vec3(1.f, 0.f, 0.f));
  MV = m_V * M;
  MVP = m_P * MV;
  N = glm::inverse(glm::mat3(MV));

  //normalMatrix = glm::inverse(normalMatrix);
  (*shader)["BeckmannProgram"]->use();
  GLint beckmannID = shader->getProgramID("BeckmannProgram");
  // Set this MVP on the GPU
  glUniformMatrix4fv(glGetUniformLocation(beckmannID, "MVP"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MVP)); // a raw pointer to the data
  glUniformMatrix4fv(glGetUniformLocation(beckmannID, "MV"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(MV)); // a raw pointer to the data
  glUniformMatrix3fv(glGetUniformLocation(beckmannID, "normalMatrix"), //location of uniform
                     1, // how many matrices to transfer
                     true, // whether to transpose matrix
                     glm::value_ptr(N)); // a raw pointer to the data
  glUniformMatrix4fv(glGetUniformLocation(envID, "invV"), //location of uniform
                     1, // how many matrices to transfer
                     false, // whether to transpose matrix
                     glm::value_ptr(glm::inverse(m_V))); // a raw pointer to the data
  shader->setUniform("roughness", m_roughness);

  prim->draw("teapot");
  //m_mesh->draw();
}

void EnvScene::initTexture(const GLuint& texUnit, GLuint &texId, const char *filename) {
    // Set our active texture unit
    glActiveTexture(GL_TEXTURE0 + texUnit);

    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Create storage for our new texture
    glGenTextures(1, &texId);

    // Bind the current texture
    glBindTexture(GL_TEXTURE_2D, texId);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
                GL_TEXTURE_2D,    // The target (in this case, which side of the cube)
                0,                // Level of mipmap to load
                img.format(),     // Internal format (number of colour components)
                img.width(),      // Width in pixels
                img.height(),     // Height in pixels
                0,                // Border
                GL_RGB,          // Format of the pixel data
                GL_UNSIGNED_BYTE, // Data type of pixel data
                img.getPixels()); // Pointer to image data in memory

    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/**
 * @brief Scene::initEnvironment in texture unit 0
 */
void EnvScene::initEnvironment() {
    // Enable seamless cube mapping
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Placing our environment map texture in texture unit 0
    glActiveTexture (GL_TEXTURE0);

    // Generate storage and a reference for our environment map texture
    glGenTextures (1, &m_envTex);

    // Bind this texture to the active texture unit
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envTex);

    // Now load up the sides of the cube
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "images/nz.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "images/pz.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "images/ny.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "images/py.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "images/nx.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "images/px.png");

    // Generate mipmap levels
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Set the texture parameters for the cube map
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLfloat anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

    // Set our cube map texture to on the shader so we can use it
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->use("EnvironmentProgram");
    shader->setUniform("envMap", 0);
    shader->use("BeckmannProgram");
    shader->setUniform("envMap", 0);
}

/**
 * @brief Scene::initEnvironmentSide
 * @param texture
 * @param target
 * @param filename
 * This function should only be called when we have the environment texture bound already
 * copy image data into 'target' side of cube map
 */
void EnvScene::initEnvironmentSide(GLenum target, const char *filename) {
    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
      target,           // The target (in this case, which side of the cube)
      0,                // Level of mipmap to load
      img.format(),     // Internal format (number of colour components)
      img.width(),      // Width in pixels
      img.height(),     // Height in pixels
      0,                // Border
      GL_RGBA,          // Format of the pixel data
      GL_UNSIGNED_BYTE, // Data type of pixel data
      img.getPixels()   // Pointer to image data in memory
    );
}

void EnvScene::initFBO() {
    // First delete the FBO if it has been created previously
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE) {
        glDeleteTextures(1, &m_fboTextureID);
        glDeleteTextures(1, &m_fboDepthID);
        glDeleteFramebuffers(1, &m_fboID);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Generate a texture to write the FBO result to
    glGenTextures(1, &m_fboTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_fboTextureID);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 m_width,
                 m_height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // The depth buffer is rendered to a texture buffer too
    glGenTextures(1, &m_fboDepthID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fboDepthID);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 m_width,
                 m_height,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Create the frame buffer
    glGenFramebuffers(1, &m_fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTextureID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fboDepthID, 0);
    // Set the fragment shader output targets (DEPTH_ATTACHMENT is done automatically)
    GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBufs);
    // Check it is ready to rock and roll
    CheckFrameBuffer();
    // Unbind the framebuffer to revert to default render pipeline
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
