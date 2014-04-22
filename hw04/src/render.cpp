#include <iostream>

#include "glCanvas.h"
#include "mesh.h"
#include "edge.h"
#include "vertex.h"
#include "triangle.h"
#include "argparser.h"
#include "utils.h"
#include <list>
#include <algorithm>
#include <math.h>       /* fabs */
#include <stdio.h>      /* printf */

// Predefined colors to use
glm::vec4 floor_color(0.9,0.8,0.7,1);
glm::vec4 mesh_color(0.8,0.8,0.8,1);
glm::vec4 mirror_color(0.1,0.1,0.2,1);
glm::vec4 mirror_tint(0.85,0.9,0.95,1);

glm::vec4 red(1.0,0,0,1);
glm::vec4 green(0,1,0,0.5);

float floor_factor = 0.75;

// =======================================================================
// =======================================================================


// the light position can be animated
glm::vec3 Mesh::LightPosition() const {
  glm::vec3 min = bbox.getMin();
  glm::vec3 max = bbox.getMax();
  glm::vec3 tmp;
  bbox.getCenter(tmp);
  tmp += glm::vec3(0,1.5*(max.y-min.y),0);
  tmp += glm::vec3(cos(args->timer) * (max.x-min.x), 0, 0);
  tmp += glm::vec3(0,0,sin(args->timer) * (max.z-min.z));
  return tmp;
}


void Mesh::initializeVBOs() {

  // Regular mesh buffer
  glGenBuffers(1,&mesh_tri_verts_VBO);
  glGenBuffers(1,&mesh_tri_indices_VBO);

  // Relfection mesh buffers
  glGenBuffers(1,&reflected_mesh_tri_verts_VBO);
  glGenBuffers(1,&reflected_mesh_tri_indices_VBO);

  // Shadow buffers
  glGenBuffers(1,&shadow_polygon_tri_verts_VBO);
  glGenBuffers(1,&shadow_polygon_tri_indices_VBO);

  // Mirror buffers
  glGenBuffers(1,&mirror_tri_verts_VBO);
  glGenBuffers(1,&mirror_tri_indices_VBO);

  // Floor triangles
  glGenBuffers(1,&floor_tri_verts_VBO);
  glGenBuffers(1,&floor_tri_indices_VBO);

  // Reflected floor
  glGenBuffers(1,&reflected_floor_tri_verts_VBO);
  glGenBuffers(1,&reflected_floor_tri_indices_VBO);

  // Silhouette buffers
  glGenBuffers(1,&silhouette_edge_tri_verts_VBO);
  glGenBuffers(1,&silhouette_edge_tri_indices_VBO);

  // Light buffer
  glGenBuffers(1,&light_vert_VBO);
  bbox.initializeVBOs();
}

void Mesh::cleanupVBOs() {
  glDeleteBuffers(1,&mesh_tri_verts_VBO);
  glDeleteBuffers(1,&mesh_tri_indices_VBO);
  glDeleteBuffers(1,&reflected_mesh_tri_verts_VBO);
  glDeleteBuffers(1,&reflected_mesh_tri_indices_VBO);
  glDeleteBuffers(1,&shadow_polygon_tri_verts_VBO);
  glDeleteBuffers(1,&shadow_polygon_tri_indices_VBO);
  glDeleteBuffers(1,&mirror_tri_verts_VBO);
  glDeleteBuffers(1,&mirror_tri_indices_VBO);
  glDeleteBuffers(1,&floor_tri_verts_VBO);
  glDeleteBuffers(1,&floor_tri_indices_VBO);
  glDeleteBuffers(1,&reflected_floor_tri_verts_VBO);
  glDeleteBuffers(1,&reflected_floor_tri_indices_VBO);
  glDeleteBuffers(1,&silhouette_edge_tri_verts_VBO);
  glDeleteBuffers(1,&silhouette_edge_tri_indices_VBO);
  glDeleteBuffers(1,&light_vert_VBO);
  bbox.cleanupVBOs();
}

// ================================================================================
// ================================================================================

void Mesh::SetupLight(const glm::vec3 &light_position) {
  light_vert.push_back(VBOPosNormalColor(light_position,glm::vec3(1,0,0),glm::vec4(1,1,0,0)));
  glBindBuffer(GL_ARRAY_BUFFER,light_vert_VBO); 
  glBufferData(GL_ARRAY_BUFFER,sizeof(VBOPosNormalColor)*1,&light_vert[0],GL_STATIC_DRAW); 
}


void Mesh::SetupMirror() {
  glm::vec3 diff = bbox.getMax()-bbox.getMin();
  // create frame vertices just a bit bigger than the bounding box
  glm::vec3 a = bbox.getMin() + glm::vec3(-0.25*diff.x, 0.1*diff.y,-0.25*diff.z);
  glm::vec3 b = bbox.getMin() + glm::vec3(-0.25*diff.x, 1.25*diff.y,-0.25*diff.z);
  glm::vec3 c = bbox.getMin() + glm::vec3(-0.25*diff.x, 1.25*diff.y, 1.25*diff.z);
  glm::vec3 d = bbox.getMin() + glm::vec3(-0.25*diff.x, 0.1*diff.y, 1.25*diff.z);
  glm::vec3 normal = ComputeNormal(a,c,b);
  glm::vec4 color(0.1,0.1,0.1,1);

  // OBJ Style
  mirror_tri_verts.push_back(VBOPosNormalColor(a,normal,mirror_color));
  mirror_tri_verts.push_back(VBOPosNormalColor(b,normal,mirror_color));
  mirror_tri_verts.push_back(VBOPosNormalColor(c,normal,mirror_color));
  mirror_tri_verts.push_back(VBOPosNormalColor(d,normal,mirror_color));
  mirror_tri_indices.push_back(VBOIndexedTri(0,1,2));
  mirror_tri_indices.push_back(VBOIndexedTri(0,2,3));


  //glBindBuffer
  glBindBuffer(GL_ARRAY_BUFFER,mirror_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColor) * mirror_tri_verts.size(), 
	       &mirror_tri_verts[0],
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mirror_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedTri) * mirror_tri_indices.size(),
	       &mirror_tri_indices[0], GL_STATIC_DRAW);
}


void Mesh::SetupFloor() {
  glm::vec3 diff = bbox.getMax()-bbox.getMin();
  // create vertices just a bit bigger than the bounding box
  glm::vec3 a = bbox.getMin() + glm::vec3(-floor_factor*diff.x,0,-floor_factor*diff.z);
  glm::vec3 b = bbox.getMin() + glm::vec3(-floor_factor*diff.x,0, (1+floor_factor)*diff.z);
  glm::vec3 c = bbox.getMin() + glm::vec3( (1+floor_factor)*diff.x,0, (1+floor_factor)*diff.z);
  glm::vec3 d = bbox.getMin() + glm::vec3( (1+floor_factor)*diff.x,0,-floor_factor*diff.z);
  glm::vec3 normal = ComputeNormal(a,c,d);
  floor_tri_verts.push_back(VBOPosNormalColor(a,normal,floor_color));
  floor_tri_verts.push_back(VBOPosNormalColor(b,normal,floor_color));
  floor_tri_verts.push_back(VBOPosNormalColor(c,normal,floor_color));
  floor_tri_verts.push_back(VBOPosNormalColor(d,normal,floor_color));
  floor_tri_indices.push_back(VBOIndexedTri(0,1,2));
  floor_tri_indices.push_back(VBOIndexedTri(0,2,3));
  glBindBuffer(GL_ARRAY_BUFFER,floor_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColor) * floor_tri_verts.size(), 
	       &floor_tri_verts[0],
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,floor_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedTri) * floor_tri_indices.size(),
	       &floor_tri_indices[0], GL_STATIC_DRAW);
}


void Mesh::SetupMesh() {
  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;
    glm::vec3 a = (*t)[0]->getPos();
    glm::vec3 b = (*t)[1]->getPos();
    glm::vec3 c = (*t)[2]->getPos();    
    glm::vec3 na = ComputeNormal(a,b,c);
    glm::vec3 nb = na;
    glm::vec3 nc = na;
    if (args->gouraud_normals) {
      na = (*t)[0]->getGouraudNormal();
      nb = (*t)[1]->getGouraudNormal();
      nc = (*t)[2]->getGouraudNormal();
    }
    int start = mesh_tri_verts.size();
    mesh_tri_verts.push_back(VBOPosNormalColor(a,na,mesh_color));
    mesh_tri_verts.push_back(VBOPosNormalColor(b,nb,mesh_color));
    mesh_tri_verts.push_back(VBOPosNormalColor(c,nc,mesh_color));
    mesh_tri_indices.push_back(VBOIndexedTri(start,start+1,start+2));
  }
  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColor) * mesh_tri_verts.size(), 
	       &mesh_tri_verts[0],
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedTri) * mesh_tri_indices.size(),
	       &mesh_tri_indices[0], GL_STATIC_DRAW);
}


// draw a second copy of the object where it appears to be on the other side of the mirror
void Mesh::SetupReflectedMesh() {

  // Setup some stuff
  glm::vec3 diff = bbox.getMax()-bbox.getMin();

  float mirror_x = (-0.25)*diff.x + bbox.getMin().x;

  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;
    
    glm::vec3 a = (*t)[0]->getPos();
    glm::vec3 b = (*t)[1]->getPos();
    glm::vec3 c = (*t)[2]->getPos();    


    glm::vec3 * v[3] = {&a,&b,&c};
    
    for(int i = 0; i < 3; i++){
      //translate
      float dist_2_mirror = sqrt(pow((v[i]->x - mirror_x),2));
      v[i]->x = v[i]->x - (2*dist_2_mirror);
    }


    // Alter order
    glm::vec3 na = ComputeNormal(c,b,a);
    glm::vec3 nb = na;
    glm::vec3 nc = na;
    if (args->gouraud_normals) {
      na = (*t)[0]->getGouraudNormal();
      nb = (*t)[1]->getGouraudNormal();
      nc = (*t)[2]->getGouraudNormal();
    }
    // Alter order
    int start = reflected_mesh_tri_verts.size();
    reflected_mesh_tri_verts.push_back(VBOPosNormalColor(c,nc,mesh_color));
    reflected_mesh_tri_verts.push_back(VBOPosNormalColor(b,nb,mesh_color));
    reflected_mesh_tri_verts.push_back(VBOPosNormalColor(a,na,mesh_color));
    reflected_mesh_tri_indices.push_back(VBOIndexedTri(start,start+1,start+2));
  }
  
  // After pushing
  glBindBuffer(GL_ARRAY_BUFFER,reflected_mesh_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColor) * reflected_mesh_tri_verts.size(), 
	       &reflected_mesh_tri_verts[0],
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,reflected_mesh_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedTri) * reflected_mesh_tri_indices.size(),
	       &reflected_mesh_tri_indices[0], GL_STATIC_DRAW);



}


// draw a second copy of the floor where it appears to be on the other side of the mirror
void Mesh::SetupReflectedFloor() {


  glm::vec3 diff = bbox.getMax()-bbox.getMin();
  float mirror_x = (-0.25)*diff.x + bbox.getMin().x;
  // create vertices just a bit bigger than the bounding box
  glm::vec3 a = bbox.getMin() + glm::vec3(-floor_factor*diff.x,0,-floor_factor*diff.z);
  glm::vec3 b = bbox.getMin() + glm::vec3(-floor_factor*diff.x,0, (1+floor_factor)*diff.z);
  glm::vec3 c = bbox.getMin() + glm::vec3( (1+floor_factor)*diff.x,0, (1+floor_factor)*diff.z);
  glm::vec3 d = bbox.getMin() + glm::vec3( (1+floor_factor)*diff.x,0,-floor_factor*diff.z);

    glm::vec3 * v[4] = {&a,&b,&c,&d};
    
    for(int i = 0; i < 4; i++){
      //translate
      float dist_2_mirror = sqrt(pow((v[i]->x - mirror_x),2));
      if(v[i]->x < mirror_x){
      v[i]->x = v[i]->x + (2*dist_2_mirror);
      
      }else{
      v[i]->x = v[i]->x - (2*dist_2_mirror);
      
      }
    }

  glm::vec3 normal = ComputeNormal(d,c,a);
  reflected_floor_tri_verts.push_back(VBOPosNormalColor(a,normal,floor_color));
  reflected_floor_tri_verts.push_back(VBOPosNormalColor(b,normal,floor_color));
  reflected_floor_tri_verts.push_back(VBOPosNormalColor(c,normal,floor_color));
  reflected_floor_tri_verts.push_back(VBOPosNormalColor(d,normal,floor_color));
  reflected_floor_tri_indices.push_back(VBOIndexedTri(2,1,0));
  reflected_floor_tri_indices.push_back(VBOIndexedTri(3,2,0));
  glBindBuffer(GL_ARRAY_BUFFER,reflected_floor_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColor) * reflected_floor_tri_verts.size(), 
	       &reflected_floor_tri_verts[0],
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,reflected_floor_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedTri) * reflected_floor_tri_indices.size(),
	       &reflected_floor_tri_indices[0], GL_STATIC_DRAW);

  // ASSIGNMENT: WRITE THIS FUNCTION


}


// figure out which edges are the silhouette of the object 
void Mesh::SetupSilhouetteEdges(const glm::vec3 &light_position) {

  std::cout << "running" << std::endl;
  std::list<Edge *> silhouette;

  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;

    glm::vec3 a = (*t)[0]->getPos();
    glm::vec3 b = (*t)[1]->getPos();
    glm::vec3 c = (*t)[2]->getPos();    
    
    // Getting center of the triangle
    glm::vec3 center = (*t).getCenter();

    // Getitng light direction from light to center
    glm::vec3 lightDir =  light_position - center;

    if(glm::dot(lightDir, ComputeNormal(a,b,c)) >= 0.0){
      // ploygon faces away from light source
      Edge * ea  = (*t).getEdge();
      Edge * eb  = (*t).getEdge()->getNext();
      Edge * ec  = (*t).getEdge()->getNext()->getNext();

      Edge * edges[3] = {ea,eb,ec};

      // for each edge check if I am in the list
      for(int i=0; i < 3; i++){


        std::list<Edge*>::iterator found = std::find(
            silhouette.begin(),silhouette.end(),edges[i]->getOpposite());

        if(found != silhouette.end()){
          //alread in list
          // remove
          silhouette.erase(found);
        }else{
          //add to list
          silhouette.push_back(edges[i]);
        }
      
      }//each edge
    }//if_face
  }//each tri

  std::cout << "all computed" << std::endl;
  printf("DEBUG: silhouette %d\n", silhouette.size());

  float thickness = 0.001*getBoundingBox().maxDim();
  

  // for each edge
  std::list<Edge*>::iterator cur = silhouette.begin();
  for(;cur != silhouette.end(); cur++){
    glm::vec3 v1 = (*cur)->getStartVertex()->getPos();
    glm::vec3 v2 = (*cur)->getEndVertex()->getPos();

    //Pushing back for SetupShadowPolygons
    extend_edges.push_back(*cur);

    //Making lines visualizations
    addEdgeGeometry(
        silhouette_edge_tri_verts,silhouette_edge_tri_indices,
        v1,v2,red,red,thickness,thickness);
    //save for later
  }
  

  glBindBuffer(GL_ARRAY_BUFFER,silhouette_edge_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,sizeof(VBOPosNormalColor)*silhouette_edge_tri_verts.size(),&silhouette_edge_tri_verts[0],GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,silhouette_edge_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(VBOIndexedTri)*silhouette_edge_tri_indices.size(),&silhouette_edge_tri_indices[0],GL_STATIC_DRAW);

}


// project the silhouette edges away from the light source
void Mesh::SetupShadowPolygons(const glm::vec3 &light_position) {
  //jump
  
  // Check
  assert(extend_edges.size() != 0);

  // constants
  float thickness = 0.001*getBoundingBox().maxDim();
  float large_number = 3;

  //for each edge
  for(int i = 0; i < extend_edges.size(); i++){

    //getting vectors
    glm::vec3 v1 =  extend_edges[i]->getStartVertex()->getPos();
    glm::vec3 v2 =  extend_edges[i]->getEndVertex()->getPos();

    glm::vec3 lightDir_v1 = v1 - light_position;
    glm::vec3 lightDir_v2 = v2 - light_position;

    lightDir_v1  = glm::normalize(lightDir_v1);
    lightDir_v2  = glm::normalize(lightDir_v2);

    glm::vec3 projected_v1 = v1 + lightDir_v1 * large_number;
    glm::vec3 projected_v2 = v2 + lightDir_v2 * large_number;

    //clock wise correct
    glm::vec3 normal = ComputeNormal(light_position,v1,v2);

    //counter-clockwise
    //glm::vec3 normal = ComputeNormal(light_position,v2,v1);


    //count clock wise
    int start = shadow_polygon_tri_verts.size();
    shadow_polygon_tri_verts.push_back(VBOPosNormalColor(v1,normal,green));
    shadow_polygon_tri_verts.push_back(VBOPosNormalColor(v2,normal,green));
    shadow_polygon_tri_verts.push_back(VBOPosNormalColor(projected_v1,normal,green));
    shadow_polygon_tri_verts.push_back(VBOPosNormalColor(projected_v2,normal,green));
    //shadow_polygon_tri_verts.push_back(VBOPosNormalColor(light_position,normal,green));

    //clock wise
    //shadow_polygon_tri_indices.push_back(VBOIndexedTri(start,start+1,start+4));
    shadow_polygon_tri_indices.push_back(VBOIndexedTri(start,start+2,start+3));
    shadow_polygon_tri_indices.push_back(VBOIndexedTri(start+1,start,start+3));


  }
  // ASSIGNMENT: WRITE THIS FUNCTION
  
  glBindBuffer(GL_ARRAY_BUFFER,shadow_polygon_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColor) * shadow_polygon_tri_verts.size(), 
	       &shadow_polygon_tri_verts[0],GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,shadow_polygon_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedTri) * shadow_polygon_tri_indices.size(),
	       &shadow_polygon_tri_indices[0], GL_STATIC_DRAW);

}

// ================================================================================
// ================================================================================

void Mesh::DrawLight() {
  HandleGLError("enter draw mirror");
  glPointSize(10);
  glBindBuffer(GL_ARRAY_BUFFER, light_vert_VBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
  glDrawArrays(GL_POINTS, 0, light_vert.size());
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  HandleGLError("enter draw mirror");
}

void Mesh::DrawMirror() {
  HandleGLError("enter draw mirror");
  glBindBuffer(GL_ARRAY_BUFFER,mirror_tri_verts_VBO); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mirror_tri_indices_VBO); 
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
  glDrawElements(GL_TRIANGLES, mirror_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  HandleGLError("leaving draw mirror");
}

void Mesh::DrawFloor() {
  HandleGLError("enter draw floor");
  glBindBuffer(GL_ARRAY_BUFFER,floor_tri_verts_VBO); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,floor_tri_indices_VBO); 
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
  glDrawElements(GL_TRIANGLES, floor_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  HandleGLError("leaving draw floor");
}

void Mesh::DrawMesh() {
  HandleGLError("enter draw mesh");
  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_verts_VBO); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_tri_indices_VBO); 
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
  glDrawElements(GL_TRIANGLES, mesh_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  HandleGLError("leaving draw mesh");
}

void Mesh::DrawReflectedFloor() {
  if (reflected_floor_tri_verts.size() > 0) {
    HandleGLError("enter draw reflected_floor");
    glBindBuffer(GL_ARRAY_BUFFER,reflected_floor_tri_verts_VBO); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,reflected_floor_tri_indices_VBO); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
    glDrawElements(GL_TRIANGLES, reflected_floor_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    HandleGLError("leaving draw reflected_floor");
  }
}

void Mesh::DrawReflectedMesh() {
  if (reflected_mesh_tri_verts.size() > 0) {
    HandleGLError("enter draw reflected_mesh");

    glBindBuffer(GL_ARRAY_BUFFER,reflected_mesh_tri_verts_VBO); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,reflected_mesh_tri_indices_VBO); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
    glDrawElements(GL_TRIANGLES, reflected_mesh_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    HandleGLError("leaving draw reflected_mesh");
  }
}

void Mesh::DrawSilhouetteEdges() {
  if (silhouette_edge_tri_verts.size() > 0) {
    HandleGLError("enter draw silhouette_edge");
    glBindBuffer(GL_ARRAY_BUFFER,silhouette_edge_tri_verts_VBO); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,silhouette_edge_tri_indices_VBO); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
    glDrawElements(GL_TRIANGLES, silhouette_edge_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    HandleGLError("leaving draw silhouette_edge");
  }
}

void Mesh::DrawShadowPolygons() {
  if (shadow_polygon_tri_verts.size() > 0) {
    HandleGLError("enter draw silhouette_edge");
    glBindBuffer(GL_ARRAY_BUFFER,shadow_polygon_tri_verts_VBO); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,shadow_polygon_tri_indices_VBO); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
    glDrawElements(GL_TRIANGLES, shadow_polygon_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    HandleGLError("leaving draw silhouette_edge");
  }
}

// ======================================================================================
// ======================================================================================

void Mesh::setupVBOs() {
  // delete all the old geometry
  mesh_tri_verts.clear(); 
  mesh_tri_indices.clear();
  reflected_mesh_tri_verts.clear(); 
  reflected_mesh_tri_indices.clear();
  shadow_polygon_tri_verts.clear(); 
  shadow_polygon_tri_indices.clear();
  mirror_tri_verts.clear(); 
  mirror_tri_indices.clear();
  floor_tri_verts.clear(); 
  floor_tri_indices.clear();
  reflected_floor_tri_verts.clear(); 
  reflected_floor_tri_indices.clear();
  silhouette_edge_tri_verts.clear(); 
  silhouette_edge_tri_indices.clear();
  light_vert.clear();

  //edit
  extend_edges.clear();
  

  // setup the new geometry
  glm::vec3 light_position = LightPosition();
  SetupLight(light_position);
  SetupMirror();
  SetupFloor();
  SetupMesh();
  SetupReflectedMesh();
  SetupReflectedFloor();
  SetupSilhouetteEdges(light_position);
  SetupShadowPolygons(light_position);
  bbox.setupVBOs();
}

void Mesh::drawVBOs() {


  // mode 1: STANDARD PHONG LIGHTING (LIGHT ON)
  glUniform1i(GLCanvas::colormodeID, 1);

  // shader 0: NO SHADER
  glUniform1i(GLCanvas::whichshaderID, 0);


  HandleGLError("enter draw vbos");
  // --------------------------
  // NEITHER SHADOWS NOR MIRROR
  if (!args->mirror && !args->shadow) {
    DrawMirror();
    DrawFloor();
    if (args->geometry) {
      // shader 1: CHECKERBOARD
      // shader 2: ORANGE
      // shader 3: other
      glUniform1i(GLCanvas::whichshaderID, args->whichshader);
      DrawMesh();
      glUniform1i(GLCanvas::whichshaderID, 0);
    }
  } 

  // ---------------------
  // MIRROR ONLY RENDERING
  else if (args->mirror && !args->shadow) {
    // Clear frame, depth & stencil buffers
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Draw all non-mirror geometry to frame & depth buffers
    DrawMesh();
    DrawFloor();
    // draw back of mirror (if we can see the back)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    DrawMirror();     
    glDisable(GL_CULL_FACE);

    // Draw mirror to stencil buffer, where depth buffer passes
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    glStencilFunc(GL_ALWAYS,1,~0); 
    // (only draw the mirror if we can see the front)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    DrawMirror();     
    glDisable(GL_CULL_FACE);
    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE); // disable frame buffer writes      
    glDepthRange(1,1);
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_EQUAL,1,~0); 
    glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
    DrawMirror();     

    // Set depth to infinity, where stencil buffer passes
    glDepthFunc(GL_LESS);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); // enable frame buffer writes
    glDepthRange(0,1);

    // Draw reflected geometry to frame & depth buffer, where stencil buffer passes
    DrawReflectedMesh();
    DrawReflectedFloor();

    glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE); // disable frame buffer writes      
    glStencilOp(GL_KEEP,GL_KEEP,GL_ZERO);
    glDepthFunc(GL_ALWAYS);
    DrawMirror();
    glDepthFunc(GL_LESS);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); // enable frame buffer writes    

    glDisable(GL_STENCIL_TEST);
  } 


  // ---------------------
  // SHADOW ONLY RENDERING
  else if (!args->mirror && args->shadow) {
    
    // Clear frame, depth & stencil buffers, disable stencil
    // enable light

    // Light on
    glUniform1i(GLCanvas::colormodeID, 1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glColorMask(1,1,1,1); 
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw scene
    DrawMesh();
    DrawFloor();

    // Turn light off
    glUniform1i(GLCanvas::colormodeID, 2);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(0);
    // do not disturb the depth buffer
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilMask(0x1);
    // just write least significant
    // stencil bit

    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    // invert stencil bit if depth pass
    glColorMask(0,0,0,0);
    // do not disturb the color buffer

    // shadow polygons
    glDisable(GL_CULL_FACE);
    DrawShadowPolygons();
    glEnable(GL_CULL_FACE);



    glEnable(GL_LIGHTING);
    // use lighting
    glDisable(GL_LIGHT0);
    // just not the shadowed light
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);
    // must match depth from 1st step
    glDepthMask(0);

    glEnable(GL_STENCIL_TEST);
    // and use stencil to update only
    glStencilFunc(GL_EQUAL, 0x1, 0x1);
    // pixels tagged as
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // “in the shadow volume”
    glColorMask(1,1,1,1);
    DrawMesh();
    DrawFloor();


  // ASSIGNMENT: WRITE THIS RENDERING MODE


  // use the following code to turn the lights on & off
  // ... instead of glEnable(GL_LIGHTS), etc.
  // mode 2: AMBIENT ONLY (LIGHT OFF) 
  //glUniform1i(GLCanvas::colormodeID, 2);
  // mode 1: STANDARD PHONG LIGHTING (LIGHT ON)
  //glUniform1i(GLCanvas::colormodeID, 1);





  }

  // --------------------------
  // MIRROR & SHADOW!
  else {
    assert (args->mirror && args->shadow);




    // EXTRA CREDIT: IMPLEMENT THIS INTERACTION




  }


  // -------------------------
  // ADDITIONAL VISUALIZATIONS (for debugging)
  if (args->reflected_geometry) {
    DrawReflectedMesh();
    DrawReflectedFloor();
  }

  // mode 0: NO LIGHTING
  glUniform1i(GLCanvas::colormodeID, 0);

  DrawLight();
  
  if (args->bounding_box) {
    bbox.drawVBOs();
  }
  if (args->silhouette_edges) {
    DrawSilhouetteEdges();
  }

  if (args->shadow_polygons) {

    // FIXME (not part of assignment): shadow polygons are currently
    //   visualized opaque.  in glut version they are transparent (and
    //   probably more helpful).  will eventually port/debug this.
    
    //glDisable(GL_LIGHTING);
    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    //glEnable(GL_BLEND);
    //glDepthMask(GL_FALSE);
    //glColor4f(0,1,0.5,0.2);
    //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    DrawShadowPolygons();
    //glDepthMask(GL_TRUE);
    //glDisable(GL_BLEND);
    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    //glEnable(GL_LIGHTING);
  }

  HandleGLError(); 
}

// =================================================================
