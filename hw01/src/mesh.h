#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <utility>
#include <cassert>
#include <cstddef>
#include <typeinfo>
#include <map>

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
  Mesh(ArgParser *a) { 
    args = a;
    sub_division_level = 0;
  }

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
  Edge* getShortestEdge();
  int identifyVertex(Edge * edge);

  int adjSharpEdges(Edge* edge){

    // Assume I can circle around the edge's start vertex
    int s = 0;
    Edge* cur = edge;

    do{

      if(cur->getCrease() > 0 ){
        s++;
      
      }

      // Increment
      cur = cur->getOpposite();
      assert(cur != NULL);
      cur = cur->getNext();

    }while(cur != edge);
   return s;
  } 

  int valance(Edge* edge){
    
    int s = 0;
    Edge* cur = edge;

    do{
      s++;
      cur = cur->getOpposite();
      if(cur == NULL)
        return -1;
      cur = cur->getNext();
    }while(cur != edge);
   return s;
  } 


  // =========
  // TRIANGLES
  int numTriangles() const { return triangles.size(); }
  void addTriangle(Vertex *a, Vertex *b, Vertex *c);
  void removeTriangle(Triangle *t);
  void removeTriangle(Edge* edge);
  bool alteredTriPair(Edge* cur, Vertex* deadVertex, Vertex*  mergeVertex, vPair& alteredPair);
  // Subdivide methods
  void divide();
  void getControlPts_newEdge(Edge* edg, std::vector<Vec3f> &controlPts);
  void getControlPts_oldEdge(Edge* edg, std::vector<Vec3f> &controlPts);
  void getControlPts_newBound(Edge* edg, std::vector<Vec3f>  &controlPts);
  void getControlPts_oldBound(Edge* edg, std::vector<Vec3f> &controlPts);


  // ===============
  // OTHER ACCESSORS
  const BoundingBox& getBoundingBox() const { return bbox; }
  
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
  bool manifoldLegal(Edge* a, Edge* b);
  
  // ==============
  // REPRESENTATION
  ArgParser *args;                //Arguments

  std::vector<Vertex*> vertices;  //Vector of vertices pointers
                                  //Where are they created?

  std::vector<Edge *> ignoreVec;  //Ignore when trying to simplify
  std::map<Vertex*,vPair> childParentMap;
  std::map<vPair,float> creaseMap;
  

  edgeshashtype edges;            //Hash table Pair<Vertex*,Vertex*> ---> Edge*
  triangleshashtype triangles;    //Hash table of triangles
  BoundingBox bbox;               //bbox?
  vphashtype vertex_parents;      //List of parents of each vertex

  int sub_division_level;
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




