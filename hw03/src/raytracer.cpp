#include "raytracer.h"
#include "material.h"
#include "vectors.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "primitive.h"
#include "photon_mapping.h"


// ===========================================================================
// casts a single ray through the scene geometry and finds the closest hit
// single ray, used in our Trace Ray
// Given a Ray, hit class is a class that stores information about the hit
bool RayTracer::CastRay(const Ray &ray, Hit &h, bool use_rasterized_patches) const {
  bool answer = false;

  // intersect each of the quads
  for (int i = 0; i < mesh->numOriginalQuads(); i++) {
    Face *f = mesh->getOriginalQuad(i);
    if (f->intersect(ray,h,args->intersect_backfacing)) answer = true;
  }

  // intersect each of the primitives (either the patches, or the original primitives)
  if (use_rasterized_patches) {
    for (int i = 0; i < mesh->numRasterizedPrimitiveFaces(); i++) {
      Face *f = mesh->getRasterizedPrimitiveFace(i);
      if (f->intersect(ray,h,args->intersect_backfacing)) answer = true;
    }
  } else {
    int num_primitives = mesh->numPrimitives();
    for (int i = 0; i < num_primitives; i++) {
      if (mesh->getPrimitive(i)->intersect(ray,h)){
        answer = true;
      } 
    }
  }
  return answer;
}

// ===========================================================================
// does the recursive (shadow rays & recursive rays) work
Vec3f RayTracer::TraceRay(Ray &ray, Hit &hit, int bounce_count) const {

  // First cast a ray and see if we hit anything. (Done)
  hit = Hit();
  bool intersect = CastRay(ray,hit,false);
    
  // if there is no intersection, simply return the background color
  if (intersect == false) {
    // Probs need to fix this for more complex background colors
    return Vec3f(srgb_to_linear(mesh->background_color.r()),
		 srgb_to_linear(mesh->background_color.g()),
		 srgb_to_linear(mesh->background_color.b()));
  }

  // otherwise decide what to do based on the material
  Material *m = hit.getMaterial();
  assert (m != NULL);

  // Add in conditional matrial code, aka volocities

  // rays coming from the light source are set to white, don't bother to ray trace further.
  if (m->getEmittedColor().Length() > 0.001) {
    return Vec3f(1,1,1);
  } 
 
  Vec3f normal = hit.getNormal();
  Vec3f point = ray.pointAtParameter(hit.getT());
  Vec3f answer;

  // ----------------------------------------------
  //  start with the indirect light (ambient light)
  Vec3f diffuse_color = m->getDiffuseColor(hit.get_s(),hit.get_t());
  if (args->gather_indirect) {
    // photon mapping for more accurate indirect light
    answer = diffuse_color * (photon_mapping->GatherIndirect(point, normal, ray.getDirection()) + args->ambient_light);
  } else {
    // the usual ray tracing hack for indirect light
    answer = diffuse_color * args->ambient_light;
  }      

  // ----------------------------------------------
  // add contributions from each light that is not in shadow

  // For each light source
  int num_lights = mesh->getLights().size();
  for (int i = 0; i < num_lights; i++) {

    // Get that light source
    Face *f = mesh->getLights()[i];

    // Get color of light
    Vec3f lightColor = f->getMaterial()->getEmittedColor() * f->getArea();

    // Color I will have locally
    Vec3f myLightColor;

    // Middle of where light source is?
    Vec3f lightCentroid = f->computeCentroid();

    // Get the direction to that light center point
    Vec3f dirToLightCentroid = lightCentroid-point;
    dirToLightCentroid.Normalize();
    
    // How far am I from the light?
    double distToLightCentroid = (lightCentroid-point).Length();

    // Math to get my light color
    myLightColor = lightColor / (M_PI*distToLightCentroid*distToLightCentroid);


    // Checking if we have a shadow
    Hit shadowHit;

    //Ray Towards Light
    if(CastRay())

    // ===========================================
    // ASSIGNMENT:  ADD SHADOW & SOFT SHADOW LOGIC
    // ===========================================

    // add the lighting contribution from this particular light at this point
    // (fix this to check for blockers between the light & this surface)
    answer += m->Shade(ray,hit,dirToLightCentroid,myLightColor,args);

  }



  // ----------------------------------------------
  // add contribution from reflection, if the surface is shiny
  Vec3f reflectiveColor = m->getReflectiveColor();

  // =================================
  // ASSIGNMENT:  ADD REFLECTIVE LOGIC
  // =================================




  
  return answer; 
}

