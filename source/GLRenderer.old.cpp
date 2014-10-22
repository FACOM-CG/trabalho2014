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
//  OVERVIEW: GLRenderer.cpp
//  ========
//  Source file for GL renderer.

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef __GLRenderer_h
#include "GLRenderer.old.h"
#endif

using namespace Graphics;

//
// Auxiliary function
//
void
renderXZPlane()
{
  glDisable(GL_LIGHTING);
  glColor3f(0.1f, 0.2f, 0.3f);
  glScaled(1024, 0, 1024);
  glBegin(GL_QUADS);
  glNormal3f(0, 1, 0);
  glVertex3f(-1, 0, -1);
  glVertex3f(-1, 0, 1);
  glVertex3f(1, 0, 1);
  glVertex3f(1, 0, -1);
  glEnd();
  glColor3f(1.0f, 1.0f, 1.0f);
  glEnable(GL_LIGHTING);
}


//////////////////////////////////////////////////////////
//
// GLRenderer implementation
// ==========
GLRenderer::GLRenderer(Scene& scene, Camera* camera):
  Renderer(scene, camera),
  renderMode(Smooth)
{
  flags.set(useLights);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

void
GLRenderer::drawAABB(const BoundingBox& box) const
{
  Vec3 p1 = box.getP1();
  Vec3 p7 = box.getP2();
  Vec3 p2(p7.x, p1.y, p1.z);
  Vec3 p3(p7.x, p7.y, p1.z);
  Vec3 p4(p1.x, p7.y, p1.z);
  Vec3 p5(p1.x, p1.y, p7.z);
  Vec3 p6(p7.x, p1.y, p7.z);
  Vec3 p8(p1.x, p7.y, p7.z);

  drawLine(p1, p2);
  drawLine(p2, p3);
  drawLine(p3, p4);
  drawLine(p1, p4);
  drawLine(p5, p6);
  drawLine(p6, p7);
  drawLine(p7, p8);
  drawLine(p5, p8);
  drawLine(p3, p7);
  drawLine(p2, p6);
  drawLine(p4, p8);
  drawLine(p1, p5);
}

void
GLRenderer::render()
{
  startRender();
  if (renderMode == Wireframe)
    renderWireframe();
  else if (scene->getNumberOfLights() != 0)
    renderFaces();
  else
  {
    Light* light = makeDefaultLight();

    scene->addLight(light);
    renderFaces();
    scene->deleteLight(light);
  }
  endRender();
}

void
GLRenderer::setProjectionMatrix()
{
  REAL F;
  REAL B;

  updateView();
  camera->getClippingPlanes(F, B);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (camera->getProjectionType() == Camera::Parallel)
  {
    /* ------------------------------------------------------------------
     * void glOrtho(GLdouble left,
     *   GLdouble right,
     *   GLdouble bottom,
     *   GLdouble top,
     *   GLdouble near,
     *   GLdouble far);
     * ------------------------------------------------------------------
     * Creates a matrix for an orthographic parallel viewing volume and
     * multiplies the current matrix by it. (left, bottom, -near) and
     * (right, top, -near) are points on the near clipping plane that are
     * mapped to the lower-left and upper-right corners of the viewport
     * window, respectively. (left, bottom, -far) and (right, top, -far)
     * are points on the far clipping plane that are mapped to the same
     * respective corners of the viewport. Both near and far can be
     * positive or negative.
     */
    REAL h = camera->getHeight() / 2;
    REAL w = h * camera->getAspectRatio();

    glOrtho(-w, w, -h, h, -B, B);
  }
  else
    /* ------------------------------------------------------------------
     * void gluPerspective(GLdouble fovy,
     *   GLdouble aspect,
     *   GLdouble near,
     *   GLdouble far);
     * ------------------------------------------------------------------
     * Creates a matrix for a symmetric perspective-view frustum and
     * multiplies the current matrix by it. fovy is the angle of the
     * field of view in the x-z plane; its value must be in the range
     * [0.0,180.0]. aspect is the aspect ratio of the frustum, its width
     * divided by its height. near and far values the distances between
     * the viewpoint and the clipping planes, along the negative z-axis.
     * They should always be positive.
     */
    gluPerspective(camera->getViewAngle(), camera->getAspectRatio(), F, B);

  /* --------------------------------------------------------------------
   * void gluLookAt(GLdouble eyex,
   *   GLdouble eyey,
   *   GLdouble eyez,
   *   GLdouble centerx,
   *   GLdouble centery,
   *   GLdouble centerz,
   *   GLdouble upx,
   *   GLdouble upy,
   *   GLdouble upz);
   * --------------------------------------------------------------------
   * Defines a viewing matrix and multiplies it to the right of the
   * current The desired viewpoint is specified by eyex, eyey, and eyez.
   * The centerx, centery, and centerz arguments specify any point along
   * the desired line of sight, but typically they're some point in the
   * center of the scene being looked at. The upx, upy, and upz arguments
   * indicate which direction is up (that is, the direction from the
   * bottom to the top of the viewing volume).
   */
  Vec3 p = camera->getPosition();
  Vec3 f = camera->getFocalPoint();
  Vec3 u = camera->getViewUp();

  gluLookAt(p.x, p.y, p.z, f.x, f.y, f.z, u.x, u.y, u.z);
  /* --------------------------------------------------------------------
   * void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
   * --------------------------------------------------------------------
   * Defines a pixel rectangle in the window into which the final image
   * is mapped. The (x, y) parameter specifies the lower-left corner of
   * the viewport, and width and height are the size of the viewport
   * rectangle. By default, the initial viewport values are
   * (0, 0, winWidth, winHeight), where winWidth and winHeight are the
   * size of the window.
   */
  glViewport(0, 0, W, H);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

inline void
setLightProperty(long lid, long name, const Color& c)
{
  float v[4] = { (float)c.r, (float)c.g, (float)c.b, 1.0f };
  glLightfv(lid, name, v);
}

inline void
setLightGeometry(long lid, Light* light)
{
  float w = light->isDirectional() ? 0.0f : 1.0f;
  const Vec3& p = light->position;
  float v[4] = { (float)p.x, (float)p.y, (float)p.z, w };

  glLightfv(lid, GL_POSITION, v);
}

void
GLRenderer::renderLights()
{
  if (!flags.isSet(useLights) || scene->getNumberOfLights() == 0)
    return;

  LightIterator lit(scene->getLightIterator());

  for (int lid = GL_LIGHT0; lid <= GL_LIGHT7 && lit;)
  {
    Light* light = lit++;

    if (!light->isTurnedOn())
      continue;

    const Color& color = light->color;

    setLightGeometry(lid, light);
    setLightProperty(lid, GL_AMBIENT, color);
    setLightProperty(lid, GL_DIFFUSE, color);
    setLightProperty(lid, GL_SPECULAR, color);
    glEnable(lid++);
  }
  glEnable(GL_LIGHTING);
}

inline void
setMaterialProperty(long name, const Color& c)
{
  float v[4] = { (float)c.r, (float)c.g, (float)c.b, 1.0f };
  glMaterialfv(GL_FRONT, name, v);
}

void
renderMaterial(const Material& m)
{
  setMaterialProperty(GL_AMBIENT, m.surface.ambient);
  setMaterialProperty(GL_DIFFUSE, m.surface.diffuse);
  setMaterialProperty(GL_SPECULAR, m.surface.spot);
  glMaterialf(GL_FRONT, GL_SHININESS, (float)m.surface.shine);
}

void
GLRenderer::startRender()
{
  Color& bc = scene->backgroundColor;
  Color& al = scene->ambientLight;
  float c[] = { float(al.r), float(al.g), float(al.b), 1.0f };

  /* ---------------------------------------------------------------------
   * void glClearColor(GLclampf red, GLclampf green, GLclampf blue,
   *   GLclampf alpha);
   * ---------------------------------------------------------------------
   * Sets the current clearing color for use in clearing color buffers in
   * RGBA mode. The red, green, blue, and alpha values are clamped if
   * necessary to the range [0,1]. The default clearing color is
   * (0, 0, 0, 0), which is black.
   */
  glClearColor((float)bc.r, (float)bc.g, (float)bc.b, 1.0f);
  /* --------------------------------------------------------------------
   * void glClear(GLbitfield mask);
   * --------------------------------------------------------------------
   * Clears the specified buffers to their current clearing values. The
   * mask argument is a bitwise-ORed combination of the values
   * GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_ACCUM_BUFFER_BIT, and
   * GL_STENCIL_BUFFER_BIT.
   */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /* --------------------------------------------------------------------
   * void glLightModel{if}(GLenum pname, TYPE param);
   * void glLightModel{if}v(GLenum pname, TYPE* param);
   * --------------------------------------------------------------------
   * Sets properties of the lighting model. The characteristic of the
   * lighting model being set is defined by pname, which specifies a
   * named parameter. param indicates the values to which the pname
   * characteristic is set; it's a pointer to a group of values if the
   * vector version is used, or the value itself if the nonvector version
   * is used. The nonvector version can be used to set only single-valued
   * lighting model characteristics, not for GL_LIGHT_MODEL_AMBIENT.
   */
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, c);
  setProjectionMatrix();
}

void
GLRenderer::endRender()
{
  /* --------------------------------------------------------------------
   * void glFlush();
   * --------------------------------------------------------------------
   * Forces previously issued OpenGL commands to begin execution, thus
   * guaranteeing that they complete in finite time.
   */
  glFlush();
}

void
GLRenderer::renderWireframe()
{
  /* --------------------------------------------------------------------
   * void glPolygonMode(GLenum face, GLenum mode);
   * --------------------------------------------------------------------
   * Controls the drawing mode for a polygon's front and back faces. The
   * parameter face can be GL_FRONT_AND_BACK, GL_FRONT, or GL_BACK; mode
   * can be GL_POINT, GL_LINE, or GL_FILL to indicate whether the polygon
   * should be drawn as points, outlined, or filled. By default, both the
   * front and back faces are drawn filled.
   */
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  /* --------------------------------------------------------------------
   * void glColor3{b s i f d ub us ui}(TYPE r, TYPE g, TYPE b);
   * void glColor4{b s i f d ub us ui}(TYPE r, TYPE g, TYPE b, TYPE a);
   * void glColor3{b s i f d ub us ui}v(const TYPE* v);
   * void glColor4{b s i f d ub us ui}v(const TYPE* v);
   * --------------------------------------------------------------------
   * Sets the current red, green, blue, and alpha values. This command
   * can have up to three suffixes, which differentiate variations of the
   * parameters accepted. The first suffix is either 3 or 4, to indicate
   * whether you supply an alpha value in addition to the red, green, and
   * blue values. If you don't supply an alpha value, it's automatically
   * set to 1.0. The second suffix indicates the data type for parameters:
   * byte, short, integer, float, double, unsigned byte, unsigned short,
   * or unsigned integer. The third suffix is an optional v, which
   * indicates that the argument is a pointer to an array of values of
   * the given data type.
   */
  glColor3f(1.0f, 1.0f, 1.0f);
  for (ActorIterator ait(scene->getActorIterator()); ait; ++ait)
  {
    if (!ait.current()->isVisible())
      continue;

    const TriangleMesh* mesh = ait.current()->getModel()->triangleMesh();

    if (mesh == 0)
      continue;

    const TriangleMesh::Arrays& meshData = mesh->getData();
    Vec3* vertices = meshData.vertices;
    TriangleMesh::Triangle* triangles = meshData.triangles;

  /* --------------------------------------------------------------------
   * void glBegin(GLenum mode);
   * --------------------------------------------------------------------
   * Marks the beginning of a vertex-data list that describes a geometric
   * primitive. The type of primitive is indicated by mode, which can be
   * any of the values GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP,
   * GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_QUADS,
   * GL_QUAD_STRIP, or GL_POLYGON.
   */
    glBegin(GL_TRIANGLES);
    for (int i = 0, n = meshData.numberOfTriangles; i < n; i++)
      for (int d = 0; d < 3; d++)
      {
        const Vec3& p = vertices[triangles[i].v[d]];
        /* --------------------------------------------------------------
         * void glVertex{234}{sifd}[v](TYPE coords);
         * --------------------------------------------------------------
         * Specifies a vertex for use in describing a geometric object.
         * You can supply up to four coordinates (x, y, z, w) for a
         * particular vertex or as few as two (x, y) by selecting the
         * appropriate version of the command. If you use a version that
         * doesn't explicitly specify z or w, z is understood to be 0 and
         * w is understood to be 1. Calls to glVertex*() are only
         * effective between a glBegin() and glEnd() pair.
         */
        glVertex3f((float)p.x, (float)p.y, (float)p.z);
      }
    /* ------------------------------------------------------------------
     * void glEnd();
     * ------------------------------------------------------------------
     * Marks the end of a vertex-data list.
     */
    glEnd();
  }
}

void
GLRenderer::renderFaces()
{
  if (flags.isSet(drawSceneBoundingBox))
    drawAABB(scene->getBoundingBox());
  glPolygonMode(GL_FRONT, GL_FILL);
  /* --------------------------------------------------------------------
   * void glShadeModel(GLenum mode);
   * --------------------------------------------------------------------
   * Sets the shading model. The mode parameter can be either GL_SMOOTH
   * (the default) or GL_FLAT.
   */
  glShadeModel(renderMode == Flat ? GL_FLAT : GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  if (!flags.isSet(useVertexColors))
    renderLights();
  for (ActorIterator ait(scene->getActorIterator()); ait; ++ait)
  {
    if (!ait.current()->isVisible())
      continue;

    const TriangleMesh* mesh = ait.current()->getModel()->triangleMesh();

    if (mesh == 0)
      continue;

    const TriangleMesh::Arrays& meshData = mesh->getData();
    Vec3* vertices = meshData.vertices;
    TriangleMesh::Triangle* triangles = meshData.triangles;
    Vec3* normals = meshData.normals;
    Color* colors = flags.isSet(useVertexColors) ? meshData.colors : 0;

    for (int i = 0, n = meshData.numberOfTriangles; i < n; i++)
    {
      if (colors == 0)
        renderMaterial(*MaterialFactory::get(triangles[i].v[3]));
      glBegin(GL_TRIANGLES);
      for (int d = 0; d < 3; d++)
      {
        int vid = triangles[i].v[d];
        int nid = triangles[i].n[d];

        if (colors != 0)
        {
          const Color& c = colors[vid];
          glColor3f((float)c.r, (float)c.g, (float)c.b);
        }
        else if (nid > -1)
        {
          const Vec3& N = normals[nid];
          /* ------------------------------------------------------------
           * void glNormal3{bsidf}(TYPE nx, TYPE ny, TYPE nz);
           * void glNormal3{bsidf}v(const TYPE* v);
           * ------------------------------------------------------------
           * Sets the current normal vector as specified by the
           * arguments. The nonvector version (without the v) takes three
           * arguments, which specify an (nx, ny, nz) vector that's taken
           * to be the normal. Alternatively, you can use the vector
           * version of this function (with the v) and supply a single
           * array of three elements to specify the desired normal. The b,
           * s, and i versions scale their parameter values linearly to
           * the range [-1.0,1.0].
           */
          glNormal3f((float)N.x, (float)N.y, (float)N.z);
        }

        const Vec3& p = vertices[vid];
        glVertex3f((float)p.x, (float)p.y, (float)p.z);
      }
      glEnd();
    }
  }
  glDisable(GL_DEPTH_TEST);
  for (int lid = GL_LIGHT0; lid <= GL_LIGHT7; ++lid)
    glDisable(lid);
  glDisable(GL_LIGHTING);
}

void
GLRenderer::drawLine(const Vec3& p1, const Vec3& p2) const
{
  glColor3f(0.5f, 0.6f, 0.5f);
  glBegin(GL_LINES);
  glVertex3f((REAL)p1.x, (REAL)p1.y, (REAL)p1.z);
  glVertex3f((REAL)p2.x, (REAL)p2.y, (REAL)p2.z);
  glEnd();
}
