#ifndef USB_H
#define USB_H

#include <glm/glm.hpp>
#include <ngl/Obj.h>
#include <string>

struct USBmesh
{
  USBmesh(float _roughness,
          float _metallic,
          float _diffAmount,
          float _specAmount,
          glm::vec3 _materialDiffuse,
          glm::vec3 _materialSpecular,
          float _alpha);

  ngl::Obj *mesh;
  float roughness;
  float metallic;
  float diffAmount;
  float specAmount;
  glm::vec3 materialDiffuse;
  glm::vec3 materialSpecular;
  float alpha;
  GLuint shaderID;
};
#endif
