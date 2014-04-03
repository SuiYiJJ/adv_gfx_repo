#include "glCanvas.h"

#include "radiosity.h"
#include "mesh.h"
#include "face.h"
#include "glCanvas.h"
#include "sphere.h"
#include "raytree.h"
#include "raytracer.h"
#include "utils.h"
#include <stdio.h>
#include <float.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef EPSILON
#define EPSILON .0001
#endif
// ================================================================
// CONSTRUCTOR & DESTRUCTOR
// ================================================================
Radiosity::Radiosity(Mesh *m, ArgParser *a) {
  mesh = m;
  args = a;
  num_faces = -1;  
  formfactors = NULL;
  area = NULL;
  undistributed = NULL;
  absorbed = NULL;
  radiance = NULL;
  ambient = 0;
  max_undistributed_patch = -1;
  total_area = -1;
  Reset();
}

Radiosity::~Radiosity() {
  Cleanup();
  cleanupVBOs();
}

void Radiosity::Cleanup() {
  delete [] formfactors;
  delete [] area;
  delete [] undistributed;
  delete [] absorbed;
  delete [] radiance;
  num_faces = -1;
  formfactors = NULL;
  area = NULL;
  undistributed = NULL;
  absorbed = NULL;
  radiance = NULL;
  max_undistributed_patch = -1;
  total_area = -1;
}

void Radiosity::Reset() {
  delete [] area;
  delete [] undistributed;
  delete [] absorbed;
  delete [] radiance;

  // create and fill the data structures
  num_faces = mesh->numFaces();
  area = new double[num_faces];
  undistributed = new Vec3f[num_faces];
  absorbed = new Vec3f[num_faces];
  radiance = new Vec3f[num_faces];
  for (int i = 0; i < num_faces; i++) {
    Face *f = mesh->getFace(i);
    f->setRadiosityPatchIndex(i);
    setArea(i,f->getArea());
    Vec3f emit = f->getMaterial()->getEmittedColor();
    setUndistributed(i,emit);
    setAbsorbed(i,Vec3f(0,0,0));
    setRadiance(i,emit);
  }

  // find the patch with the most undistributed energy
  findMaxUndistributed();
}


// =======================================================================================
// =======================================================================================

void Radiosity::findMaxUndistributed() {
  // find the patch with the most undistributed energy 
  // don't forget that the patches may have different sizes!
  // Edit: return's max index
  max_undistributed_patch = -1;
  total_undistributed = 0;
  total_area = 0;
  double max = -1;
  for (int i = 0; i < num_faces; i++) {
    double m = getUndistributed(i).Length() * getArea(i);
    total_undistributed += m;
    total_area += getArea(i);
    if (max < m) {
      max = m;
      max_undistributed_patch = i;
    }
  }
  assert (max_undistributed_patch >= 0 && max_undistributed_patch < num_faces);
}


void Radiosity::ComputeFormFactors() {

  // Barb's code
  assert (formfactors == NULL);
  assert (num_faces > 0);
  formfactors = new double[num_faces*num_faces];
  findMaxUndistributed();

  // keep track of which form factor I am on.
  unsigned int index = 0;

  // ? are these triangles or quads? I think quads

  // For each of the patches
  for(int i = 0; i < num_faces; i ++){

    // Getting the patch i
    Face * patch_i = mesh->getFace(i);
    Vec3f normal_i = patch_i->computeNormal(); //this might be wrong direction


    // For each other patch
    for(int j = 0; j < num_faces; j ++){

      // Getting the patch j
      Face * patch_j = mesh->getFace(j);
      Vec3f normal_j = patch_j->computeNormal();  //this might be wrong direction


      // ray from i->j these are correct directions
      Vec3f direct_to_j = patch_j->computeCentroid() - patch_i->computeCentroid();
      Vec3f direct_to_i = patch_i->computeCentroid() - patch_j->computeCentroid();
      direct_to_j.Normalize();
      direct_to_i.Normalize();
      Ray ray_ij(patch_i->computeCentroid(), direct_to_j);



      double angle_i,angle_j;
      // Getting the angles between normals fixing
      // issue with normals facing weird directions

      angle_i = normal_i.AngleBetween(direct_to_j);
      angle_j = normal_j.AngleBetween(direct_to_i);



      double distance = patch_i->computeCentroid().Distance3f(patch_j->computeCentroid());

      // Assuming visablity is 1
      double visablity = 0;
      // Todo implement shadows on form factors


      if(args->num_shadow_samples >= 1){
      	// visablity calcs
      	int hit_count = 0;

      	for(int r = 0; r < args->num_form_factor_samples; r++){
      		//printf("Shooting shadow ray\n" );
      		// get random points on patch j
      		Vec3f rand_patch_i = patch_i->RandomPoint();
      		Vec3f rand_patch_j = patch_j->RandomPoint();

      		// get the direction vector
      		Vec3f dir_ray = rand_patch_j - rand_patch_i;
      		dir_ray.Normalize();

      		// Make ray
      		Ray freedom(rand_patch_i, dir_ray);
      		Hit justice;

      		// Trace ray + incr if I hit something
      		if(raytracer->CastRay(freedom,justice,true)){
      			Vec3f hitPoint = freedom.pointAtParameter(justice.getT());
      			//std::cout << "========================================\n";
      			//std::cout << "Hit: " << hitPoint << "Aiming for " << rand_patch_j;
      			// Did i hit near my location?
      			double dist_diff = rand_patch_j.Distance3f(hitPoint);
      			//std::cout << "Missed by: " << dist_diff << std::endl;
      			if(dist_diff <= EPSILON){hit_count++;}
      		}
      	}//for

      	// Assign a visablity
      	//printf("Hit count: %d\tOut of %d\n",hit_count,args->num_form_factor_samples);
      	visablity = (double)hit_count / args->num_form_factor_samples;
      	assert(0 <= visablity && visablity <= 1 );

      }else{
	      visablity  = 1.0;
      }

      // From class
      //double formFac = (cos(angle_i) * cos(angle_j) * visablity * patch_j->getArea()) / (M_PI* distance*distance);

      // (Wallace et al. 1989)
      double formFac = (cos(angle_i) * cos(angle_j) * visablity * patch_j->getArea()) / ((M_PI * distance*distance) + patch_j->getArea());

      // Fixing my hack
      if(cos(angle_i) * cos(angle_j) < 0 && visablity != 0){
      	formFac = 0;
      }

      formFac = fabs(formFac); // the key is to have an even number or errors


      setFormFactor(i,j,formFac);
      // Printing those who's normal angles are inverse

      //Visalization
      if(max_undistributed_patch == i){
        Hit hit;
        if(raytracer->CastRay(ray_ij,hit,false)){
          RayTree::AddShadowSegment(ray_ij,0,hit.getT()/2);
        }else{
          RayTree::AddMainSegment(ray_ij,0,.5);
        }
        // Normals
        Ray normalRay(patch_j->computeCentroid(), normal_j);
        RayTree::AddTransmittedSegment(normalRay,0,.2);

        // Backwards Ray
        Vec3f direct_to_i = patch_i->computeCentroid() - patch_j->computeCentroid();
        direct_to_i.Normalize();
        hit = Hit();
        Ray ray_ji(patch_j->computeCentroid(), direct_to_i);
        raytracer->CastRay(ray_ji,hit,false);
        RayTree::AddReflectedSegment(ray_ji,0,hit.getT()/2);
      }

      index++;
    }
  }
}

// ================================================================
// ================================================================

// jump
double Radiosity::Iterate() {

	// Set ambient light back so I have real data
	if(args->ambient_term){
	  for(int i = 0; i <num_faces;i++){
	  	//convert ambient_total to grey
	  	Vec3f dif = mesh->getFace(i)->getMaterial()->getDiffuseColor();
	  	Vec3f amb_dif = ambient * dif;
	  	setRadiance(i,getRadiance(i) - amb_dif);
	  }
	}

  Vec3f white(1.0,1.0,1.0);
  // Set up the form factors will only run once
  if (formfactors == NULL)
    ComputeFormFactors();
  assert (formfactors != NULL);

  // Update for the brightest
  findMaxUndistributed();


  // Go though all the faces to find how much of max_undistrubted effects them
  for(int i = 0; i < num_faces; i++){

    if(i == max_undistributed_patch) { continue; }

    // Used to compute absored

    // WARNING: Maybe flip args
    double F_i_max = getFormFactor(i,max_undistributed_patch);
    Vec3f  B_max = getUndistributed(max_undistributed_patch);
    Vec3f  D_i = mesh->getFace(i)->getMaterial()->getDiffuseColor(); //maybe reflective?
    Vec3f  A_i =  white - D_i;
    double  roughness = mesh->getFace(i)->getMaterial()->getRoughness();
    Vec3f  bounced 	= D_i*F_i_max*B_max;
    Vec3f  absorbed = A_i*F_i_max*B_max;


    // Radiance
    setRadiance(i,getRadiance(i) + bounced);
    setUndistributed(i,getUndistributed(i) + bounced);
    setAbsorbed(i,getAbsorbed(i)+absorbed);

    // Statisitics 
    if(false){
      //printf("Patch: %d stats=================\n",i);
      //std::cout << "undistributed: " << getUndistributed(i) << std::endl;
      //std::cout << "absorbed:      " << getAbsorbed(i)      << std::endl;
      //std::cout << "radiance:      " << getRadiance(i)      << std::endl;
    }

  }

  //jump
  setUndistributed(max_undistributed_patch,Vec3f(0,0,0));
  findMaxUndistributed(); //calc a lot of stuff

  // Trying to calculte ambient light
  if(args->ambient_term){
	  double radiosity_delta= total_undistributed / total_area;
	  double diff_total = 0.0;

  	// percent reflective
	  for(int i = 0; i < num_faces; i++)
	  	diff_total += mesh->getFace(i)->getMaterial()->getDiffuseColor().Length() * getArea(i);

	  
	  double diffuse_delta = diff_total/total_area;
	  assert(diffuse_delta > 0 && radiosity_delta >0);
	  double R_total  = 1.0 / (1.0 - diffuse_delta);
	  double ambient_total = R_total * radiosity_delta;
	  assert(0<=ambient_total && ambient_total <=1);
	  ambient = ambient_total;


	  // update
	  for(int i = 0; i <num_faces;i++){
	  	//convert ambient_total to grey
	  	Vec3f dif = mesh->getFace(i)->getMaterial()->getDiffuseColor();
	  	Vec3f amb_dif = ambient * dif;
	  	setRadiance(i,getRadiance(i) + amb_dif);
	  }
	}
  return total_undistributed*total_area;

}


// =======================================================================================
// VBO & DISPLAY FUNCTIONS
// =======================================================================================

// for interpolation
void CollectFacesWithVertex(Vertex *have, Face *f, std::vector<Face*> &faces) {
  for (unsigned int i = 0; i < faces.size(); i++) {
    if (faces[i] == f) return;
  }
  if (have != (*f)[0] && have != (*f)[1] && have != (*f)[2] && have != (*f)[3]) return;
  faces.push_back(f);
  for (int i = 0; i < 4; i++) {
    Edge *ea = f->getEdge()->getOpposite();
    Edge *eb = f->getEdge()->getNext()->getOpposite();
    Edge *ec = f->getEdge()->getNext()->getNext()->getOpposite();
    Edge *ed = f->getEdge()->getNext()->getNext()->getNext()->getOpposite();
    if (ea != NULL) CollectFacesWithVertex(have,ea->getFace(),faces);
    if (eb != NULL) CollectFacesWithVertex(have,eb->getFace(),faces);
    if (ec != NULL) CollectFacesWithVertex(have,ec->getFace(),faces);
    if (ed != NULL) CollectFacesWithVertex(have,ed->getFace(),faces);
  }
}

// different visualization modes
Vec3f Radiosity::setupHelperForColor(Face *f, int i, int j) {
  assert (mesh->getFace(i) == f);
  assert (j >= 0 && j < 4);
  if (args->render_mode == RENDER_MATERIALS) {
    return f->getMaterial()->getDiffuseColor();
  } else if (args->render_mode == RENDER_RADIANCE && args->interpolate == true) {
    std::vector<Face*> faces;
    CollectFacesWithVertex((*f)[j],f,faces);
    double total = 0;
    Vec3f color = Vec3f(0,0,0);
    Vec3f normal = f->computeNormal();
    for (unsigned int i = 0; i < faces.size(); i++) {
      Vec3f normal2 = faces[i]->computeNormal();
      double area = faces[i]->getArea();
      if (normal.Dot3(normal2) < 0.5) continue;
      assert (area > 0);
      total += area;
      color += area * getRadiance(faces[i]->getRadiosityPatchIndex());
    }
    assert (total > 0);
    color /= total;
    return color;
  } else if (args->render_mode == RENDER_LIGHTS) {
    return f->getMaterial()->getEmittedColor();
  } else if (args->render_mode == RENDER_UNDISTRIBUTED) { 
    return getUndistributed(i);
  } else if (args->render_mode == RENDER_ABSORBED) {
    return getAbsorbed(i);
  } else if (args->render_mode == RENDER_RADIANCE) {
    return getRadiance(i);
  } else if (args->render_mode == RENDER_FORM_FACTORS) {
    if (formfactors == NULL) ComputeFormFactors();
    double scale = 0.2 * total_area/getArea(i);
    double factor = scale * getFormFactor(max_undistributed_patch,i);
    return Vec3f(factor,factor,factor);
  } else {
    assert(0);
  }
  exit(0);
}


void Radiosity::initializeVBOs() {
  // create a pointer for the vertex & index VBOs
  glGenBuffers(1, &mesh_quad_verts_VBO);
  glGenBuffers(1, &mesh_quad_indices_VBO);
  glGenBuffers(1, &mesh_textured_quad_indices_VBO);
  glGenBuffers(1, &mesh_interior_edge_indices_VBO);
  glGenBuffers(1, &mesh_border_edge_indices_VBO);
}


void Radiosity::setupVBOs() {
  mesh_quad_verts.clear();
  mesh_quad_indices.clear();
  mesh_textured_quad_indices.clear();
  mesh_border_edge_indices.clear();
  mesh_interior_edge_indices.clear();

  // initialize the data in each vector
  int num_faces = mesh->numFaces();
  assert (num_faces > 0);
  for (int i = 0; i < num_faces; i++) {
    Face *f = mesh->getFace(i);
    Edge *e = f->getEdge();
    for (int j = 0; j < 4; j++) {
      Vec3f pos = ((*f)[j])->get();
      Vec3f normal = f->computeNormal();
      Vec3f color = setupHelperForColor(f,i,j);
      color = Vec3f(linear_to_srgb(color.r()),
		    linear_to_srgb(color.g()),
		    linear_to_srgb(color.b()));
      mesh_quad_verts.push_back(VBOPosNormalColorTexture(pos,normal,color,(*f)[j]->get_s(),(*f)[j]->get_t()));
      if (e->getOpposite() == NULL) { 
	mesh_border_edge_indices.push_back(VBOIndexedEdge(i*4+j,i*4+(j+1)%4));
      } else if (e->getStartVertex()->getIndex() < e->getEndVertex()->getIndex()) {
	mesh_interior_edge_indices.push_back(VBOIndexedEdge(i*4+j,i*4+(j+1)%4));
      }
      e = e->getNext();
    }
    if (f->getMaterial()->hasTextureMap()) {
      mesh_textured_quad_indices.push_back(VBOIndexedQuad(i*4,i*4+1,i*4+2,i*4+3));
    } else {
      mesh_quad_indices.push_back(VBOIndexedQuad(i*4,i*4+1,i*4+2,i*4+3));
    }
    // also outline the max_undistributed patch
    if (args->render_mode == RENDER_FORM_FACTORS && i == max_undistributed_patch) {
      mesh_border_edge_indices.push_back(VBOIndexedEdge(i*4+0,i*4+1));
      mesh_border_edge_indices.push_back(VBOIndexedEdge(i*4+1,i*4+2));
      mesh_border_edge_indices.push_back(VBOIndexedEdge(i*4+2,i*4+3));
      mesh_border_edge_indices.push_back(VBOIndexedEdge(i*4+3,i*4+0));
    }
  }
  assert ((int)mesh_quad_verts.size() == num_faces*4);
  assert ((int)mesh_quad_indices.size() + (int)mesh_textured_quad_indices.size() == num_faces);

  // cleanup old buffer data (if any)
  cleanupVBOs();

  // copy the data to each VBO
  glBindBuffer(GL_ARRAY_BUFFER,mesh_quad_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColorTexture) * num_faces * 4,
	       &mesh_quad_verts[0],
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_quad_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedQuad) * mesh_quad_indices.size(),
	       &mesh_quad_indices[0], GL_STATIC_DRAW);
  if (mesh_textured_quad_indices.size() > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_textured_quad_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOIndexedQuad) * mesh_textured_quad_indices.size(),
		 &mesh_textured_quad_indices[0], GL_STATIC_DRAW);
  }
  if (mesh_interior_edge_indices.size() > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_interior_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOIndexedEdge) * mesh_interior_edge_indices.size(),
		 &mesh_interior_edge_indices[0], GL_STATIC_DRAW);
  }
  if (mesh_border_edge_indices.size() > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_border_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOIndexedEdge) * mesh_border_edge_indices.size(),
		 &mesh_border_edge_indices[0], GL_STATIC_DRAW);
  }

  // WARNING: this naive VBO implementation only allows a single texture
  int num_textured_materials = 0;
  for (unsigned int mat = 0; mat < mesh->materials.size(); mat++) {
    Material *m = mesh->materials[mat];
    if (m->hasTextureMap()) {
      glBindTexture(GL_TEXTURE_2D,m->getTextureID());
      num_textured_materials++;
    }
  }
  assert (num_textured_materials <= 1);
}


void Radiosity::drawVBOs() {
  // =====================
  // DRAW ALL THE POLYGONS
  if (args->render_mode == RENDER_MATERIALS) {
    glEnable(GL_LIGHTING);
  } else {
    glDisable(GL_LIGHTING);
  }
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.1,4.0);
  int num_faces = mesh->numFaces();
  assert ((int)mesh_quad_indices.size() + (int)mesh_textured_quad_indices.size() == num_faces);

  glBindBuffer(GL_ARRAY_BUFFER, mesh_quad_verts_VBO);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOPosNormalColorTexture), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOPosNormalColorTexture), BUFFER_OFFSET(12));
  glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(3, GL_FLOAT, sizeof(VBOPosNormalColorTexture), BUFFER_OFFSET(24));

  // draw non textured faces
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_quad_indices_VBO);
  glDrawElements(GL_QUADS, 
		 mesh_quad_indices.size()*4,
		 GL_UNSIGNED_INT,
		 BUFFER_OFFSET(0));

  // draw textured faces
  if (args->render_mode == RENDER_MATERIALS) {
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer( 2, GL_FLOAT, sizeof(VBOPosNormalColorTexture), BUFFER_OFFSET(36));
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_textured_quad_indices_VBO);
  glDrawElements(GL_QUADS, 
		 mesh_textured_quad_indices.size()*4,
		 GL_UNSIGNED_INT,
		 BUFFER_OFFSET(0));
  if (args->render_mode == RENDER_MATERIALS) {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
  }

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  glDisable(GL_POLYGON_OFFSET_FILL);  


  // =====================
  // DRAW WIREFRAME
  if (args->wireframe) {
    glDisable(GL_LIGHTING);
    if (mesh_interior_edge_indices.size() > 0) {
      glLineWidth(1);
      glColor3f(0,0,0);
      glBindBuffer(GL_ARRAY_BUFFER, mesh_quad_verts_VBO);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, sizeof(VBOPosNormalColorTexture), BUFFER_OFFSET(0));
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_interior_edge_indices_VBO);
      glDrawElements(GL_LINES, mesh_interior_edge_indices.size()*2, GL_UNSIGNED_INT, 0);
      glDisableClientState(GL_VERTEX_ARRAY);
    }
    if (mesh_border_edge_indices.size() > 0) {
      glLineWidth(3);
      glColor3f(1,0,0);
      glBindBuffer(GL_ARRAY_BUFFER, mesh_quad_verts_VBO);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, sizeof(VBOPosNormalColorTexture), BUFFER_OFFSET(0));
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_border_edge_indices_VBO);
      glDrawElements(GL_LINES, mesh_border_edge_indices.size()*2, GL_UNSIGNED_INT, 0);
      glDisableClientState(GL_VERTEX_ARRAY);
    }
  }
  HandleGLError(); 
}


void Radiosity::cleanupVBOs() {
  glDeleteBuffers(1, &mesh_quad_verts_VBO);
  glDeleteBuffers(1, &mesh_quad_indices_VBO);
  glDeleteBuffers(1, &mesh_textured_quad_indices_VBO);
  glDeleteBuffers(1, &mesh_interior_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_border_edge_indices_VBO);
}

