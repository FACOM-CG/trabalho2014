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
#include "MeshSweeper.h"

using namespace Graphics;

template <typename T>
inline GLsizeiptr
sizeOf(int n)
{
  return sizeof(T) * n;
}

static const char* vertexShader =
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
static const char* fragmentShader =
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
ObjectPtr<TriangleMesh> GLRenderer::cone;
ObjectPtr<TriangleMesh> GLRenderer::cube;

void
GLRenderer::initMeshes()
{
  if (cone == 0)
    cone = MeshSweeper::makeCone();
  if (cube == 0)
    cube = MeshSweeper::makeCube();
}

GLRenderer::GLRenderer(Scene& scene, Camera* camera):
  Renderer(scene, camera),
  renderMode(Smooth),
  program("renderer program")
{
  flags.set(UseLights);
  program.addShader(GL_VERTEX_SHADER, GLSL::STRING, vertexShader);
  program.addShader(GL_FRAGMENT_SHADER, GLSL::STRING, fragmentShader);
  program.use();
  vpMatrixLoc = program.getUniformLocation("vpMatrix");
  modelMatrixLoc = program.getUniformLocation("modelMatrix");
  OaLoc = program.getUniformLocation("Oa");
  OdLoc = program.getUniformLocation("Od");
  ambientLightLoc = program.getUniformLocation("ambientLight");
  initMeshes();
}

void
GLRenderer::drawBoundingBox(const Bounds3& box)
{
  vec3 p1 = box.getMin();
  vec3 p7 = box.getMax();
  vec3 p2(p7.x, p1.y, p1.z);
  vec3 p3(p7.x, p7.y, p1.z);
  vec3 p4(p1.x, p7.y, p1.z);
  vec3 p5(p1.x, p1.y, p7.z);
  vec3 p6(p7.x, p1.y, p7.z);
  vec3 p8(p1.x, p7.y, p7.z);

  setLineColor(Color(0.2f, 0.2f, 0.2f));
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

  REAL s = camera->windowHeight();

  s = dMin<REAL>(s, s * camera->getAspectRatio());
  drawGround(s, s / 20);
  program.use();
  program.setUniform(vpMatrixLoc, vpMatrix);
  program.setUniform(ambientLightLoc, scene->ambientLight);
}

void
GLRenderer::endRender()
{
  if (flags.isSet(DrawSceneBounds))
    drawBoundingBox(scene->boundingBox());
  glFlush();
  program.disuse();
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
GLRenderer::drawMesh(const Model* model)
{
  TriangleMesh* mesh = (TriangleMesh*)model->triangleMesh();

  if (mesh == 0)
    return;
  if (GLVertexArray* vb = vertexArray(mesh))
  {
    if (flags.isSet(DrawActorBounds))
      drawBoundingBox(model->boundingBox());

    const Material* m = model->getMaterial();
    mat4 t = model->getMatrix();

    program.setUniform(modelMatrixLoc, t);
    program.setUniform(OaLoc, m->surface.ambient);
    program.setUniform(OdLoc, m->surface.diffuse);
    vb->render();
    if (flags.isSet(DrawNormals))
      drawNormals(mesh, t);
    if (flags.isSet(DrawAxes))
    {
      mat3 r(t);

      r[0].normalize();
      r[1].normalize();
      r[2].normalize();
      drawAxes(vec3(t[3]), r);
    }
  }
}

void
GLRenderer::renderWireframe()
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (ActorIterator ait(scene->getActorIterator()); ait;)
  {
    const Actor* a = ait++;

    if (a->isVisible())
      drawMesh(a->getModel());
  }
}

void
GLRenderer::renderFaces()
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  for (ActorIterator ait(scene->getActorIterator()); ait;)
  {
    const Actor* a = ait++;

    if (a->isVisible())
      drawMesh(a->getModel());
  }
  glDisable(GL_DEPTH_TEST);
}

void
GLRenderer::drawLine(const vec3& p1, const vec3& p2)
{
  vec4 points[2];
  
  points[0] = vpMatrix.transform(vec4(p1, 1));
  points[1] = vpMatrix.transform(vec4(p2, 1));
  GLPainter::drawLine(points);
}

void
GLRenderer::drawMesh(TriangleMesh* mesh, const mat4& m)
{
  if (GLVertexArray* vb = vertexArray(mesh))
  {
    using namespace GLSL;

    Program* current = Program::getCurrent();

    program.use();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    program.setUniform(modelMatrixLoc, m);
    program.setUniform(OaLoc, getLineColor());
    program.setUniform(OdLoc, getLineColor());
    vb->render();
    renderMode == Wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : 0;
    Program::setCurrent(current);
  }
}

void
GLRenderer::drawNormals(TriangleMesh* mesh, const mat4& m)
{
  const TriangleMesh::Arrays& a = mesh->getData();

  if (a.normals == 0)
    return;
  setLineColor(Color::white);
  for (int i = 0; i < a.numberOfVertices; i++)
  {
    const vec3 p = m.transform3x4(a.vertices[i]);
    const vec3 N = m.transformVector(a.normals[i]).versor();

    drawVector(p, N, 0.5);
  }
}

void
GLRenderer::drawVector(const vec3& p, const vec3& d, REAL s)
{
  vec3 a;

  if (Math::isZero(d.x) && Math::isZero(d.z))
    a = d.y < 0 ? vec3(0, 0, 1) : vec3::up();
  else
    a.set(d.x, d.y + 1, d.z);

  const vec3 end = p + d * s;
  const mat4 m = mat4::TRS(end, quat(180, a), vec3(0.1f, 0.4f, 0.1f));

  drawLine(p, end);
  drawCone(m);
}

inline Color
royalBlue()
{
  return Color(65, 105, 255);
}

void
GLRenderer::drawAxes(const vec3& p, const mat3& r, REAL s)
{
  GLboolean dt = glIsEnabled(GL_DEPTH_TEST);

  glDisable(GL_DEPTH_TEST);
  setLineColor(Color::red);
  drawVector(p, r[0], s);
  setLineColor(Color::green);
  drawVector(p, r[1], s);
  setLineColor(royalBlue());
  drawVector(p, r[2], s);
  dt ? glEnable(GL_DEPTH_TEST) : 0;
}

void
GLRenderer::drawGround(REAL size, REAL step)
{
  setLineColor(Color(0.2f, 0.2f, 0.2f));
  for (float s = step; s <= size; s += step)
  {
    drawLine(vec3(-size, 0, +s), vec3(size, 0, +s));
    drawLine(vec3(-size, 0, -s), vec3(size, 0, -s));
    drawLine(vec3(+s, 0, -size), vec3(+s, 0, size));
    drawLine(vec3(-s, 0, -size), vec3(-s, 0, size));
  }
  setLineColor(Color::red);
  drawLine(vec3(-size, 0, 0), vec3(size, 0, 0));
  setLineColor(royalBlue());
  drawLine(vec3(0, 0, -size), vec3(0, 0, size));
}
