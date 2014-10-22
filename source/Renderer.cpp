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
//  OVERVIEW: Renderer.cpp
//  ========
//  Source file for generic renderer.

#include "Renderer.h"

//
// Canonical view volume
//
#define CVVX1 (REAL)-1.0
#define CVVX2 (REAL)+1.0
#define CVVY1 (REAL)-1.0
#define CVVY2 (REAL)+1.0

#define DFL_IMAGe_W 400
#define DFL_IMAGE_H 400

using namespace Graphics;


//////////////////////////////////////////////////////////
//
// Renderer implementation
// ========
Renderer::Renderer(Scene& aScene, Camera* aCamera):
  scene(&aScene),
  camera(aCamera != 0 ? aCamera : new Camera()),
  defaultLight(0)
//[]---------------------------------------------------[]
//|  Constructor                                        |
//[]---------------------------------------------------[]
{
  makeDefaultLight();
}

Renderer::~Renderer()
//[]---------------------------------------------------[]
//|  Destructor                                         |
//[]---------------------------------------------------[]
{
  delete defaultLight;
}

Light*
Renderer::makeDefaultLight()
//[]---------------------------------------------------[]
//|  Make default light                                 |
//[]---------------------------------------------------[]
{
  const vec3& p = camera->getPosition();

  if (defaultLight == 0)
  {
    defaultLight = new Light(p, Color::gray);
    System::makeUse(defaultLight);
  }
  //else
  //  defaultLight->position = p;
  return defaultLight;
}

void
Renderer::setScene(Scene& scene)
//[]---------------------------------------------------[]
//|  Set scene                                          |
//[]---------------------------------------------------[]
{
  if (&scene != this->scene)
    this->scene = &scene;
}

void
Renderer::setCamera(Camera* camera)
//[]---------------------------------------------------[]
//|  Set camera                                         |
//[]---------------------------------------------------[]
{
  if (camera != this->camera)
    this->camera = camera != 0 ? camera : new Camera();
}

void
Renderer::setImageSize(int w, int h)
//[]---------------------------------------------------[]
//|  Set imagem size                                    |
//[]---------------------------------------------------[]
{
  W = w;
  H = h;
}

void
Renderer::update()
//[]---------------------------------------------------[]
//|  Update                                             |
//[]---------------------------------------------------[]
{
  camera->updateView();
}
