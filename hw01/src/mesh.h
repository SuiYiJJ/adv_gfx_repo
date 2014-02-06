#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <utility>
#include <cassert>
#include <cstddef>
#include <typeinfo>

#include "vectors.h"
#include "hash.h"
#include "boundingbox.h"
#include "argparser.h"

#include "edge.h"
#include "triangle.h"

// Easier to read 
typedef std::pair<Vertex*,Vertex*> vPair;

//Lets me use these classes in my .cpp file
class Vertex;
class Edge;
class Triangle;

// ======================================================================
// ======================================================================

// helper structures for VBOs, for rendering
// (note, the data stored in each of these is application specific, 
// adjust as needed!)

struct VBOVert {
  VBOVert() {}
  VBOVert(const Vec3f &p) {
    x = p.x(); y = p.y(); z = p.z();
  }
  float x, y, z;    // position
};

struct VBOEdge {
  VBOEdge() {}
  VBOEdge(unsigned int a, unsigned int b) {
    verts[0] = a;
    verts[1] = b;
  }
  unsigned int verts[2];
};

struct VBOTriVert {
  VBOTriVert() {}
  VBOTriVert(const Vec3f &p, const Vec3f &n) {
    x = p.x(); y = p.y(); z = p.z();
    nx = n.x(); ny = n.y(); nz = n.z();
  }
  float x, y, z;    // position
  float nx, ny, nz; // normal
};

struct VBOTri {
  VBOTri() {}
  VBOTri(unsigned int a, unsigned int b, unsigned int c) {
    verts[0] = a;
    verts[1] = b;
    verts[2] = c;
  }
  unsigned int verts[3];
};

// ======================================================================
// ======================================================================
// Stores and renders all the vertices, triangles, and edges for a 3D model

class Mesh {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Mesh(ArgParser *a) { args = a; }
  ~Mesh();
  void Load(const std::string &input_file);
  
  // ========
  // VERTICES
  int numVertices() const { return vertices.size(); }
  Vertex* addVertex(const Vec3f &pos);
  // look up vertex by index from original .obj file
  Vertex* getVertex(int i) const {
    assert (i >= 0 && i < numVertices());
    Vertex *v = vertices[i];
    assert (v != NULL);
    return v; }

  // ==================================================
  // PARENT VERTEX RELATIONSHIPS (used for subdivision)
  // this creates a relationship between 3 vertices (2 parents, 1 child)
  void setParentsChild(Vertex *p1, Vertex *p2, Vertex *child);
  // this accessor will find a child vertex (if it exists) when given
  // two parent vertices
  Vertex* getChildVertex(Vertex *p1, Vertex *p2) const;

  // =====
  // EDGES
  int numEdges() const { return edges.size(); }
  // this efficiently looks for an edge with the given vertices, using a hash table
  Edge* getMeshEdge(Vertex *a, Vertex *b) const;

  // =========
  // TRIANGLES
  int numTriangles() const { return triangles.size(); }
  void addTriangle(Vertex *a, Vertex *b, Vertex *c);
  void removeTriangle(Triangle *t);

  // ===============
  // OTHER ACCESSORS
  const BoundingBox& getBoundingBox() const { return bbox; }

  void printEdges(){

    std::cout << "*-_-*-_-*-_-*-_EDGES-*-_-*-_-*-_-*-_-*" <<std::endl;
    for (edgeshashtype::iterator iter = edges.begin();
         iter != edges.end(); iter++) {

      std::cout << (*iter).second << std::endl;
      //std::cout << (*iter).second.first->getStartVertex().getIndex() << "\t"; 
      //std::cout << (*iter).second.second->getStartVertex().getIndex() << "\n"; 
    }
    
  }
  
  
  //JUMP
  bool alteredTriPair(Edge* cur, Vertex* deadVertex, Vertex*  mergeVertex, vPair& alteredPair){
    
    // WARNING: The following function returns vPair with no contents
    // in some instances. We applogize for any inconvince this might cause
    // and appricate your continued support. 

    // Get three nodes of the triangle
    // NOTE: One is bound to be deadVertex, one might be mergeVertex
    Vertex* a = cur->getStartVertex();                       
    Vertex* b = cur->getNext()->getStartVertex();            
    Vertex* c = cur->getNext()->getNext()->getStartVertex(); 

    // Check if valid, if valid return pair ELSE null pair
    if( a == mergeVertex || b == mergeVertex || c == mergeVertex ){
      // Do nothing, leaving alterPair unititated
      return false;
    }else if( a == deadVertex){
      //Vec3f one = c->getPos(); // Probabily redudent
      //Vec3f two = b->getPos(); 
      //alteredPair = std::make_pair(one,two);
      alteredPair = std::make_pair(b,c);

    }else if( b == deadVertex){
      //Vec3f one = a->getPos();
      //Vec3f two = c->getPos(); 
      //alteredPair = std::make_pair(one,two);
      alteredPair = std::make_pair(a,c);

    }else if( c == deadVertex){
      //Vec3f one = a->getPos();
      //Vec3f two = b->getPos(); 
      //alteredPair = std::make_pair(one,two);
      alteredPair = std::make_pair(a,b);

    }else{
      // Something went wrong...
      std::cout << "Something wen wrong" <<std::endl;
    }

    return true;
  }
  
  // ===+=====
  // RENDERING
  void initializeVBOs();
  void setupVBOs();
  void drawVBOs();
  void cleanupVBOs();

  // ==========================
  // MESH PROCESSING OPERATIONS
  void LoopSubdivision();
  void Simplification(int target_tri_count);

private:

  // don't use these constructors
  Mesh(const Mesh &/*m*/) { assert(0); exit(0); }
  const Mesh& operator=(const Mesh &/*m*/) { assert(0); exit(0); }

  // helper functions
  void setupTriVBOs();
  void setupEdgeVBOs();
  
  // ==============
  // REPRESENTATION
  ArgParser *args;                //Arguments

  std::vector<Vertex*> vertices;  //Vector of vertices pointers
                                  //Where are they created?

  edgeshashtype edges;            //Hash table of edges
  triangleshashtype triangles;    //Hash table of triangles
  BoundingBox bbox;               //bbox?
  vphashtype vertex_parents;      //List of parents of each vertex

  //TODO Find out how verticies are handled!

  int num_boundary_edges;
  int num_crease_edges;
  int num_other_edges;

  GLuint mesh_tri_verts_VBO;
  GLuint mesh_tri_indices_VBO;
  GLuint mesh_verts_VBO;
  GLuint mesh_boundary_edge_indices_VBO;
  GLuint mesh_crease_edge_indices_VBO;
  GLuint mesh_other_edge_indices_VBO;

};

// ======================================================================
// ======================================================================


#endif




