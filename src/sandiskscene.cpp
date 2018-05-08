#include "sandiskscene.h"

#include <glm/gtc/type_ptr.hpp>
#include <ngl/Obj.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Image.h>
#include <ngl/NGLStream.h>

EnvScene::EnvScene() : Scene() {}

void EnvScene::initGL() noexcept
{
  ngl::NGLInit::instance();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_ARB_multisample);

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->loadShader("EnvironmentProgram",
                     "shaders/env_vert.glsl",
                     "shaders/env_frag.glsl");

  shader->loadShader("BeckmannProgram",
                     "shaders/beckmann_vert.glsl",
                     "shaders/beckmann_frag.glsl");

  shader->loadShader("BrushedMetalProgram",
                     "shaders/brushedMetal_vert.glsl",
                     "shaders/brushedMetal_frag.glsl");

  shader->loadShader("TranslucentPlasticProgram",
                     "shaders/translucentPlastic_vert.glsl",
                     "shaders/translucentPlastic_frag.glsl");

  shader->loadShader("DOFProgram",
                     "shaders/dof_vert.glsl",
                     "shaders/dof_frag.glsl");

  shader->loadShader("FakeShadowProgram",
                     "shaders/fakeShadow_vert.glsl",
                     "shaders/fakeShadow_frag.glsl");

  m_environmentID = shader->getProgramID("EnvironmentProgram");
  m_beckmannID = shader->getProgramID("BeckmannProgram");
  m_brushedMetalID = shader->getProgramID("BrushedMetalProgram");
  m_translucentPlasticID = shader->getProgramID("TranslucentPlasticProgram");
  m_dofID = shader->getProgramID("DOFProgram");
  m_fakeShadowID = shader->getProgramID("FakeShadowProgram");

  shader->use("BrushedMetalProgram");
  initTexture(1, m_logoMap, "images/sdlogo.png");
  shader->setUniform("logoMap", 1);

  shader->use("BrushedMetalProgram");
  initTexture(2, m_metalDispMap, "images/test.png");
  shader->setUniform("dispMap", 2);

  shader->use("TranslucentPlasticProgram");
  initTexture(3, m_textMap, "images/underText.png");
  shader->setUniform("textMap", 3);

  initEnvironment();
  m_roughness = 0.5f;

  m_usbMeshes[0].mesh = new ngl::Obj("models/blackPlastic.obj");
  m_usbMeshes[0].shaderID = m_beckmannID;

  m_usbMeshes[1].mesh = new ngl::Obj("models/bluePlastic.obj");
  m_usbMeshes[1].shaderID = m_beckmannID;

  m_usbMeshes[2].mesh = new ngl::Obj("models/gold.obj");
  m_usbMeshes[2].shaderID = m_beckmannID;

  m_usbMeshes[3].mesh = new ngl::Obj("models/metal.obj");
  m_usbMeshes[3].shaderID = m_brushedMetalID;

  m_usbMeshes[4].mesh = new ngl::Obj("models/translucentPlastic.obj");
  m_usbMeshes[4].shaderID = m_translucentPlasticID;

  m_usbMeshes[5].mesh = new ngl::Obj("models/faceplate.obj");
  m_usbMeshes[5].shaderID = m_fakeShadowID;

  for (auto &i : m_usbMeshes)
  {
    i.mesh->createVAO();
  }

  ngl::VAOPrimitives *prim = ngl::VAOPrimitives::instance();
  prim->createTrianglePlane("plane", 2.f, 2.f, 1, 1, ngl::Vec3::up());

  initShadowFBO();
}

void EnvScene::resizeGL(GLint width, GLint height) noexcept
{
  Scene::resizeGL(width, height);
  m_fboIsDirty = true;
}

void EnvScene::paintGL() noexcept
{
  //Render to FBO texture first
  if (m_fboIsDirty)
  {
    initDepthFBO();
    m_fboIsDirty = false;
  }

  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo1ID);
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

  dof();
}

void EnvScene::dof()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, m_fbo1TexID);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, m_fbo1TexID);

  glUseProgram(m_dofID);
  glUniform1i(glGetUniformLocation(m_dofID, "fboTex"), 4);
  glUniform1i(glGetUniformLocation(m_dofID, "fboDepthTex"), 6);
  glUniform1f(glGetUniformLocation(m_dofID, "focalDistance"), 2.f);
  glUniform1f(glGetUniformLocation(m_dofID, "blurRadius"), 0.01f);

  glm::mat4 planeMVP = glm::rotate(glm::mat4(1.f), -glm::pi<float>() * 0.5f, glm::vec3(1.f, 0.f, 0.f));
  glUniformMatrix4fv(glGetUniformLocation(m_dofID, "MVP"),
                     1,
                     false,
                     glm::value_ptr(planeMVP));

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->draw("plane");
  glBindTexture(GL_TEXTURE_2D, 0);
}

void EnvScene::loadEnvironmentUniforms()
{
  glUseProgram(m_environmentID);

  glm::mat4 M, MV, MVP;
  glm::mat3 N;

  M = glm::scale(M, glm::vec3(20.f, 20.f, 20.f));
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

  if (_mesh.shaderID == m_fakeShadowID)
  {
    glUniform3fv(glGetUniformLocation(m_fakeShadowID, "lightPos"),
                 1,
                 glm::value_ptr(m_lightPos[0]));
    return;
  }

  glUniform3fv(glGetUniformLocation(_mesh.shaderID, "lightCol"),
               m_lightCol.size(),
               glm::value_ptr(m_lightCol[0]));

  glUniform3fv(glGetUniformLocation(_mesh.shaderID, "lightPos"),
               m_lightCol.size(),
               glm::value_ptr(m_lightPos[0]));


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

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

void EnvScene::initDepthFBO()
{
   glBindFramebuffer(GL_FRAMEBUFFER, m_fbo1ID);
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == (GL_FRAMEBUFFER_COMPLETE))
   {
     glDeleteTextures(1, &m_fbo1TexID);
     glDeleteTextures(1, &m_fbo1DepthTexID);
     glDeleteFramebuffers(1, &m_fbo1ID);
   }

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   glGenTextures(1, &m_fbo1TexID);
   glActiveTexture(GL_TEXTURE4);
   glBindTexture(GL_TEXTURE_2D, m_fbo1TexID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glGenTextures(1, &m_fbo1DepthTexID);
   glActiveTexture(GL_TEXTURE6);
   glBindTexture(GL_TEXTURE_2D, m_fbo1DepthTexID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenFramebuffers(1, &m_fbo1ID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo1ID);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo1TexID, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fbo1DepthTexID, 0);

  GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, drawBufs);

  CheckFrameBuffer();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EnvScene::initShadowFBO()
{
   glBindFramebuffer(GL_FRAMEBUFFER, m_fbo2ID);
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == (GL_FRAMEBUFFER_COMPLETE))
   {
     glDeleteTextures(1, &m_fbo2DepthTexID);
     glDeleteFramebuffers(1, &m_fbo2ID);
   }

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   glGenTextures(1, &m_fbo2DepthTexID);
   glActiveTexture(GL_TEXTURE8);
   glBindTexture(GL_TEXTURE_2D, m_fbo2DepthTexID);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenFramebuffers(1, &m_fbo2ID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo2ID);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fbo2DepthTexID, 0);

  GLenum drawBufs[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, drawBufs);

  CheckFrameBuffer();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
