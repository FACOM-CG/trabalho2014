#ifndef __GLRenderer_h
#define __GLRenderer_h

//[]------------------------------------------------------------------------[]
//|                                                                          |
//|                          GVSG Graphics Library                           |
//|                               Version 1.0                                |
//|                                                                          |
//|              Copyright® 2007-2014, Paulo Aristarco Pagliosa              |
//|              All Rights Reserved.                                        |
//|                                                                          |
//[]------------------------------------------------------------------------[]
//
//  OVERVIEW: GLRenderer.h
//  ========
//  Class definition for GL renderer.

#include "GLPainter.h"
#include "Renderer.h"
#include "TriangleMeshShape.h"

namespace Graphics
{ // begin namespace Graphics


//////////////////////////////////////////////////////////
//
// GLVertexArray: GL vertex array class
// =============
class GLVertexArray : public Object
{
public:
  // Contructor
  GLVertexArray(const TriangleMesh*);

  // Destructor
  ~GLVertexArray();

  // Render triangles
  void render();

private:
  GLuint vao;
  GLuint buffers[4];
  GLsizei count;

}; // GLVertexArray


//////////////////////////////////////////////////////////
//
// GLRenderer: GL renderer class
// ==========
class GLRenderer: public Renderer, public GLPainter
{
public:
  enum RenderMode
  {
    Wireframe = 1,
    HiddenLines = 2,
    Flat = 4,
    Smooth = 0
  };

  // Flags
  enum
  {
    UseLights = 1,
    DrawSceneBounds = 2,
    DrawActorBounds = 4,
    DrawAxes = 8,
    DrawNormals = 16,
    UseVertexColors = 32
  };

  RenderMode renderMode;
  Flags flags;

  // Constructor
  GLRenderer(Scene&, Camera* = 0);

  void update();
  void render();

  void drawLine(const vec3&, const vec3&);
  void drawBoundingBox(const Bounds3&);
  void drawVector(const vec3&, const vec3&, REAL);

  void drawCone(const mat4& m = mat4::identity())
  {
    drawMesh(cone, m);
  }

  void drawCube(const mat4& m = mat4::identity())
  {
    drawMesh(cube, m);
  }

protected:
  virtual void startRender();
  virtual void endRender();
  virtual void renderWireframe();
  virtual void renderFaces();

  void drawMesh(const Model*);
  void drawAxes(const vec3&, const mat3&, REAL = 1);

private:
  GLSL::Program program;
  mat4f vpMatrix;
  GLint vpMatrixLoc;
  GLint modelMatrixLoc;
  GLint ambientLightLoc;
  GLint OaLoc;
  GLint OdLoc;

  void drawMesh(TriangleMesh*, const mat4&);
  void drawNormals(TriangleMesh*, const mat4&);
  void drawGround(REAL, REAL);

  static ObjectPtr<TriangleMesh> cone;
  static ObjectPtr<TriangleMesh> cube;

  static void initMeshes();

}; // GLRenderer

} // end namespace Graphics

#endif // __GLRenderer_h
