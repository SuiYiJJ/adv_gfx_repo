#include "glCanvas.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <map>


#include "mesh.h"
#include "edge.h"
#include "vertex.h"
#include "triangle.h"

// Easier to read 
typedef std::pair<Vertex*,Vertex*> vPair;



int Triangle::next_triangle_id = 0;

// helper for VBOs
#define BUFFER_OFFSET(i) ((char *)NULL + (i))


// =======================================================================
// MESH DESTRUCTOR 
// =======================================================================

Mesh::~Mesh() {
  cleanupVBOs();

  // delete all the triangles
  std::vector<Triangle*> todo;
  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;
    todo.push_back(t);
  }
  int num_triangles = todo.size();
  for (int i = 0; i < num_triangles; i++) {
    removeTriangle(todo[i]);
  }
  // delete all the vertices
  int num_vertices = numVertices();
  for (int i = 0; i < num_vertices; i++) {
    delete vertices[i];
  }
}

// =======================================================================
// MODIFIERS:   ADD & REMOVE
// =======================================================================

Vertex* Mesh::addVertex(const Vec3f &position) {
  int index = numVertices();
  Vertex *v = new Vertex(index, position);
  vertices.push_back(v);
  if (numVertices() == 1)
    bbox = BoundingBox(position,position);
  else 
    bbox.Extend(position);
  return v;
}


void Mesh::addTriangle(Vertex *a, Vertex *b, Vertex *c) {
  // create the triangle
  Triangle *t = new Triangle();
  // create the edges
  Edge *ea = new Edge(a,b,t);
  Edge *eb = new Edge(b,c,t);
  Edge *ec = new Edge(c,a,t);
  // point the triangle to one of its edges
  t->setEdge(ea);
  // connect the edges to each other
  ea->setNext(eb);
  eb->setNext(ec);
  ec->setNext(ea);
  // verify these edges aren't already in the mesh 
  // (which would be a bug, or a non-manifold mesh)
  assert (edges.find(std::make_pair(a,b)) == edges.end());
  assert (edges.find(std::make_pair(b,c)) == edges.end());
  assert (edges.find(std::make_pair(c,a)) == edges.end());
  // add the edges to the master list
  edges[std::make_pair(a,b)] = ea;
  edges[std::make_pair(b,c)] = eb;
  edges[std::make_pair(c,a)] = ec;
  // setCrease
  if(creaseMap.count(std::make_pair(a,b)) == 1)
    ea->setCrease(creaseMap[std::make_pair(a,b)]);
  if(creaseMap.count(std::make_pair(b,a)) == 1)
    ea->setCrease(creaseMap[std::make_pair(b,a)]);

  if(creaseMap.count(std::make_pair(b,c)) == 1)
    eb->setCrease(creaseMap[std::make_pair(b,c)]);
  if(creaseMap.count(std::make_pair(c,b)) == 1)
    eb->setCrease(creaseMap[std::make_pair(c,b)]);
  
  if(creaseMap.count(std::make_pair(c,a)) == 1)
    ec->setCrease(creaseMap[std::make_pair(c,a)]);
  if(creaseMap.count(std::make_pair(a,c)) == 1)
    ec->setCrease(creaseMap[std::make_pair(a,c)]);

  // connect up with opposite edges (if they exist)
  edgeshashtype::iterator ea_op = edges.find(std::make_pair(b,a)); 
  edgeshashtype::iterator eb_op = edges.find(std::make_pair(c,b)); 
  edgeshashtype::iterator ec_op = edges.find(std::make_pair(a,c)); 
  if (ea_op != edges.end()) { ea_op->second->setOpposite(ea); }
  if (eb_op != edges.end()) { eb_op->second->setOpposite(eb); }
  if (ec_op != edges.end()) { ec_op->second->setOpposite(ec); }
  // add the triangle to the master list
  assert (triangles.find(t->getID()) == triangles.end());
  triangles[t->getID()] = t;
}

void Mesh::removeTriangle(Edge* edge) {
  // Input: An edge pointer
  // Assumptions: Triangle that pointer is on is valid
  // Output: None
  // Modification: Deletion of a triangle
  Edge *ea = edge;
  Edge *eb = ea->getNext();
  Edge *ec = eb->getNext();
  Vertex *a = ea->getStartVertex();
  Vertex *b = eb->getStartVertex();
  Vertex *c = ec->getStartVertex();
  // remove these elements from master lists
  edges.erase(std::make_pair(a,b)); 
  edges.erase(std::make_pair(b,c)); 
  edges.erase(std::make_pair(c,a)); 
  // check that I removed it
  assert (edges.find(std::make_pair(a,b)) == edges.end());
  assert (edges.find(std::make_pair(b,c)) == edges.end());
  assert (edges.find(std::make_pair(c,a)) == edges.end());

  Triangle* t = edge->getTriangle();
  triangles.erase(t->getID());
  // clean up memory
  delete ea;
  delete eb;
  delete ec;
  delete t;
}


void Mesh::removeTriangle(Triangle *t) {
  Edge *ea = t->getEdge();
  Edge *eb = ea->getNext();
  Edge *ec = eb->getNext();
  Vertex *a = ea->getStartVertex();
  Vertex *b = eb->getStartVertex();
  Vertex *c = ec->getStartVertex();
  // remove these elements from master lists
  edges.erase(std::make_pair(a,b)); 
  edges.erase(std::make_pair(b,c)); 
  edges.erase(std::make_pair(c,a)); 
  // check that I removed it
  assert (edges.find(std::make_pair(a,b)) == edges.end());
  assert (edges.find(std::make_pair(b,c)) == edges.end());
  assert (edges.find(std::make_pair(c,a)) == edges.end());
  triangles.erase(t->getID());
  // clean up memory
  delete ea;
  delete eb;
  delete ec;
  delete t;
}


// =======================================================================
// Helper functions for accessing data in the hash table
// =======================================================================


Edge* Mesh::getMeshEdge(Vertex *a, Vertex *b) const {
  // Given two verticies you return the correct edge
  edgeshashtype::const_iterator iter = edges.find(std::make_pair(a,b));
  if (iter == edges.end()) return NULL;
  return iter->second;
}

Vertex* Mesh::getChildVertex(Vertex *p1, Vertex *p2) const {
  // Given two verticies you get the child
  vphashtype::const_iterator iter = vertex_parents.find(std::make_pair(p1,p2)); 
  if (iter == vertex_parents.end()) return NULL;
  return iter->second; 
}

void Mesh::setParentsChild(Vertex *p1, Vertex *p2, Vertex *child) {
  // Given two verticies and a child, you set the parent
  // Some form of relationship between triangles?
  assert (vertex_parents.find(std::make_pair(p1,p2)) == vertex_parents.end());
  vertex_parents[std::make_pair(p1,p2)] = child; 
}


// =======================================================================
// the load function parses very simple .obj files
// the basic format has been extended to allow the specification 
// of crease weights on the edges.
// =======================================================================

#define MAX_CHAR_PER_LINE 200

void Mesh::Load(const std::string &input_file) {

  std::ifstream istr(input_file.c_str());
  if (!istr) {
    std::cout << "ERROR! CANNOT OPEN: " << input_file << std::endl;
    return;
  }

  char line[MAX_CHAR_PER_LINE];
  std::string token, token2;
  float x,y,z;
  int a,b,c;
  int index = 0;
  int vert_count = 0;
  int vert_index = 1;

  // read in each line of the file
  while (istr.getline(line,MAX_CHAR_PER_LINE)) { 
    // put the line into a stringstream for parsing
    std::stringstream ss;
    ss << line;

    // check for blank line
    token = "";   
    ss >> token;
    if (token == "") continue;

    if (token == std::string("usemtl") ||
	token == std::string("g")) {
      vert_index = 1; 
      index++;
    } else if (token == std::string("v")) {
      vert_count++;
      ss >> x >> y >> z;
      addVertex(Vec3f(x,y,z));
    } else if (token == std::string("f")) {
      a = b = c = -1;
      ss >> a >> b >> c;
      a -= vert_index;
      b -= vert_index;
      c -= vert_index;
      assert (a >= 0 && a < numVertices());
      assert (b >= 0 && b < numVertices());
      assert (c >= 0 && c < numVertices());
      addTriangle(getVertex(a),getVertex(b),getVertex(c));
    } else if (token == std::string("e")) {
      a = b = -1;
      ss >> a >> b >> token2;

      // whoops: inconsistent file format, don't subtract 1
      assert (a >= 0 && a <= numVertices());
      assert (b >= 0 && b <= numVertices());
      if (token2 == std::string("inf")) x = 1000000; // this is close to infinity...
      x = atof(token2.c_str());
      Vertex *va = getVertex(a);
      Vertex *vb = getVertex(b);
      Edge *ab = getMeshEdge(va,vb);
      Edge *ba = getMeshEdge(vb,va);
      assert (ab != NULL);
      assert (ba != NULL);
      ab->setCrease(x);
      ba->setCrease(x);
    } else if (token == std::string("vt")) {
    } else if (token == std::string("vn")) {
    } else if (token[0] == '#') {
    } else {
      printf ("LINE: '%s'",line);
    }
  }
}


// =======================================================================
// DRAWING
// =======================================================================

Vec3f AverageNormal(std::vector<Vec3f> &vectors ){

  //Add up all the vectors
  Vec3f sum; 

  for(unsigned int i = 0; i < vectors.size(); i++){
    sum = sum + vectors[i];
  }

  double n = vectors.size();

  sum = n * sum;

  return sum;
}


Vec3f ComputeNormal(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
  Vec3f v12 = p2;
  v12 -= p1;
  Vec3f v23 = p3;
  v23 -= p2;
  Vec3f normal;
  Vec3f::Cross3(normal,v12,v23);
  normal.Normalize();
  return normal;
}

Vec3f getAverageNormals(Edge* givenEdge){


  // Finding avereage normal of givenEdge
  Edge* cur = givenEdge;
  std::vector<Vec3f> curSum;
  bool reversed = false;

  do{

    Triangle* curTri = (*cur).getTriangle();
    // Get that face's normal
    Vec3f v1= (*curTri)[0]->getPos();
    Vec3f v2= (*curTri)[1]->getPos();
    Vec3f v3= (*curTri)[2]->getPos();   
    // Find normal of that face
    Vec3f n = ComputeNormal(v1,v2,v3);
    //Push into curSum
    curSum.push_back(n);
    //incremement pointer
    cur = cur->getOpposite();

    //If opposite is a dead end
    if(cur == NULL && !reversed){
        cur = givenEdge->getPrev();
        reversed = true;
    }else if(cur == NULL && reversed){
        cur = givenEdge;
    }else if(cur != NULL){
        if(reversed)
            cur = cur->getPrev();
        else
            cur = cur->getNext();
    }

  }while ( givenEdge != cur );

  Vec3f normal = AverageNormal(curSum);
  return normal;

}

void Mesh::initializeVBOs() {
  // create a pointer for the vertex & index VBOs
  glGenBuffers(1, &mesh_tri_verts_VBO);
  glGenBuffers(1, &mesh_tri_indices_VBO);
  glGenBuffers(1, &mesh_verts_VBO);
  glGenBuffers(1, &mesh_boundary_edge_indices_VBO);
  glGenBuffers(1, &mesh_crease_edge_indices_VBO);
  glGenBuffers(1, &mesh_other_edge_indices_VBO);
  setupVBOs();
}

void Mesh::setupVBOs() {
  HandleGLError("in setup mesh VBOs");
  setupTriVBOs();
  setupEdgeVBOs();
  HandleGLError("leaving setup mesh");
}


void Mesh::setupTriVBOs() {
  // Set up Triangle Vertex Buffer Object

  VBOTriVert* mesh_tri_verts;
  VBOTri* mesh_tri_indices;
  unsigned int num_tris = triangles.size();

  // allocate space for the data (goes on the heap)
  // NOTE: VBOTriVert -> What is it
  mesh_tri_verts = new VBOTriVert[num_tris*3];
  mesh_tri_indices = new VBOTri[num_tris];

  // write the vertex & triangle data
  unsigned int i = 0;
  triangleshashtype::iterator iter = triangles.begin();
  for (; iter != triangles.end(); iter++,i++) {
    Triangle *t = iter->second;

    Vec3f a = (*t)[0]->getPos();
    Vec3f b = (*t)[1]->getPos();
    Vec3f c = (*t)[2]->getPos();
    
    if (args->gouraud) {

      // Find avaerage normal of faces surrouding each vertex. (a,b,c)
        
      // Three Edges in triangle
      Edge* edgeA = (*t).getEdge();
      Edge* edgeB = (*t).getEdge()->getNext();
      Edge* edgeC = (*t).getEdge()->getNext()->getNext();

      // Call to getAverageNormals -- This function calculates the normal of 
      // surrounding faces of a triangle. 
      Vec3f normalA = getAverageNormals(edgeA);
      Vec3f normalB = getAverageNormals(edgeB);
      Vec3f normalC = getAverageNormals(edgeC);

      // Send normal to VBO
      mesh_tri_verts[i*3]   = VBOTriVert(a,normalA);
      mesh_tri_verts[i*3+1] = VBOTriVert(b,normalB);
      mesh_tri_verts[i*3+2] = VBOTriVert(c,normalC);

    } else {
      Vec3f normal = ComputeNormal(a,b,c);
      mesh_tri_verts[i*3]   = VBOTriVert(a,normal);
      mesh_tri_verts[i*3+1] = VBOTriVert(b,normal);
      mesh_tri_verts[i*3+2] = VBOTriVert(c,normal);
    }
    mesh_tri_indices[i] = VBOTri(i*3,i*3+1,i*3+2);
  }

  // cleanup old buffer data (if any)
  glDeleteBuffers(1, &mesh_tri_verts_VBO);
  glDeleteBuffers(1, &mesh_tri_indices_VBO);

  // copy the data to each VBO
  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOTriVert) * num_tris * 3,
	       mesh_tri_verts,
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOTri) * num_tris,
	       mesh_tri_indices, GL_STATIC_DRAW);

  delete [] mesh_tri_verts;
  delete [] mesh_tri_indices;

}


void Mesh::setupEdgeVBOs() {

  VBOVert* mesh_verts;
  VBOEdge* mesh_boundary_edge_indices;
  VBOEdge* mesh_crease_edge_indices;
  VBOEdge* mesh_other_edge_indices;

  mesh_boundary_edge_indices = NULL;
  mesh_crease_edge_indices = NULL;
  mesh_other_edge_indices = NULL;

  unsigned int num_verts = vertices.size();

  // first count the edges of each type
  num_boundary_edges = 0;
  num_crease_edges = 0;
  num_other_edges = 0;
  for (edgeshashtype::iterator iter = edges.begin();
       iter != edges.end(); iter++) {
    Edge *e = iter->second;
    int a = e->getStartVertex()->getIndex();
    int b = e->getEndVertex()->getIndex();
    if (e->getOpposite() == NULL) {
      num_boundary_edges++;
    } else {
      if (a < b) continue; // don't double count edges!
      if (e->getCrease() > 0) num_crease_edges++;
      else num_other_edges++;
    }
  }

  // allocate space for the data
  mesh_verts = new VBOVert[num_verts];
  if (num_boundary_edges > 0)
    mesh_boundary_edge_indices = new VBOEdge[num_boundary_edges];
  if (num_crease_edges > 0)
    mesh_crease_edge_indices = new VBOEdge[num_crease_edges];
  if (num_other_edges > 0)
    mesh_other_edge_indices = new VBOEdge[num_other_edges];

  // write the vertex data
  for (unsigned int i = 0; i < num_verts; i++) {
    mesh_verts[i] = VBOVert(vertices[i]->getPos());
  }

  // write the edge data
  int bi = 0;
  int ci = 0;
  int oi = 0; 
  for (edgeshashtype::iterator iter = edges.begin();
       iter != edges.end(); iter++) {
    Edge *e = iter->second;
    int a = e->getStartVertex()->getIndex();
    int b = e->getEndVertex()->getIndex();
    if (e->getOpposite() == NULL) {
      mesh_boundary_edge_indices[bi++] = VBOEdge(a,b);
    } else {
      if (a < b) continue; // don't double count edges!
      if (e->getCrease() > 0) 
	mesh_crease_edge_indices[ci++] = VBOEdge(a,b);
      else 
	mesh_other_edge_indices[oi++] = VBOEdge(a,b);
    }
  }
  assert (bi == num_boundary_edges);
  assert (ci == num_crease_edges);
  assert (oi == num_other_edges);

  // cleanup old buffer data (if any)
  glDeleteBuffers(1, &mesh_verts_VBO);
  glDeleteBuffers(1, &mesh_boundary_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_crease_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_other_edge_indices_VBO);

  // copy the data to each VBO
  glBindBuffer(GL_ARRAY_BUFFER,mesh_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOVert) * num_verts,
	       mesh_verts,
	       GL_STATIC_DRAW); 

  if (num_boundary_edges > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_boundary_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOEdge) * num_boundary_edges,
		 mesh_boundary_edge_indices, GL_STATIC_DRAW);
  }
  if (num_crease_edges > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_crease_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOEdge) * num_crease_edges,
		 mesh_crease_edge_indices, GL_STATIC_DRAW);
  }
  if (num_other_edges > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_other_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOEdge) * num_other_edges,
		 mesh_other_edge_indices, GL_STATIC_DRAW);
  }

  delete [] mesh_verts;
  delete [] mesh_boundary_edge_indices;
  delete [] mesh_crease_edge_indices;
  delete [] mesh_other_edge_indices;
}


void Mesh::cleanupVBOs() {
  glDeleteBuffers(1, &mesh_tri_verts_VBO);
  glDeleteBuffers(1, &mesh_tri_indices_VBO);
  glDeleteBuffers(1, &mesh_verts_VBO);
  glDeleteBuffers(1, &mesh_boundary_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_crease_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_other_edge_indices_VBO);
}


void Mesh::drawVBOs() {

  HandleGLError("in draw mesh");

  // scale it so it fits in the window
  Vec3f center; bbox.getCenter(center);
  float s = 1/bbox.maxDim();
  glScalef(s,s,s);
  glTranslatef(-center.x(),-center.y(),-center.z());

  // this offset prevents "z-fighting" bewteen the edges and faces
  // so the edges will always win
  if (args->wireframe) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.1,4.0); 
  } 

  // ======================
  // draw all the triangles
  unsigned int num_tris = triangles.size();
  glColor3f(1,1,1);

  // select the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, mesh_tri_verts_VBO);
  // describe the layout of data in the vertex buffer
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));

  // select the index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_tri_indices_VBO);
  // draw this data
  glDrawElements(GL_TRIANGLES, 
		 num_tris*3,
		 GL_UNSIGNED_INT,
		 BUFFER_OFFSET(0));

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  if (args->wireframe) {
    glDisable(GL_POLYGON_OFFSET_FILL); 
  }

  // =================================
  // draw the different types of edges
  if (args->wireframe) {
    glDisable(GL_LIGHTING);

    // select the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh_verts_VBO);
    // describe the layout of data in the vertex buffer
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VBOVert), BUFFER_OFFSET(0));

    // draw all the boundary edges
    glLineWidth(3);
    glColor3f(1,0,0);
    // select the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_boundary_edge_indices_VBO);
    // draw this data
    glDrawElements(GL_LINES, num_boundary_edges*2, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    // draw all the interior, crease edges
    glLineWidth(3);
    glColor3f(1,1,0);
    // select the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_crease_edge_indices_VBO);
    // draw this data
    glDrawElements(GL_LINES, num_crease_edges*2, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    // draw all the interior, non-crease edges
    glLineWidth(1);
    glColor3f(0,0,0);
    // select the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_other_edge_indices_VBO);
    // draw this data
    glDrawElements(GL_LINES, num_other_edges*2, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    glDisableClientState(GL_VERTEX_ARRAY);
  }

  HandleGLError("leaving draw VBOs");
}

// =================================================================
// SUBDIVISION
// =================================================================

// jumpSub

void Mesh::LoopSubdivision() {
  printf ("Subdivide the mesh!\n");

  // =========================================
  // Refine/Apply Mask to edges
  // =========================================

  // Divide the mesh!
  std::cout << "=-=-=-=-==--=SUBDIVIDE-=--=--==-=-\n";
  divide();
  std::cout << "=-=-=-=-==--=REFINE-=--=--==-=-\n";

  // Update Vector
  std::vector<std::pair<Vertex*, Vec3f>> updateVec;
  
  // Go through all the triangles
  triangleshashtype::iterator iter = triangles.begin();

  while(iter!=triangles.end()){
    
    // Each triangle has 3 edges/ each have a vertex
    Edge* ab = (*iter).second->getEdge();
    Edge* bc = ab->getNext();
    Edge* ca = bc->getNext();

    // Saving in array to take care of
    Edge* curEdges[3] = {ab,bc,ca};

    for(int i = 0; i < 3; i++){

      std::cout<<"-----------------\n";

      if(curEdges[i]->getStartVertex()->getRefineLevel()
          == sub_division_level){
        // This vertex has alread been calculated for
        std::cout << "Already Done\n";
        continue;
      }

      int typeVertex = identifyVertex(curEdges[i]);
      std::vector<Vec3f> controlPts;
      Vec3f newPos;
      //Vec3f newPos; 
      // 1) New Vertex
      // 2) Old Vertex
      // 3) New Boundry
      // 4) Old Boundry
      // 0) None
 
      if(typeVertex == 1){
        std::cout << "New Edge\n";
        getControlPts_newEdge(curEdges[i], controlPts);
      }

      if(typeVertex == 2){
        std::cout << "Old Edge\n";
        getControlPts_oldEdge(curEdges[i], controlPts);
      }

      if(typeVertex == 3){
        std::cout << "New Bound\n";
        getControlPts_newBound(curEdges[i], controlPts);
      }
      
      if(typeVertex == 4){
        std::cout <<"Old Bound\n";
        getControlPts_oldBound(curEdges[i], controlPts);
      }
      
      if(typeVertex == 0){
        controlPts.push_back(curEdges[i]->getStartVertex()->getPos());
        std::cout <<"Not Concidered\n";
      }//endif

      // find average of control points
      for(unsigned int c = 0; c  < controlPts.size(); c++)
        newPos = newPos + controlPts[c];

      // Toss into updateVector
      std::pair<Vertex*, Vec3f> update(curEdges[i]->getStartVertex(), newPos);
      updateVec.push_back(update);

      // Update it's refine level
      curEdges[i]->getStartVertex()->incrRefineLevel();
      
    } //forEachVertexinTriangle
    iter++;
  } //forEachTriangleinMesh

  std::cout << "-=-=-=-=--=-UPDATES-=-=-=-=-=-=-=-\n";
  // Assumed that updateVec has all updates to apply
  for(unsigned int v = 0; v < updateVec.size(); v++){
    // For everything in my updateVec go ahead and update its point
    Vertex* curVertex = updateVec[v].first;
    Vec3f curChange = updateVec[v].second;
    std::cout << "update" << curChange<< std::endl;
    curVertex->setPos(curChange);
  }
  
}

// =================================================================
// SIMPLIFICATION
// =================================================================

void Mesh::Simplification(int target_tri_count) {

  // clear out any previous relationships between vertices
  vertex_parents.clear();

  // This is my edge to collapse
  Edge* collapseEdge = getShortestEdge();

  // Make sure my half edge has its pair 
  if(collapseEdge->getOpposite() == NULL || collapseEdge->getTriangle() == NULL){
    std::cout << "Boundry Edge" << std::endl;
    return;
  }

  // deadVertex will be merged onto mergeVertex
  Vertex* deadVertex = collapseEdge->getStartVertex();
  Vertex* mergeVertex = collapseEdge->getOpposite()->getStartVertex();

  // Make sure my half edge won't break the mesh
  if(!manifoldLegal(collapseEdge, collapseEdge->getOpposite())){
    std::cout << "Manifold Illegal" << std::endl;
    ignoreVec.push_back(collapseEdge);
    ignoreVec.push_back(collapseEdge->getOpposite());
    return;
  }

  // triangle to delete later
  std::vector<Edge *> triangleToDelete;

  // vector of pairs to save ptr to recreate altered triangles
  std::vector<vPair> triangleToAlter;

  ////////////////////////////////////////////////////////////
  // Removing verticies adj to collapse Edge
  Edge* cur = collapseEdge;

  do{
    // Get the edge to push into triangles to delete
    triangleToDelete.push_back(cur);

    // Get the other two verticies to save to recreate
    vPair aPair;
    bool isChangable = alteredTriPair(cur,deadVertex,mergeVertex, aPair);

    if(isChangable){
      triangleToAlter.push_back(aPair);
    }

    // If I ever encounter a deadend, just delete what I have so far
    cur = cur->getOpposite();
    if(cur == NULL){ 
      std::cout << "Ran into NULL" << std::endl;
      break;
    }

    // Increment
    cur = cur->getNext();

    // sanity check
    assert(cur->getStartVertex() == deadVertex);

  }while( cur != collapseEdge );

  /////////////////////////////////////////////////////////////
  // Removing/Adding

  // Remove all dead triangles 
  for(unsigned int v = 0; v < triangleToDelete.size(); v++){
    removeTriangle(triangleToDelete[v]);
  }
  
  //Recreate new triangles
  for(unsigned int v = 0; v < triangleToAlter.size(); v++){
    //std::cout << "Adding Triangle" <<std::endl;
    
    if( edges.find(std::make_pair(mergeVertex,triangleToAlter[v].first)) != edges.end())
      return;
    if ( edges.find(std::make_pair(triangleToAlter[v].first,triangleToAlter[v].second)) != edges.end())
      return;
    if ( edges.find(std::make_pair(triangleToAlter[v].second,mergeVertex)) != edges.end())
      return;
  
    addTriangle(mergeVertex,triangleToAlter[v].first, triangleToAlter[v].second);
  }


}

bool Mesh::alteredTriPair(Edge* cur, Vertex* deadVertex, Vertex*  mergeVertex, vPair& alteredPair){
  // Input: cur edges, two verticies, and an empty alteredPair
  // Assumptions: Triangle the cur edge resides on exist, all half edges within that triangle exisit
  // Output: True if I should recreate this triangle, false if not
  // Modified: alteried pair has two veritice pointers
  
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
    // Don't include deadVertex
    alteredPair = std::make_pair(b,c);

  }else{
    std::cout << "ERROR 753";
    return false;
  }

  return true;
}


Edge* Mesh::getShortestEdge(){
  // Input: None
  // Assumptions: Hashtable with edges works
  // Output: The Edge with shortest distance between veritices 
  // SideEffect: None

  edgeshashtype::iterator iter = edges.begin();
  double shortest_dist = 100000;
  Edge* shortEdge;

  while(iter != edges.end()){

    // There is no opposite
    if((*iter).second == NULL || (*iter).second->getOpposite() == NULL){
      iter++;
      continue;
    }


    // If it's in the ignore list
    if(std::find(ignoreVec.begin(), ignoreVec.end(),(*iter).second) != ignoreVec.end()){
      iter++;
      continue;
    }
    // Compute the distance
    Vec3f a = ((*iter).first).first->getPos();
    Vec3f b = ((*iter).first).second->getPos();

    double delta_x = pow(a.x() - b.x(), 2.0);
    double delta_y = pow(a.y() - b.y(), 2.0);
    double delta_z = pow(a.z() - b.z(), 2.0);

    double dist = sqrt(delta_x + delta_y + delta_z);

    //Compare distance
    if(dist < shortest_dist){
      shortest_dist = dist;
      shortEdge = (*iter).second;
    }

    iter++;
  }

  std::cout << shortEdge << "\tDistance: " << shortest_dist << std::endl;
  return shortEdge;

}

bool Mesh::manifoldLegal(Edge* a, Edge* b){
  // Input: two verticies I will check for manifold-illegalness
  // Assume: these veritices share an edge
  // Output: true if its safe to remove, false if it isn't
  // Modify: none

  // Storing counts in a map
  std::map<Vertex*, unsigned int> count;

  // Checking those nodes around a
  Edge* cur = a;
  bool reversed = false;

  do{

    // Collecting all verticies` from triangle
    Vertex* one = cur->getTriangle()->getEdge()->getStartVertex();
    Vertex* two = cur->getTriangle()->getEdge()->getNext()->getStartVertex();
    Vertex* thr = cur->getTriangle()->getEdge()->getNext()->getNext()->getStartVertex();

    // Adding those vertices to counter
    (count.find(one) == count.end()) ? count[one] = 1: count[one] += 1;
    (count.find(two) == count.end()) ? count[two] = 1: count[two] += 1;
    (count.find(thr) == count.end()) ? count[thr] = 1: count[thr] += 1;

    // Update cur
    cur = cur->getOpposite();

    // Where to iterate next
    if(cur == NULL && !reversed){
      cur = a->getPrev(); reversed = true;
    }else if(cur == NULL && reversed){
      cur = a;
    }else if(cur != NULL && !reversed){
      cur = cur->getNext();
    }else if(cur!= NULL && reversed){
      cur = cur->getPrev();
    }

  }while(cur != a);

  // Checking those nodes around b
  cur = b;
  reversed = false;
  do{

    // Collecting all verticies` from triangle
    Vertex* one = cur->getTriangle()->getEdge()->getStartVertex();
    Vertex* two = cur->getTriangle()->getEdge()->getNext()->getStartVertex();
    Vertex* thr = cur->getTriangle()->getEdge()->getNext()->getNext()->getStartVertex();

    // Adding those vertices to counter
    (count.find(one) == count.end()) ? count[one] = 1: count[one] += 1;
    (count.find(two) == count.end()) ? count[two] = 1: count[two] += 1;
    (count.find(thr) == count.end()) ? count[thr] = 1: count[thr] += 1;
  
    // Update cur
    cur = cur->getOpposite();

    // Where to iterate next
    if(cur == NULL && !reversed){
      cur = b->getPrev(); reversed = true;
    }else if(cur == NULL && reversed){
      cur = b;
    }else if(cur != NULL && !reversed){
      cur = cur->getNext();
    }else if(cur!= NULL && reversed){
      cur = cur->getPrev();
    }

  }while(cur != b);

  std::map<Vertex*, unsigned int>::iterator iter = count.begin();

  //std::cout << "Vertex a " << a->getStartVertex() << "\tCount: " << count[a->getStartVertex()] <<std::endl;
  //std::cout << "Vertex b " << b->getStartVertex() << "\tCount: " << count[b->getStartVertex()] <<std::endl;
  do{

    //std::cout << "-=-=-=-=-=-=-==\n";
    //std::cout << "Vertex " << (*iter).first << "\tCount: " << (*iter).second <<std::endl;

    // vertex a and b will be there many times
    if((*iter).first != a->getStartVertex() && 
        (*iter).first != b->getStartVertex()){

      // greater then 2 we have a non-manifold
      if((*iter).second >  4){ return false;}

    }

    // Increment iter
    iter++;
  }while(iter != count.end());

  return true;
}

void Mesh::divide(){
  // Purpose: Every triangle in the orginal mesh, divides into 4 new triangles

  sub_division_level ++;

  // Clear out any previous relationships
  // Not the nessary case
  vertex_parents.clear();
  childParentMap.clear();

  // =========================================
  //  Create new triangles
  // =========================================

  triangleshashtype::iterator iter = triangles.begin();
  std::vector<Vertex*> newTriangles;
  std::vector<Triangle*> delTriangles;
  
  while(iter!= triangles.end()){

    // Mark for removal
    delTriangles.push_back((*iter).second);

    Vertex* a = (*iter).second->getEdge()->getStartVertex();
    Vertex* b = (*iter).second->getEdge()->getNext()->getStartVertex();
    Vertex* c = (*iter).second->getEdge()->getNext()->getNext()->getStartVertex();

    Vertex* ab = getChildVertex(a,b);
    Vertex* bc = getChildVertex(b,c);     
    Vertex* ca = getChildVertex(c,a);

    if(ab == NULL){
      ab = addVertex(a->getPos().midPoint3f(b->getPos()));
      setParentsChild(a,b,ab);
      childParentMap[ab] = std::make_pair(a,b);
      creaseMap[std::make_pair(a,ab)] = getMeshEdge(a,b)->getCrease();
      creaseMap[std::make_pair(ab,b)] = getMeshEdge(a,b)->getCrease();
    }

    if(bc == NULL){
      bc = addVertex(b->getPos().midPoint3f(c->getPos()));
      setParentsChild(b,c,bc);
      childParentMap[bc] = std::make_pair(b,c);
      creaseMap[std::make_pair(b,bc)] = getMeshEdge(b,c)->getCrease();
      creaseMap[std::make_pair(bc,c)] = getMeshEdge(b,c)->getCrease();
    }

    if(ca == NULL){
      ca = addVertex(c->getPos().midPoint3f(a->getPos()));
      setParentsChild(c,a,ca);
      childParentMap[ca] = std::make_pair(c,a);
      creaseMap[std::make_pair(c,ca)] = getMeshEdge(a,c)->getCrease();
      creaseMap[std::make_pair(ca,a)] = getMeshEdge(a,c)->getCrease();
    }

    // New Triangles
    newTriangles.push_back(a);
    newTriangles.push_back(ab);
    newTriangles.push_back(ca);

    newTriangles.push_back(b);
    newTriangles.push_back(bc);
    newTriangles.push_back(ab);
    
    newTriangles.push_back(c);
    newTriangles.push_back(ca);
    newTriangles.push_back(bc);

    newTriangles.push_back(ca);
    newTriangles.push_back(ab);
    newTriangles.push_back(bc);
  
    iter++;

  }
 
  // Have everything I need, remove and recreate

  for(unsigned int i = 0; i < delTriangles.size(); i++){
    removeTriangle(delTriangles[i]);
  }
 
  for(unsigned int i = 0; i < newTriangles.size(); i = i + 3){
    addTriangle(newTriangles[i], 
        newTriangles[i+1], 
        newTriangles[i+2]);
  }
}
void Mesh::getControlPts_newEdge(Edge* edg, std::vector<Vec3f> &controlPts){

  // I know this is new.
  
  // Getting control point 1 and 2, WIN
  Vertex* one = childParentMap[edg->getStartVertex()].first;
  Vertex* two = childParentMap[edg->getStartVertex()].second;
  Vertex* three = NULL;
  Vertex* four = NULL;

  Edge* curEdge = edg;

  do{
    // Using the way I build triangles
    // Getting gettingEdge[0] gives me parent
    Vertex* mystery = curEdge->getTriangle()->getEdge()->getStartVertex();
    if(mystery != one && mystery != two){

      // Means that I have one of the middle triangles, lets assert to make sure
      assert(curEdge->getStartVertex() == edg->getStartVertex());
      Vertex * possible = curEdge->getNext()->getOpposite()->getPrev()->getStartVertex();

      if(three == NULL){
        three = possible;

      }else if(three != NULL && three != possible){
        four = possible;
        break;
      }
      
    }

    curEdge = curEdge->getOpposite()->getNext();
  }while(curEdge != edg);


  assert(one != NULL);
  assert(two != NULL);
  assert(three != NULL);
  assert(four != NULL);
  // Apply weights
  Vec3f onePos =  (3/(double)8) * one->getPos();
  Vec3f twoPos =  (3/(double)8) * two->getPos();
  Vec3f threePos = (1/(double)8) * three->getPos();
  Vec3f fourPos =  (1/(double)8) * four->getPos();
  std::cout << onePos << "\n";
  std::cout << twoPos << "\n";
  std::cout << threePos << "\n";
  std::cout << fourPos<< "\n";
  // Save to recalcuate
  controlPts.push_back(onePos);
  controlPts.push_back(twoPos);
  controlPts.push_back(threePos);
  controlPts.push_back(fourPos);
  
}

void Mesh::getControlPts_oldEdge(Edge* edg, std::vector<Vec3f> &controlPts){

  std::vector<Edge*> adjEdges;
  int adjSharp = 0;
  Edge* cur  = edg;

  // Counting how many edges/triangles i have surounding me
  do{

    std::cout << "######TRACE IN GET:###########\n";
    std::cout << cur << std::endl;
    // Legal?
    assert(cur->getStartVertex());
    adjEdges.push_back(cur);
    if(cur->getCrease() != 0)
      adjSharp++;

    // Increment
    cur = cur->getOpposite();
    assert(cur != NULL);
    cur = cur->getNext();
  
  }while(cur != edg);


  // I know how valancy of this vertex, find weight
  double weight;

  if(adjEdges.size() > 3){
    weight = (3 /((double) 8*adjEdges.size()));
  }else if(adjEdges.size() == 3){
    weight = 3/(double)16;
  }


  /*
  // CORNER VERTEX
  if(adjSharp > 2){
    // If I have a corner
    controlPts.push_back(edg->getStartVertex()->getPos());
    return;
  }  
  
  
  // CREASE VERTEX
  if(adjSharp == 2){

    // I have all adj edges
    for(unsigned int i = 0; i < adjEdges.size(); i++){

      //Navigating to old vertex
      Vertex* c = adjEdges[i]->getNext()->getOpposite()->
        getNext()->getOpposite()->getPrev()->getStartVertex();

      if(adjEdges[i]->getCrease() > 0){
        Vec3f temp =  (1/(double)8)* c->getPos();
        controlPts.push_back(temp);
      }

    }

    // Adding in oringal vertex
    controlPts.push_back((6/(double)8) * edg->getStartVertex()->getPos());
    return;
  }
  */

  // I have all adj edges
  for(unsigned int i = 0; i < adjEdges.size(); i++){

    //Navigating to old vertex
    Vertex* c = adjEdges[i]->getNext()->getOpposite()->
      getNext()->getOpposite()->getPrev()->getStartVertex();

    // Saving control points
    Vec3f temp = weight * c->getPos();
    controlPts.push_back(temp);

  }

  // Adding in oringal vertex
  double oldWeight = 1 - (adjEdges.size() * weight);
  controlPts.push_back(oldWeight * edg->getStartVertex()->getPos());


}
void Mesh::getControlPts_newBound(Edge* edg, std::vector<Vec3f> &controlPts){
  std::pair<Vertex*,Vertex*> oldVert;

  Vec3f one;
  Vec3f two;

  oldVert = childParentMap[edg->getStartVertex()];

  one = (0.5) * oldVert.first->getPos();
  two = (0.5) * oldVert.second->getPos();
  
  controlPts.push_back(one);
  controlPts.push_back(two);

  
}

void Mesh::getControlPts_oldBound(Edge* edg, std::vector<Vec3f> &controlPts){
  // Here I run into an slight issue. 
  // I have a vertex, x and want to find orginal vertices o 
  // I dont know how many triangles are around/between
  
  Edge* right = edg;
  Edge* left =  edg;

  while(true){
    
    right = right->getPrev();

    if(right->getOpposite() != NULL){
      right = right->getOpposite();
      
    }else{
      right = right->getNext();
      break;
    
    }
  }//Finding right most
    
  while(true){

    if(left->getOpposite() != NULL){
      left = left->getOpposite();
      left = left->getNext();
    
    }else{
      left = left->getPrev();
      break;
    }

  }//Finding left most

  Vec3f one = right->getNext()->getOpposite()->getPrev()->getOpposite()->getPrev()->getStartVertex()->getPos();
  Vec3f two = left->getPrev()->getOpposite()->getNext()->getOpposite()->getPrev()->getStartVertex()->getPos();

  one = one * (1/(double)8);
  two = two * (1/(double)8);
  Vec3f org = (3/(double)4) * edg->getStartVertex()->getPos();

  controlPts.push_back(one);
  controlPts.push_back(two);
  controlPts.push_back(org);
  
}

int Mesh::identifyVertex(Edge * edg){

  Vertex* mysteryVertex = edg->getStartVertex();

  if(childParentMap.count(mysteryVertex) == 1){
  
    // I know I have a child, easy to find if boundry
  
    Edge* cur  = edg;

    // Counting how many edges/triangles i have surounding me
    do{
      // Increment
      cur = cur->getOpposite();
      if(cur == NULL){
        return 3;
      }
      cur = cur->getNext();
    
    }while(cur != edg);

    return 1;
  }else{

    Edge* cur  = edg;

    // Counting how many edges/triangles i have surounding me
    do{
      // Increment
      cur = cur->getOpposite();
      if(cur == NULL){
        return 4;
      }
      cur = cur->getNext();
    
    }while(cur != edg);

    return 2;
  }

  //std::cout << "Problems!\n";
  return 0;

  /*
  // THIS WORKS FOR MOST CASES, NOT BOUNDS THOU!
  Edge* curEdge = edge;

  do{

    curEdge = curEdge->getOpposite();
    if(curEdge == NULL){
      return 0;
    }else{
      curEdge = curEdge->getNext();
    }

  }while(curEdge != edge);



    //TODO Only works for first iteration
    if(childParentMap.count(mysteryVertex) == 1){
      return 1;
    }else{
      return 2;
    }
    

  return 0;

  */
}
// =================================================================
