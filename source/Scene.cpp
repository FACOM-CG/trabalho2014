//[]------------------------------------------------------------------------[]
//|                                                                          |
//|                          GVSG Graphics Classes                           |
//|                               Version 1.0                                |
//|                                                                          |
//|              Copyright® 2007-2014, Paulo Aristarco Pagliosa              |
//|              All Rights Reserved.                                        |
//|                                                                          |
//[]------------------------------------------------------------------------[]
//
//  OVERVIEW: Scene.cpp
//  ========
//  Source file for scene.

#ifndef __Scene_h
#include "Scene.h"
#endif

using namespace Graphics;


//////////////////////////////////////////////////////////
//
// Scene implementation
// =====
Scene*
Scene::New()
{
  static uint sid;
  static char name[16];

  sprintf(name, "scene%d", ++sid);
  return new Scene(name);
}

Scene::~Scene()
//[]---------------------------------------------------[]
//|  Destructor                                         |
//[]---------------------------------------------------[]
{
  deleteAll();
}

Actor*
Scene::findActor(Model* model) const
//[]---------------------------------------------------[]
//|  Find actor                                         |
//[]---------------------------------------------------[]
{
  for (Actor* actor = actors.peekHead(); actor != 0; actor = actor->next)
  if (actor->model == model)
    return actor;
  return 0;
}

void
Scene::addActor(Actor* actor)
//[]---------------------------------------------------[]
//|  Add actor                                          |
//[]---------------------------------------------------[]
{
  if (actor != 0)
  {
    actors.add(actor);
    actor->scene = this;
    System::makeUse(actor);
    if (!modifiedBounds)
      bounds.inflate(actor->model->boundingBox());
  }
}

void
Scene::deleteActor(Actor* actor)
//[]---------------------------------------------------[]
//|  Delete actor                                       |
//[]---------------------------------------------------[]
{
  if (actor != 0 && actor->getScene() == this)
  {
    actors.remove(*actor);
    actor->scene = 0;
    actor->release();
    modifiedBounds = true;
  }
}

void
Scene::addLight(Light* light)
//[]---------------------------------------------------[]
//|  Add light                                          |
//[]---------------------------------------------------[]
{
  if (light != 0)
  {
    lights.add(light);
    light->scene = this;
    System::makeUse(light);
  }
}

void
Scene::deleteLight(Light* light)
//[]---------------------------------------------------[]
//|  Delete light                                       |
//[]---------------------------------------------------[]
{
  if (light != 0 && light->getScene() == this)
  {
    lights.remove(*light);
    light->scene = 0;
    light->release();
  }
}

void
Scene::deleteActors()
//[]---------------------------------------------------[]
//|  Delete all actors                                  |
//[]---------------------------------------------------[]
{
  for (Actor* actor; (actor = actors.peekHead()) != 0;)
  {
    actors.remove(*actor);
    actor->scene = 0;
    actor->release();
  }
  bounds.setEmpty();
  modifiedBounds = false;
}

void
Scene::deleteLights()
//[]---------------------------------------------------[]
//|  Delete all lights                                  |
//[]---------------------------------------------------[]
{
  for (Light* light; (light = lights.peekHead()) != 0;)
  {
    lights.remove(*light);
    light->scene = 0;
    light->release();
  }
}

inline void
Scene::updateBounds()
{
  bounds.setEmpty();
  for (Actor* actor = actors.peekHead(); actor != 0; actor = actor->next)
    bounds.inflate(actor->model->boundingBox());
}

const Bounds3&
Scene::boundingBox()
//[]---------------------------------------------------[]
//|  Bounding box                                       |
//[]---------------------------------------------------[]
{
  if (modifiedBounds)
    updateBounds();
  return bounds;
}
