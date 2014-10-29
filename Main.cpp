#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GLRenderer.h"
#include "MeshReader.h"
#include "MeshSweeper.h"
#include "Scene.h"

#define WIN_W 1024
#define WIN_H 768

using namespace Graphics;

GLRenderer* renderer;
Scene* scene;

// Mouse globals
int mouseX, mouseY;

// Keyboard globals
const int MAX_KEYS = 256;
bool keys[MAX_KEYS];

// Camera globals
const float CAMERA_RES = 0.01f;
const float ZOOM_SCALE = 1.01f;

// Animation globals
bool animateFlag;
const int UPDATE_RATE = 40;

inline void
printControls()
{
  printf("\n"
    "Camera controls:\n"
    "----------------\n"
    "(w) pan forward  (s) pan backward\n"
    "(q) pan up       (z) pan down\n"
    "(a) pan left     (d) pan right\n"
    "(+) zoom in      (-) zoom out\n"
    "GL render mode controls:\n"
    "------------------------\n"
    "(b) bounds       (n) normals       (x) axes\n"
    "(,) wireframe    (/) Smooth\n\n");
}

static bool drawAxes = false;
static bool drawBounds = false;
static bool drawNormals = false;

void
processKeys()
{
  Camera* camera = renderer->getCamera();

  for (int i = 0; i < MAX_KEYS; i++)
  {
    if (!keys[i])
      continue;

    float len = camera->getDistance() * CAMERA_RES;

    switch (i)
    {
      // Camera controls
      case 'w':
        camera->move(0, 0, -len);
        break;
      case 's':
        camera->move(0, 0, +len);
        break;
      case 'q':
        camera->move(0, +len, 0);
        break;
      case 'z':
        camera->move(0, -len, 0);
        break;
      case 'a':
        camera->move(-len, 0, 0);
        break;
      case 'd':
        camera->move(+len, 0, 0);
        break;
      case '-':
        camera->zoom(1.0f / ZOOM_SCALE);
        keys[i] = false;
        break;
      case '+':
        camera->zoom(ZOOM_SCALE);
        keys[i] = false;
        break;
      case 'p':
        camera->changeProjectionType();
        break;
      case 'b':
        drawBounds ^= true;
        renderer->flags.enable(GLRenderer::DrawSceneBounds, drawBounds);
        renderer->flags.enable(GLRenderer::DrawActorBounds, drawBounds);
        break;
      case 'x':
        drawAxes ^= true;
        renderer->flags.enable(GLRenderer::DrawAxes, drawAxes);
        break;
      case 'n':
        drawNormals ^= true;
        renderer->flags.enable(GLRenderer::DrawNormals, drawNormals);
        break;
      case ',':
        renderer->renderMode = GLRenderer::Wireframe;
        break;
      case '/':
        renderer->renderMode = GLRenderer::Smooth;
        break;
    }
  }
}

void
initGL(int *argc, char **argv)
{
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  //glutInitContextProfile(GLUT_CORE_PROFILE);
  //glutInitContextVersion(3, 3);
  glutInitWindowSize(WIN_W, WIN_H);
  glutCreateWindow("RT");
  GLSL::init();
  glutReportErrors();
}

void
displayCallback()
{
  processKeys();
  renderer->render();
  glutSwapBuffers();
}

void
reshapeCallback(int w, int h)
{
  renderer->setImageSize(w, h);
  renderer->getCamera()->setAspectRatio(REAL(w) / REAL(h));
}

void
mouseCallback(int, int, int x, int y)
{
  mouseX = x;
  mouseY = y;
}

void
motionCallback(int x, int y)
{
  Camera* camera = renderer->getCamera();
  float da = camera->getViewAngle() * CAMERA_RES;
  float ay = (mouseX - x) * da;
  float ax = (mouseY - y) * da;

  camera->rotateYX(ay, ax);
  mouseX = x;
  mouseY = y;
  glutPostRedisplay();
}

void
mouseWheelCallback(int, int dir, int, int y)
{
  if (y == 0)
    return;
  if (dir > 0)
    renderer->getCamera()->zoom(ZOOM_SCALE);
  else
    renderer->getCamera()->zoom(1.0f / ZOOM_SCALE);
  glutPostRedisplay();
}

void
idleCallback()
{
  static GLint currentTime;
  GLint time = glutGet(GLUT_ELAPSED_TIME);

  if (abs(time - currentTime) >= UPDATE_RATE)
  {
    Camera* camera = renderer->getCamera();

    camera->azimuth(camera->getHeight() * CAMERA_RES);
    currentTime = time;
    glutPostRedisplay();
  }
}

void
keyboardCallback(unsigned char key, int /*x*/, int /*y*/)
{
  keys[key] = true;
  glutPostRedisplay();
}

void
keyboardUpCallback(unsigned char key, int /*x*/, int /*y*/)
{
  keys[key] = false;
  switch (key)
  {
    case 27:
      exit(EXIT_SUCCESS);
      break;
    case 'o':
      animateFlag ^= true;
      glutIdleFunc(animateFlag ? idleCallback : 0);
      glutPostRedisplay();
      break;
  }
}

Actor*
newActor(
  TriangleMesh* mesh,
  const vec3& position = vec3::null(),
  const vec3& size = vec3(1, 1, 1),
  const Color& color = Color::white)
{
  Primitive* p = new TriangleMeshShape(mesh);

  p->setMaterial(MaterialFactory::New(color));
  p->setMatrix(position, quat::identity(), size);
  return new Actor(*p);
}

void
createScene()
{
  TriangleMesh* s = MeshSweeper::makeSphere();

  scene = new Scene("test");
  scene->addActor(newActor(s, vec3(-3, -3, 0), vec3(1, 1, 1), Color::yellow));
  scene->addActor(newActor(s, vec3(+3, -3, 0), vec3(2, 1, 1), Color::green));
  scene->addActor(newActor(s, vec3(+3, +3, 0), vec3(1, 2, 1), Color::red));
  scene->addActor(newActor(s, vec3(-3, +3, 0), vec3(1, 1, 2), Color::blue));
  s = MeshReader().execute("f-16.obj");
  scene->addActor(newActor(s, vec3(2, -4, -10)));
}

int
main(int argc, char **argv)
{
  // init OpenGL
  initGL(&argc, argv);
  glutDisplayFunc(displayCallback);
  glutReshapeFunc(reshapeCallback);
  glutMouseFunc(mouseCallback);
  glutMotionFunc(motionCallback);
  glutMouseWheelFunc(mouseWheelCallback);
  glutKeyboardFunc(keyboardCallback);
  glutKeyboardUpFunc(keyboardUpCallback);
  // print controls
  printControls();
  // create the scene
  createScene();
  // create the renderer
  renderer = new GLRenderer(*scene);
  renderer->renderMode = GLRenderer::Smooth;
  glutMainLoop();
  return 0;
}
