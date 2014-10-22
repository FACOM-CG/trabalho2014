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
//  OVERVIEW: MeshSweeper.cpp
//  ========
//  Source file for mesh sweeper.

#include <stdio.h>
#include "MeshSweeper.h"

using namespace Graphics;


//////////////////////////////////////////////////////////
//
// MeshSweeper implementation
// ===========
//
TriangleMesh*
MeshSweeper::makeCylinder(
  const Polyline& circle,
  const vec3& path,
  const mat4& m)
//[]----------------------------------------------------[]
//|  Make cylinder                                       |
//[]----------------------------------------------------[]
{
  int np = circle.getNumberOfVertices();
  int nv = np * 2; // number of vertices
  int nb = np - 2; // number of triangles of the base
  int nt = nv + 2 * nb; // number of triangles
  TriangleMesh::Arrays data;

  data.vertices = new vec3[data.numberOfVertices = nv];
  data.triangles = new TriangleMesh::Triangle[data.numberOfTriangles = nt];

  vec3 c(0, 0, 0);

  if (true)
  {
    Polyline::VertexIterator vit(circle.getVertexIterator());

    for (int i = 0; i < np; i++)
    {
      const vec3& p = vit++.position;

      c += p;
      data.vertices[i + np] = m.transform3x4(p);
      data.vertices[i] = m.transform3x4(p + path);
    }
    c *= Math::inverse(REAL(np));
  }

  TriangleMesh::Triangle* triangle = data.triangles;

  for (int i = 0; i < np; i++)
  {
    int j = (i + np);
    int k = (i + 1) % np;

    triangle->setVertices(i, j, k);
    triangle[1].setVertices(j, k + np, k);
    triangle += 2;
  }

  int v0 = 0;
  int v1 = 1;
  int v2 = 2;

  for (int i = 0; i < nb; i++)
  {
    triangle->setVertices(v0, v1, v2);
    triangle[nb].setVertices(v0 + np, v2 + np, v1 + np);
    triangle++;
    v2 = ((v1 = (v0 = v2) + 1) + 1) % np;
  }

  TriangleMesh* mesh = new TriangleMesh(data);

  mesh->computeNormals();
  return mesh;
}

TriangleMesh*
MeshSweeper::makeBox(
  const vec3& center,
  const vec3& normal,
  const vec3& up,
  const vec3& size,
  const mat4& m)
//[]----------------------------------------------------[]
//|  Make box 1                                          |
//[]----------------------------------------------------[]
{
  Polyline poly;
  vec3 N(normal.versor());
  vec3 U(up.cross(normal).versor());
  vec3 V(N.cross(U));

  N *= size.z * (REAL)0.5;
  U *= size.x * (REAL)0.5;
  V *= size.y * (REAL)0.5;
  poly.mv(center - U - V - N);
  poly.mv(center + U - V - N);
  poly.mv(center + U + V - N);
  poly.mv(center - U + V - N);
  poly.close();
  return makeCylinder(poly, 2 * N, m);
}

TriangleMesh*
MeshSweeper::makeBox(
  const vec3& center,
  const vec3& rotation,
  const vec3& scale)
//[]----------------------------------------------------[]
//|  Make box 2                                          |
//[]----------------------------------------------------[]
{
#define BOX_O  vec3(0,0,0)
#define BOX_X  vec3(1,0,0)
#define BOX_Y  vec3(0,1,0)
#define BOX_Z  vec3(0,0,1)
#define BOX_S  vec3(1,1,1)

  mat4 m = mat4::TRS(center, quat::eulerAngles(rotation), scale);
  return makeBox(BOX_O, BOX_Z, BOX_Y, BOX_S, m);

#undef BOX_O
#undef BOX_X
#undef BOX_Y
#undef BOX_Z
#undef BOX_S
}

TriangleMesh*
MeshSweeper::makeCylinder(
  const vec3& center,
  REAL radius,
  const vec3& normal,
  REAL height,
  int segments)
//[]----------------------------------------------------[]
//|  Make cylinder                                       |
//[]----------------------------------------------------[]
{
  Polyline circle = makeCircle(center, radius, normal, segments);
  return makeCylinder(circle, normal * -height);
}

TriangleMesh*
MeshSweeper::makeSphere(const vec3& center, REAL radius, int mers)
//[]----------------------------------------------------[]
//|  Make sphere                                         |
//[]----------------------------------------------------[]
{
  if (mers < 6)
    mers = 6;

  int sections = mers;
  int nv = sections * mers + 2; // number of vertices (and normals)
  int nt = 2 * mers * sections; // number of triangles
  TriangleMesh::Arrays data;

  data.vertices = new vec3[data.numberOfVertices = nv];
  data.normals = new vec3[data.numberOfNormals = nv];
  data.triangles = new TriangleMesh::Triangle[data.numberOfTriangles = nt];
  {
    Polyline arc = makeArc(center, radius, vec3(0, 0, 1), 180, sections + 1);
    Polyline::VertexIterator vit = arc.getVertexIterator();
    vec3* vertex = data.vertices;
    vec3* normal = data.normals;
    REAL invRadius = Math::inverse<REAL>(radius);

    *normal = ((*vertex = (vit++).position) - center) * invRadius;

    mat4 rot = mat4::rotation(*normal, REAL(360) / mers, center);

    vertex++;
    normal++;
    for (int s = 0; s < sections; s++)
    {
      vec3 p = *vertex = (vit++).position;

      *normal = (p - center) * invRadius;
      vertex++;
      normal++;
      for (int m = 1; m < mers; m++)
      {
        *vertex = p = rot.transform3x4(p);
        *normal = (p - center) * invRadius;
        vertex++;
        normal++;
      }
    }
    *normal = ((*vertex = (vit++).position) - center) * invRadius;
  }

  TriangleMesh::Triangle* triangle = data.triangles;

  for (int i = 1; i <= mers; i++)
  {
    int j = i % mers + 1;

    triangle->setVertices(0, i, j);
    triangle++;
  }
  for (int s = 1; s < sections; s++)
    for (int m = 0, b = (s - 1) * mers + 1; m < mers;)
    {
      int i = b + m;
      int k = b + ++m % mers;
      int j = i + mers;
      int l = k + mers;

      triangle->setVertices(i, j, k);
      triangle[1].setVertices(k, j, l);
      triangle += 2;
    }
  for (int m = 0, b = (sections - 1) * mers + 1, j = nv - 1; m < mers;)
  {
    int i = b + m;
    int k = b + ++m % mers;

    triangle->setVertices(i, j, k);
    triangle++;
  }
  return new TriangleMesh(data);
}
