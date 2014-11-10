#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GLRenderer.h"
#include "MeshReader.h"
#include "MeshSweeper.h"
#include "Scene.h"
#include "tinyxml2.h"
#include <string.h>

#define WIN_W 1024
#define WIN_H 768

using namespace Graphics;
using namespace tinyxml2;

GLRenderer* renderer;
Scene* scene;
Camera* camera;

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
createScene(bool isDefault, char* filename = NULL)
{
	if (isDefault) //gera a cena padrão
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
	else //gera a cena baseada no arquivo a ser lido
	{
		XMLDocument doc;
		doc.LoadFile(filename);
		XMLNode* root = doc.FirstChildElement("rt");
		XMLNode* cur, *aux;

		//checa a existencia do elemento image
		if (root->FirstChildElement("image"))
		{
			cur = root->FirstChildElement("image");
			if (cur->FirstChildElement("width"))
			{
				//TODO: alocar o width
			}
			if (cur->FirstChildElement("height"))
			{
				//TODO: alocar o height
			}
		}
		//checa a existencia do elemento camera
		/*if (root->FirstChildElement("camera"))
		{
			cur = root->FirstChildElement("camera");

			if (cur->FirstChildElement("position"))
			{
				string text = cur->FirstChildElement("position")->GetText();
				vec3 pos;

				int c_pos1 = text.find(" ");
				int c_pos2 = text.find(" ", c_pos1+1);
				int c_pos3 = text.find(" ", c_pos2 + 1);

				pos.x = atof(text.substr(0, c_pos1 - 1).c_str());
				pos.y = atof(text.substr(c_pos1 + 1, c_pos2 - 1).c_str());
				pos.z = atof(text.substr(c_pos2 + 1).c_str());
				
				camera->setPosition(pos);
			}
			if (cur->FirstChildElement("to"))
			{
				string text = cur->FirstChildElement("to")->GetText();
				vec3 to;

				int c_pos1 = text.find(" ");
				int c_pos2 = text.find(" ", c_pos1 + 1);
				int c_pos3 = text.find(" ", c_pos2 + 1);

				to.x = atof(text.substr(0, c_pos1 - 1).c_str());
				to.y = atof(text.substr(c_pos1 + 1, c_pos2 - 1).c_str());
				to.z = atof(text.substr(c_pos2 + 1).c_str());

				camera->setDirectionOfProjection(to);
			}
			if (cur->FirstChildElement("up"))
			{
				string text = cur->FirstChildElement("up")->GetText();
				vec3 up;

				int c_pos1 = text.find(" ");
				int c_pos2 = text.find(" ", c_pos1 + 1);
				int c_pos3 = text.find(" ", c_pos2 + 1);

				up.x = atof(text.substr(0, c_pos1 - 1).c_str());
				up.y = atof(text.substr(c_pos1 + 1, c_pos2 - 1).c_str());
				up.z = atof(text.substr(c_pos2 + 1).c_str());

				camera->setViewUp(up);
			}
			if (cur->FirstChildElement("angle"))
			{
				string text = cur->FirstChildElement("angle")->GetText();
				camera->setViewAngle(atof(text.c_str()));
			}
			if (cur->FirstChildElement("aspect"))
			{
				float aspect;
				int w, h;

				string text = cur->FirstChildElement("aspect")->GetText();

				int c_pos1 = text.find(":");

				w = atoi(text.substr(0, c_pos1 - 1).c_str());
				h = atoi(text.substr(c_pos1 + 1).c_str());
				aspect = w / h;

				camera->setAspectRatio(aspect);
			}
		}*/

		//Leitura do elemento scene
		cur = root->FirstChildElement("scene");
		scene = new Scene("Pavarine");
		for (tinyxml2::XMLElement* aux = cur->FirstChildElement(); aux != NULL; aux = aux->NextSiblingElement())
		{
			if ( strcmp(aux->Value(),"background") == 0 )
			{
				printf("achou background a scene\n");
				string text = cur->FirstChildElement("background")->GetText();
				Color color;

				int c_pos1 = text.find(" ");
				int c_pos2 = text.find(" ", c_pos1 + 1);
				int c_pos3 = text.find(" ", c_pos2 + 1);

				color.r = atof(text.substr(0, c_pos1 - 1).c_str());
				color.g = atof(text.substr(c_pos1 + 1, c_pos2 - 1).c_str());
				color.b = atof(text.substr(c_pos2 + 1).c_str());

				scene->backgroundColor = color;
				
			}
			else if (strcmp(aux->Value(), "ambient") == 0)
			{
				printf("achou ambient da scene\n");
				string text = cur->FirstChildElement("ambient")->GetText();
				Color color;

				int c_pos1 = text.find(" ");
				int c_pos2 = text.find(" ", c_pos1 + 1);
				int c_pos3 = text.find(" ", c_pos2 + 1);

				color.r = atof(text.substr(0, c_pos1 - 1).c_str());
				color.g = atof(text.substr(c_pos1 + 1, c_pos2 - 1).c_str());
				color.b = atof(text.substr(c_pos2 + 1).c_str());

				scene->ambientLight = color;
			}
			else if (strcmp(aux->Value(), "light") == 0)
			{
				//TODO: alocar light
			}
			else if (strcmp(aux->Value(), "mesh") == 0)
			{
				//TODO: alocar mesh
			}
			else if (strcmp(aux->Value(), "sphere") == 0)
			{
				printf("achou sphere\n");
				XMLElement* aux2;
				aux2 = aux;

				vec3 center;
				vec3 radius;

				if (aux2->FirstChildElement("center"))
				{
					string text = aux2->FirstChildElement("center")->GetText();
					
					int c_pos1 = text.find(" ");
					int c_pos2 = text.find(" ", c_pos1 + 1);
					int c_pos3 = text.find(" ", c_pos2 + 1);

					center.x = atof(text.substr(0, c_pos1 - 1).c_str());
					center.y = atof(text.substr(c_pos1 + 1, c_pos2 - 1).c_str());
					center.z = atof(text.substr(c_pos2 + 1).c_str());
				}
				if (aux2->FirstChildElement("radius"))
				{
					string text = aux2->FirstChildElement("radius")->GetText();
					radius.x = atof(text.c_str());
					radius.y = atof(text.c_str());
					radius.z = atof(text.c_str());
				}

				TriangleMesh* s = MeshSweeper::makeSphere();
				scene->addActor(newActor(s, center, radius));

			}
			else if (aux->Value() == "box")
			{
				//TODO: alocar box
			}
			else if (aux->Value() == "cone")
			{
				//TODO: alocar cone
			}
			else if (aux->Value() == "cylinder")
			{
				//TODO: alocar cylinder
			}

		}



	}
  
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
  createScene(false, argv[1]);
  // create the renderer
  renderer = new GLRenderer(*scene);
  renderer->renderMode = GLRenderer::Smooth;
  glutMainLoop();
  return 0;
}
