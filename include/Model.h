#ifndef __Model_h
#define __Model_h

//[]------------------------------------------------------------------------[]
//|                                                                          |
//|                          GVSG Graphics Library                           |
//|                               Version 1.0                                |
//|                                                                          |
//|              Copyright® 2010-2014, Paulo Aristarco Pagliosa              |
//|              All Rights Reserved.                                        |
//|                                                                          |
//[]------------------------------------------------------------------------[]
//
//  OVERVIEW: Model.h
//  ========
//  Class definition for generic model.

#include "Geometry/Bounds3.h"
#include "Material.h"

using namespace Ds;
using namespace Geometry;

namespace Graphics
{ // begin namespace Graphics

class TriangleMesh;


//////////////////////////////////////////////////////////
//
// Model: generic simple model class
// =====
class Model: public Object
{
public:
  // Destructor
  virtual ~Model()
  {
    // do nothing
  }

  virtual const Material* getMaterial() const = 0;
  virtual Bounds3 boundingBox() const = 0;
  virtual const TriangleMesh* triangleMesh() const = 0;
  virtual mat4 getMatrix() const = 0;

  virtual void setMaterial(Material*) = 0;
  virtual void setMatrix(const vec3&, const quat&, const vec3&) = 0;

}; // Model


//////////////////////////////////////////////////////////
//
// Primitive: generic primitive model class
// =========
class Primitive: public Model
{
public:
  const Material* getMaterial() const
  {
    return material;
  }

  mat4 getMatrix() const
  {
    return matrix;
  }

  void setMaterial(Material* m)
  {
    material = m == 0 ? Material::getDefault() : m;
  }

  void setMatrix(const vec3& p, const quat& q, const vec3& s)
  {
    matrix = mat4::TRS(p, q, s);
  }

protected:
  ObjectPtr<Material> material;
  mat4 matrix;

  // Protected constructors
  Primitive():
    matrix(mat4::identity()),
    material(Material::getDefault())
  {
    // do nothing
  }

  Primitive(const Primitive& primitive):
    material(primitive.material)
  {
    // do nothing
  }

}; // Primitive

} // end namespace Graphics

#endif // __Model_h
