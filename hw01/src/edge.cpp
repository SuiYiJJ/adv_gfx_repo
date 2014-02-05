#ifndef _EDGE_H_
#define _EDGE_H_

#include "vertex.h"
#include "edge.h"
#include "triangle.h"   //max
#include <math.h>


// ===========
// CONSTRUCTOR
Edge::Edge(Vertex *vs, Vertex *ve, Triangle *t) {
  start_vertex = vs;
  end_vertex = ve;
  triangle = t;
  next = NULL;
  opposite = NULL;
  crease = 0;
}


// ==========
// DESTRUCTOR
Edge::~Edge() { 
  // disconnect from the opposite edge
  if (opposite != NULL)
    opposite->opposite = NULL;
  // NOTE: the "prev" edge (which has a "next" pointer pointing to
  // this edge) will also be deleted as part of the triangle removal,
  // so we don't need to disconnect that
}


// ========
// ACCESSOR
float Edge::Length() const {
  Vec3f diff = start_vertex->getPos() - end_vertex->getPos();
  return diff.Length();
}

float Edge::DihedralAngle(){
  /*
   * Warning this function returns NULL 
   * when there is an edge without two 
   * adjcent triangles
   *
   */

  // Is there even an angle here?
  if(opposite == NULL)
    return (float)NULL;

  // Find the angle of the face of a triangle
  Triangle* a = triangle;
  Triangle* b = opposite->getTriangle();

  Vec3f normalA = a->getNormal();
  Vec3f normalB = b->getNormal();

  // Using Equation theta = acos( (a . b) / (|a||b|)) 
  double top = normalA.Dot3(normalB);
  double bottom = normalA.Length() * normalB.Length();
  double result = acos(top/bottom);
  return result;

}
#endif
