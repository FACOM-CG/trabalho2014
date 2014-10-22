#ifndef __MeshSweeper_h
#define __MeshSweeper_h

//[]------------------------------------------------------------------------[]
//|                                                                          |
//|                          GVSG Graphics Classes                           |
//|                               Version 1.0                                |
//|                                                                          |
//|              Copyright® 2010-2014, Paulo Aristarco Pagliosa              |
//|              All Rights Reserved.                                        |
//|                                                                          |
//[]------------------------------------------------------------------------[]
//
//  OVERVIEW: MeshSweeper.h
//  ========
//  Class definition for mesh sweeper.

#include "Sweeper.h"
#include "TriangleMesh.h"

namespace Graphics
{ // begin namespace Graphics


//////////////////////////////////////////////////////////
//
// MeshSweeper: mesh sweeper class
// ===========
class MeshSweeper: public Sweeper
{
public:
  // Make box
  static TriangleMesh* makeBox(
    const vec3& center,
    const vec3& normal,
    const vec3& up,
    const vec3& size,
    const mat4& m = mat4::identity());

  static TriangleMesh* makeBox(
    const vec3& center,
    const vec3& orientation,
    const vec3& scale);

  // Make cube
  static TriangleMesh* makeCube(
    const vec3& center,
    const vec3& normal,
    const vec3& up,
    REAL size)
  {
    return makeBox(center, normal, up, vec3(size, size, size));
  }

  static TriangleMesh* makeCube(
    const vec3& center,
    const vec3& orientation,
    REAL scale)
  {
    return makeBox(center, orientation, vec3(scale, scale, scale));
  }

  static TriangleMesh* makeCube()
  {
    return makeCube(vec3::null(), vec3::null(), 1);
  }

  // Make sphere
  static TriangleMesh* makeSphere(
    const vec3& center,
    REAL radius,
    int meridians = 16);

  static TriangleMesh* makeSphere()
  {
    return makeSphere(vec3::null(), 1);
  }

  // Make cylinder
  static TriangleMesh* makeCylinder(
    const vec3& center,
    REAL radius,
    const vec3& normal,
    REAL height,
    int segments = 16);

private:
  // Make cylinder
  static TriangleMesh* makeCylinder(
    const Polyline& circle,
    const vec3& path,
    const mat4& m = mat4::identity());

}; // MeshSweeper

} // end namespace Graphics

#endif // __MeshSweeper_h
