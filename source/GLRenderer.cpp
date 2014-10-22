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

#include "GLRenderer.h"

using namespace Graphics;

template <typename T>
inline GLsizeiptr
sizeOf(int n)
{
  return sizeof(T)* n;
}

const char* vertexShader =
  "#version 400\n"
  "layout (location = 0) in vec4 position;\n"
  "layout (location = 1) in vec3 normal;\n"
  "uniform mat4 vpMatrix;\n"
  "uniform mat4 modelMatrix;\n"
  "uniform vec4 Oa;\n"
  "uniform vec4 Od;\n"
  "uniform vec4 ambientLight = vec4(1, 1, 1, 1);\n"
  "uniform vec4 lightPosition = vec4(-5, 5, 10, 1);\n"
  "uniform vec4 lightColor = vec4(1, 1, 1, 1);\n"
  "out vec4 color;\n"
  "void main() {\n"
  "  vec4 P = modelMatrix * position;\n"
  "  gl_Position = vpMatrix * P;\n"
  "  vec3 N = normalize(mat3(modelMatrix) * normal);\n"
  "  vec4 L = normalize(P - lightPosition);\n"
  "  float cos_theta = -dot(N, vec3(L));\n"
  "  color = Oa * ambientLight;\n"
  "  if (cos_theta > 0)\n"
  "    color += Od * lightColor * cos_theta;\n"
  "}";

// The input variable "color" of the fragment shader is the
// (interpolated) output variable "color" of the vertex shader
// (note that the types and names have to be equal)
const char* fragmentShader =
  "#version 400\n"
  "in vec4 color;\n"
  "out vec4 fragmentColor;\n"
  "void main() {\n"
  "  fragmentColor = color;\n"
  "}";


//////////////////////////////////////////////////////////
//
// GLVertexArray implementation
// =============
inline
GLVertexArray::GLVertexArray(const TriangleMesh* mesh)
{
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(3, buffers);

  const TriangleMesh::Arrays& a = mesh->getData();

  if (GLsizeiptr s = sizeOf<vec3>(a.numberOfVertices))
  {
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, s, a.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
  }
  if (GLsizeiptr s = sizeOf<vec3>(a.numberOfNormals))
  {
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, s, a.normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
  }
  if (GLsizeiptr s = sizeOf<TriangleMesh::Triangle>(a.numberOfTriangles))
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, s, a.triangles, GL_STATIC_DRAW);
  }
  count = 3 * a.numberOfTriangles;
}

inline void
GLVertexArray::render()
{
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}

GLVertexArray::~GLVertexArray()
{
  glDeleteBuffers(3, buffers);
  glDeleteVertexArrays(1, &vao);
}


//////////////////////////////////////////////////////////
//
// GLRenderer implementation
// ==========
GLRenderer::GLRenderer(Scene& scene, Camera* camera):
  Renderer(scene, camera),
  renderMode(Smooth),
  program("renderer program")
{
  flags.set(UseLights);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  program.addShader(GL_VERTEX_SHADER, GLSL::STRING, vertexShader);
  program.addShader(GL_FRAGMENT_SHADER, GLSL::STRING, fragmentShader);
  program.use();
  vpMatrixLoc = program.getUniformLocation("vpMatrix");
  modelMatrixLoc = program.getUniformLocation("modelMatrix");
  OaLoc = program.getUniformLocation("Oa");
  OdLoc = program.getUniformLocation("Od");
  ambientLightLoc = program.getUniformLocation("ambientLight");
}

void
GLRenderer::drawAABB(const Bounds3& box) const
{
  vec3 p1 = box.getMin();
  vec3 p7 = box.getMax();
  vec3 p2(p7.x, p1.y, p1.z);
  vec3 p3(p7.x, p7.y, p1.z);
  vec3 p4(p1.x, p7.y, p1.z);
  vec3 p5(p1.x, p1.y, p7.z);
  vec3 p6(p7.x, p1.y, p7.z);
  vec3 p8(p1.x, p7.y, p7.z);

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

inline mat4
getVpMatrix(Camera* c)
{
  return c->getProjectionMatrix() * c->getWorldToCameraMatrix();
}

void
GLRenderer::update()
{
  Renderer::update();
  vpMatrix = getVpMatrix(camera);
  program.setUniform(vpMatrixLoc, vpMatrix);
  program.setUniform(ambientLightLoc, scene->ambientLight);
  glViewport(0, 0, W, H);
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
GLRenderer::startRender()
{
  update();

  const Color& bc = scene->backgroundColor;

  glClearColor((float)bc.r, (float)bc.g, (float)bc.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
GLRenderer::endRender()
{
  glFlush();
}

template <typename T>
inline T*
mapBuffer(GLenum target, GLenum access)
{
  return (T*)glMapBuffer(target, access);
}

inline void
unmapBuffer(GLenum target)
{
  glUnmapBuffer(target);
}

inline GLVertexArray*
getVertexArray(const TriangleMesh* mesh)
{
  return dynamic_cast<GLVertexArray*>((Object*)mesh->userData);
}

inline GLVertexArray*
vertexArray(TriangleMesh* mesh)
{
  GLVertexArray* vb = getVertexArray(mesh);

  if (vb == 0)
  {
    vb = new GLVertexArray(mesh);
    mesh->userData = vb;
  }
  return vb;
}

void
GLRenderer::drawMesh(const Model* model) const
{
  TriangleMesh* mesh = (TriangleMesh*)model->triangleMesh();

  if (mesh == 0)
    return;
  if (GLVertexArray* vb = vertexArray(mesh))
  {
    const Material* m = model->getMaterial();

    program.setUniform(modelMatrixLoc, model->getMatrix());
    program.setUniform(OaLoc, m->surface.ambient);
    program.setUniform(OdLoc, m->surface.diffuse);
    vb->render();
  }
}

void
GLRenderer::renderWireframe()
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (ActorIterator ait(scene->getActorIterator()); ait;)
  {
    Actor* a = ait++;

    if (!a->isVisible())
      continue;
    drawMesh(a->getModel());
    /*
    if (const TriangleMesh* mesh = a->getModel()->triangleMesh())
    {
      const TriangleMesh::Arrays& meshData = mesh->getData();
      vec3* vertices = meshData.vertices;
      TriangleMesh::Triangle* triangles = meshData.triangles;

      glBegin(GL_TRIANGLES);
      for (int i = 0, n = meshData.numberOfTriangles; i < n; i++)
        for (int d = 0; d < 3; d++)
        {
          const vec3& p = vertices[triangles[i].v[d]];
          glVertex3f((float)p.x, (float)p.y, (float)p.z);
        }
      glEnd();
    }
    */
  }
}

void
GLRenderer::renderFaces()
{
  if (flags.isSet(DrawSceneBounds))
    drawAABB(scene->boundingBox());
  glPolygonMode(GL_FRONT, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  for (ActorIterator ait(scene->getActorIterator()); ait;)
  {
    Actor* a = ait++;

    if (!a->isVisible())
      continue;
    drawMesh(a->getModel());
  }
  glDisable(GL_DEPTH_TEST);
}

void
GLRenderer::drawLine(const vec3& p1, const vec3& p2) const
{
  glColor3f(0.5f, 0.6f, 0.5f);
  glBegin(GL_LINES);
  glVertex3f((float)p1.x, (float)p1.y, (float)p1.z);
  glVertex3f((float)p2.x, (float)p2.y, (float)p2.z);
  glEnd();
}
