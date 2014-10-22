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
  virtual void setMatrix(const mat4&) = 0;

  void setTRS(const vec3& p, const quat& q, const vec3& s)
  {
    setMatrix(mat4::TRS(p, q, s));
  }

  void setTRS(const vec3& p, const vec3& a, const vec3& s)
  {
    setMatrix(mat4::TRS(p, a, s));
  }

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

  void setMatrix(const mat4& m)
  {
    matrix = m;
  }

  void setMaterial(Material* m)
  {
    material = m == 0 ? Material::getDefault() : m;
  }

protected:
  mat4 matrix;
  ObjectPtr<Material> material;

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
