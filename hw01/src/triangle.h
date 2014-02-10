#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include <string>
#include <vector>
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

  /* This is dead to me
  void getSubTrianglesVec3f(std::vector<std::vector<Vec3f>>& subTriVec){
    // Input: Empty Vector
    // Assume: None creased triangle
    // Output: A vector of Vec3f with coorindates of where the
    //         new triangles should go
    // Modify  Puts result in the subTriVec
  
    // Get the old positions
    Vec3f a = edge->getStartVertex()->getPos();
    Vec3f b = edge->getNext()->getStartVertex()->getPos();
    Vec3f c = edge->getNext()->getNext()->getStartVertex()->getPos();
    
    Vec3f abMid = a.midPoint3f(b);
    Vec3f bcMid = b.midPoint3f(c);
    Vec3f caMid = a.midPoint3f(c);

    assert(a.midPoint3f(b) == b.midPoint3f(a));
    assert(b.midPoint3f(c) == c.midPoint3f(b));
    assert(a.midPoint3f(c) == c.midPoint3f(a));

    std::cout << "abMid: " << abMid  << "\t"; 
    std::cout << "bcMid: " << bcMid  << "\t"; 
    std::cout << "acMid: " << caMid  << "\t"; 
    // Note: Pushing in clockwise
    
    //Triangle a
    std::vector<Vec3f> aTri;
    aTri.push_back(a);
    aTri.push_back(abMid);
    aTri.push_back(caMid);


    //Triangle b
    std::vector<Vec3f> bTri;
    bTri.push_back(b);
    bTri.push_back(bcMid);
    bTri.push_back(abMid);


    //Triangle c 
    std::vector<Vec3f> cTri;
    cTri.push_back(c);
    cTri.push_back(caMid);
    cTri.push_back(bcMid);

    //Triangle mid
    std::vector<Vec3f> midTri;
    midTri.push_back(abMid);
    midTri.push_back(bcMid);
    midTri.push_back(caMid);

    //Adding them to result vec
    subTriVec.push_back(aTri);
    subTriVec.push_back(bTri);
    subTriVec.push_back(cTri);
    subTriVec.push_back(midTri);

  }
  */ // DEAD CODE
    

  
  

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
