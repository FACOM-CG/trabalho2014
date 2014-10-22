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

#include "GLProgram.h"
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

  void render();

  // Destructor
  ~GLVertexArray();

private:
  GLuint vao;
  GLuint buffers[4];
  GLsizei count;

}; // GLVertexArray


//////////////////////////////////////////////////////////
//
// GLRenderer: GL renderer class
// ==========
class GLRenderer: public Renderer
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
    UseVertexColors = 4
  };

  RenderMode renderMode;
  Flags flags;

  // Constructor
  GLRenderer(Scene&, Camera* = 0);

  void update();
  void render();

protected:
  virtual void startRender();
  virtual void endRender();
  virtual void renderWireframe();
  virtual void renderFaces();

  virtual void drawLine(const vec3&, const vec3&) const;
  virtual void drawAABB(const Bounds3&) const;

  // TODO
  void drawMesh(const Model*) const;

private:
  mat4 vpMatrix;
  GLSL::Program program;
  GLint vpMatrixLoc;
  GLint modelMatrixLoc;
  GLint ambientLightLoc;
  GLint OaLoc;
  GLint OdLoc;

}; // GLRenderer

} // end namespace Graphics

#endif // __GLRenderer_h
