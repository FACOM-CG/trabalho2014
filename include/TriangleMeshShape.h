#ifndef __TriangleMeshShape_h
#define __TriangleMeshShape_h

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
//  OVERVIEW: TriangleMeshShape.h
//  ========
//  Class definition for triangle mesh shape.

#include "Model.h"
#include "TriangleMesh.h"

namespace Graphics
{ // begin namespace Graphics


//////////////////////////////////////////////////////////
//
// TriangleMeshShape: simple triangle mesh class
// =================
class TriangleMeshShape: public Primitive
{
public:
  // Constructor
  TriangleMeshShape(TriangleMesh* aMesh):
    mesh(aMesh)
  {
    if (aMesh != 0)
      bounds = aMesh->boundingBox();
  }

  Object* clone() const;
  Bounds3 boundingBox() const;
  const TriangleMesh* triangleMesh() const;

private:
  ObjectPtr<TriangleMesh> mesh;
  Bounds3 bounds;

}; // TriangleMeshShape

} // end namespace Graphics

#endif // __TriangleMeshShape_h
