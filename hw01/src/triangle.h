#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include "vertex.h"

// ===========================================================
// Stores the indices to the 3 vertices of the triangles, 
// used by the mesh class

class Triangle {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Triangle() {
    edge = NULL; 
    id = next_triangle_id;
    next_triangle_id++;
  }

  // =========
  // ACCESSORS
  Vertex* operator[](int i) const { 
    assert (edge != NULL);
    if (i==0) return edge->getStartVertex();
    if (i==1) return edge->getNext()->getStartVertex();
    if (i==2) return edge->getNext()->getNext()->getStartVertex();
    assert(0); exit(0);
  }

  Edge* getEdge() { 
    assert (edge != NULL);
    return edge; 
  }

  void setEdge(Edge *e) {
    assert (edge == NULL);
    edge = e;
  }

  int getID() { return id; }

  Vec3f getNormal(){
    Vec3f p1 = edge->getStartVertex()->getPos();
    Vec3f p2 = edge->getNext()->getStartVertex()->getPos();
    Vec3f p3 = edge->getNext()->getNext()->getStartVertex()->getPos();
    
    Vec3f v12 = p2;
    v12 -= p1;
    Vec3f v23 = p3;
    v23 -= p2;
    Vec3f normal;
    Vec3f::Cross3(normal,v12,v23);
    normal.Normalize();
    return normal;
  }

  // NOTE: If you want to modify a triangle, it is recommended that
  // you remove it from the mesh, delete it, create a triangle object
  // with the changes, and re-add it.  This will ensure the edges get
  // updated appropriately.

protected:

  // don't use these constructors
  Triangle(const Triangle &/*t*/) { assert(0); exit(0); }
  Triangle& operator= (const Triangle &/*t*/) { assert(0); exit(0); }
  
  // ==============
  // REPRESENTATION
  Edge *edge;
  int id;

  // triangles are indexed starting at 0
  static int next_triangle_id;
};

// ===========================================================

#endif
