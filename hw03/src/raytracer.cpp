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
    RayTree::AddMainSegment(ray,0,10);
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

  // Add in my ray
  RayTree::AddReflectedSegment(ray,0,hit.getT());

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

    // Get a collection of random points on light source
    std::vector<Vec3f> randomLightVec;

    // Are we doing a regular this via fuzzy shadow samples,
    // or are we doing the old fasion way?
    if(args->num_shadow_samples == 1 || args->num_shadow_samples == 0){
      randomLightVec.push_back(f->computeCentroid());
    }else{
      // Create those random points
      for(int r = 0; r < args->num_shadow_samples; r++)
        randomLightVec.push_back(f->RandomPoint());
    }

    // Contrubution of light from every shadow samples
    for(int r = 0; r < randomLightVec.size(); r++){


      // Get color of light
      Vec3f lightColor = f->getMaterial()->getEmittedColor() * f->getArea();

      // Color I will have locally
      Vec3f myLightColor;

      // Middle of where light source is?
      Vec3f lightCentroid = randomLightVec[r];

      // Get the direction to that light center point
      Vec3f dirToLightCentroid = lightCentroid-point;
      dirToLightCentroid.Normalize();
      
      // How far am I from the light?
      double distToLightCentroid = (lightCentroid-point).Length();

      // Math to get my light color
      myLightColor = lightColor / (M_PI*distToLightCentroid*distToLightCentroid);
      myLightColor = (1.0/randomLightVec.size()) * myLightColor;

      // ===========================================
      // ASSIGNMENT:  REGULAR NO-SHADOW LOGIC
      // ===========================================

      if(args->num_shadow_samples == 0){
        answer += m->Shade(ray,hit,dirToLightCentroid,myLightColor,args);
        break;
      }

      // ===========================================
      // ASSIGNMENT:  ADD SHADOW & SOFT SHADOW LOGIC
      // ===========================================

      Ray shadowRay(point, dirToLightCentroid);
      Hit shadowHit = Hit();

      // If I hit something and that something isn't the lightsource
      if(CastRay(shadowRay,shadowHit,false) && shadowHit.getMaterial()->getEmittedColor().Length() <= 0.001 ){

        RayTree::AddShadowSegment(shadowRay,0,shadowHit.getT());

      }else{

        RayTree::AddMainSegment(shadowRay,0,100);
        answer += m->Shade(ray,hit,dirToLightCentroid,myLightColor,args);
      }
    }
  }

   

  Vec3f reflectiveColor = m->getReflectiveColor();
  // ----------------------------------------------
  // add contribution from reflection, if the surface is shiny

  // Check if reflective if not just return, all color is absorbed
  if(reflectiveColor.x() == 0 && reflectiveColor.y() == 0 && reflectiveColor.z() ==0){
    return answer;
  }


  if(bounce_count > 0){


    // Calculate reflect direction vector
    double term = -1* hit.getNormal().Dot3(ray.getDirection());
    Vec3f reflectiveDir = ray.getDirection() + (2*term*hit.getNormal());
    reflectiveDir.Normalize();

    // New ray
    Ray reflectRay(point,reflectiveDir);

    Hit newHit;
    return reflectiveColor*(answer + TraceRay(reflectRay,newHit,bounce_count-1)); 

  }else{

  // These is more bounce left!
  return answer;


  }

  
}

