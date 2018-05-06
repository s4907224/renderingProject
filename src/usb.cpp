#include "usb.h"

USBmesh::USBmesh(float _roughness,
                 float _metallic,
                 float _diffAmount,
                 float _specAmount,
                 glm::vec3 _materialDiffuse,
                 glm::vec3 _materialSpecular,
                 float _alpha) :
                 roughness (_roughness),
                 metallic (_metallic),
                 diffAmount (_diffAmount),
                 specAmount (_specAmount),
                 materialDiffuse (_materialDiffuse),
                 materialSpecular (_materialSpecular),
                 alpha (_alpha)
{}
