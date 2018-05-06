#include "sandiskscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>
#include <ngl/NGLStream.h>

EnvScene::EnvScene() : Scene() {}

void EnvScene::initGL() noexcept {
    ngl::NGLInit::instance();
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->loadShader("EnvironmentProgram",
                       "shaders/env_vert.glsl",
                       "shaders/env_frag.glsl");

    shader->loadShader("BeckmannProgram",
                       "shaders/beckmann_vert.glsl",
                       "shaders/beckmann_frag.glsl");

    shader->loadShader("ShadowProgram",
                       "shaders/shadow_vert.glsl",
                       "shaders/shadow_frag.glsl");

    m_environmentID = shader->getProgramID("EnvironmentProgram");
    m_beckmannID = shader->getProgramID("BeckmannProgram");
    m_shadowID = shader->getProgramID("ShadowProgram");

    shader->use("BeckmannProgram");
    initTexture(3, m_normalMapTex, "images/bump2.jpg");
    shader->setUniform("normalMap", 3);

    initEnvironment();
    m_roughness = 0.5f;

    m_usbMeshes[0].mesh = new ngl::Obj("models/blackPlastic.obj");
    m_usbMeshes[0].shaderID = m_beckmannID;

    m_usbMeshes[1].mesh = new ngl::Obj("models/bluePlastic.obj");
    m_usbMeshes[1].shaderID = m_beckmannID;

    m_usbMeshes[2].mesh = new ngl::Obj("models/gold.obj");
    m_usbMeshes[2].shaderID = m_beckmannID;

    m_usbMeshes[3].mesh = new ngl::Obj("models/metal.obj");
    m_usbMeshes[3].shaderID = m_beckmannID;

    m_usbMeshes[4].mesh = new ngl::Obj("models/translucentPlastic.obj");
    m_usbMeshes[4].shaderID = m_beckmannID;

    for (auto &i : m_usbMeshes)
    {
      i.mesh->createVAO();
    }
}

void EnvScene::paintGL() noexcept
{
  //Draw depth texture to FBO.

  //Draw to the screen.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();

  loadEnvironmentUniforms();
  prim->draw("cube");

  for (auto &i : m_usbMeshes)
  {
    loadMemoryStickUniforms(i);
    i.mesh->draw();
  }
}

void EnvScene::loadEnvironmentUniforms()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("EnvironmentProgram");

  glm::mat4 M, MV, MVP;
  glm::mat3 N;

  M = glm::scale(M, glm::vec3(50.f, 50.f, 50.f));
  MV = m_V2 * M;
  MVP = m_P * MV;
  N = glm::inverse(glm::mat3(MV));

  glUniformMatrix4fv(glGetUniformLocation(m_environmentID, "MVP"),
                     1,
                     false,
                     glm::value_ptr(MVP));
  glUniformMatrix4fv(glGetUniformLocation(m_environmentID, "MV"),
                     1,
                     false,
                     glm::value_ptr(MV));
  glUniformMatrix4fv(glGetUniformLocation(m_environmentID, "normalMatrix"),
                     1,
                     false,
                     glm::value_ptr(N));
  glUniformMatrix4fv(glGetUniformLocation(m_environmentID, "invV"),
                     1,
                     false,
                     glm::value_ptr(glm::inverse(m_V)));
}

void EnvScene::loadMemoryStickUniforms(USBmesh _mesh)
{
  glUseProgram(_mesh.shaderID);

  glm::mat3 N;
  glm::mat4 M, MV, MVP;

  M = glm::scale(M, glm::vec3(0.1f, 0.1f, 0.1f));
  MV = m_V * M;
  MVP = m_P * MV;
  N = glm::inverse(glm::mat3(MV));

  glUniformMatrix4fv(glGetUniformLocation(_mesh.shaderID, "MVP"),
                     1,
                     false,
                     glm::value_ptr(MVP));
  glUniformMatrix4fv(glGetUniformLocation(_mesh.shaderID, "MV"),
                     1,
                     false,
                     glm::value_ptr(MV));
  glUniformMatrix3fv(glGetUniformLocation(_mesh.shaderID, "normalMatrix"),
                     1,
                     true,
                     glm::value_ptr(N));
  glUniform3fv(glGetUniformLocation(_mesh.shaderID, "eyePos"),
               1,
               glm::value_ptr(m_eyePos));

  for(size_t i=0; i<m_lightPos.size(); i++)
  {
    glUniform3fv(glGetUniformLocation(_mesh.shaderID, ("lightPos[" + std::to_string(i) + "]").c_str()),
                 3,
                 glm::value_ptr(m_lightPos[i]));
    glUniform3fv(glGetUniformLocation(_mesh.shaderID, ("lightCol[" + std::to_string(i) + "]").c_str()),
                 3,
                 glm::value_ptr(m_lightCol[i]));
  }

  int envMapMaxLod = log2(1024);

  glUniform1i(glGetUniformLocation(_mesh.shaderID, "envMapMaxLod"),
              envMapMaxLod);

  glUniform1f(glGetUniformLocation(_mesh.shaderID, "roughness"),
               _mesh.roughness);

  glUniform1f(glGetUniformLocation(_mesh.shaderID, "metallic"),
               _mesh.metallic);

  glUniform1f(glGetUniformLocation(_mesh.shaderID, "diffAmount"),
              _mesh.diffAmount);

  glUniform1f(glGetUniformLocation(_mesh.shaderID, "specAmount"),
               _mesh.specAmount);

  glUniform3fv(glGetUniformLocation(_mesh.shaderID, "materialDiff"),
               1,
               glm::value_ptr(_mesh.materialDiffuse));

  glUniform3fv(glGetUniformLocation(_mesh.shaderID, "materialSpec"),
               1,
               glm::value_ptr(_mesh.materialSpecular));

  glUniform1f(glGetUniformLocation(_mesh.shaderID, "alpha"),
              _mesh.alpha);
}

void EnvScene::initTexture(const GLuint& texUnit, GLuint &texId, const char *filename)
{
    glActiveTexture(GL_TEXTURE0 + texUnit);

    ngl::Image img(filename);

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D (
                GL_TEXTURE_2D,    // The target (in this case, which side of the cube)
                0,                // Level of mipmap to load
                img.format(),     // Internal format (number of colour components)
                img.width(),      // Width in pixels
                img.height(),     // Height in pixels
                0,                // Border
                img.format(),     // Format of the pixel data
                GL_UNSIGNED_BYTE, // Data type of pixel data
                img.getPixels()); // Pointer to image data in memory

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void EnvScene::initEnvironment()
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_envTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envTex);

    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "images/nz.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "images/pz.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "images/ny.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "images/py.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "images/nx.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "images/px.png");

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLfloat anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->use("EnvironmentProgram");
    shader->setUniform("envMap", 0);
    shader->use("BeckmannProgram");
    shader->setUniform("envMap", 0);
}

void EnvScene::initEnvironmentSide(GLenum target, const char *filename)
{
    ngl::Image img(filename);

    glTexImage2D (
      target,           // The target (in this case, which side of the cube)
      0,                // Level of mipmap to load
      img.format(),     // Internal format (number of colour components)
      img.width(),      // Width in pixels
      img.height(),     // Height in pixels
      0,                // Border
      img.format(),     // Format of the pixel data
      GL_UNSIGNED_BYTE, // Data type of pixel data
      img.getPixels()   // Pointer to image data in memory
    );
}
