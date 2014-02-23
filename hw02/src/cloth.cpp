#include "glCanvas.h"
#include <fstream>
#include <cmath>
#include "cloth.h"
#include "argparser.h"
#include "vectors.h"
#include "utils.h"
using std::vector;

// ================================================================================
// ================================================================================

Cloth::Cloth(ArgParser *_args) {
  args =_args;

  // open the file
  std::ifstream istr(args->cloth_file.c_str());
  assert (istr != NULL);
  std::string token;

  // read in the simulation parameters
  istr >> token >> k_structural; assert (token == "k_structural");  // (units == N/m)  (N = kg*m/s^2)
  istr >> token >> k_shear; assert (token == "k_shear");
  istr >> token >> k_bend; assert (token == "k_bend");
  istr >> token >> damping; assert (token == "damping");
  // NOTE: correction factor == .1, means springs shouldn't stretch more than 10%
  //       correction factor == 100, means don't do any correction
  istr >> token >> provot_structural_correction; assert (token == "provot_structural_correction");
  istr >> token >> provot_shear_correction; assert (token == "provot_shear_correction");

  // the cloth dimensions
  istr >> token >> nx >> ny; // (units == meters)
  assert (token == "m");
  assert (nx >= 2 && ny >= 2);

  // the corners of the cloth
  Vec3f a,b,c,d;
  istr >> token >> a; assert (token == "p");
  istr >> token >> b; assert (token == "p");
  istr >> token >> c; assert (token == "p");
  istr >> token >> d; assert (token == "p");

  // fabric weight  (units == kg/m^2)
  // denim ~300 g/m^2
  // silk ~70 g/m^2
  double fabric_weight;
  istr >> token >> fabric_weight; assert (token == "fabric_weight");
  double area = AreaOfTriangle(a,b,c) + AreaOfTriangle(a,c,d);

  // create the particles
  particles = new ClothParticle[nx*ny];
  double mass = area*fabric_weight / double(nx*ny);
  for (int i = 0; i < nx; i++) {
    double x = i/double(nx-1);
    Vec3f ab = (1-x)*a + x*b;
    Vec3f dc = (1-x)*d + x*c;
    for (int j = 0; j < ny; j++) {
      double y = j/double(ny-1);
      ClothParticle &p = getParticle(i,j);
      Vec3f abdc = (1-y)*ab + y*dc;
      p.setOriginalPosition(abdc);
      p.setPosition(abdc);
      p.setVelocity(Vec3f(0,0,0));
      p.setMass(mass);
      p.setFixed(false);
    }
  }

  // the fixed particles
  while (istr >> token) {
    assert (token == "f");
    int i,j;
    double x,y,z;
    istr >> i >> j >> x >> y >> z;
    ClothParticle &p = getParticle(i,j);
    p.setPosition(Vec3f(x,y,z));
    p.setFixed(true);
  }

  computeBoundingBox();
  initializeVBOs();
  setupVBOs();
}

// ================================================================================

void Cloth::computeBoundingBox() {
  box = BoundingBox(getParticle(0,0).getPosition());
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < ny; j++) {
      box.Extend(getParticle(i,j).getPosition());
      box.Extend(getParticle(i,j).getOriginalPosition());
    }
  }
}

// ================================================================================

const vector<ClothParticle*> Cloth::getAdjParticles(int i, int j) {
  // Input: Given an i,j index
  // Output: This will return the adj (structural) particles in a vector

  //Check if valid index
  assert(0 <= i && i < nx);
  assert(0 <= j && j < nx);

  vector<ClothParticle*> adjPartVec;
  ClothParticle * ptr = NULL; 

  // Get left
  if(j - 1 >= 0){
    ptr = &getParticle(i,j-1);
    adjPartVec.push_back(ptr);
  }
  // Get right
  if(j + 1 < ny){
    ptr = &getParticle(i,j+1);
    adjPartVec.push_back(ptr);
  }
  // Get top
  if(i - 1 >= 0){
    ptr = &getParticle(i-1,j);
    adjPartVec.push_back(ptr);
  }
  // Get bottom
  if(i + 1 < nx){
    ptr = &getParticle(i+1,j);
    adjPartVec.push_back(ptr);
  }

  return  adjPartVec;
}

const vector<ClothParticle*> Cloth::getShearParticles(int i, int j) {
  // Input: Given an i,j index
  // Output: This will return the adj (Shear) particles in a vector

  //Check if valid index
  assert(0 <= i && i < nx);
  assert(0 <= j && j < nx);

  vector<ClothParticle*> shearPartVec;
  ClothParticle * ptr = NULL; 

  // Get upperLeft
  if(i - 1 >= 0 && j - 1 >= 0){
    ptr = &getParticle( i-1 , j-1);
    shearPartVec.push_back(ptr);
  }

  // Get lowerRight
  if(i + 1 < nx && j + 1 < ny){
    ptr = &getParticle( i + 1 , j + 1);
    shearPartVec.push_back(ptr);
  }

  // Get upperRight
  if(i - 1 >= 0 && j + 1 < ny){
    ptr = &getParticle(i-1, j+1);
    shearPartVec.push_back(ptr);
  }

  // Get lowerLeft
  if(i + 1 < nx && j - 1 >= 0){
    ptr = &getParticle(i+1,j-1);
    shearPartVec.push_back(ptr);
  }

  return shearPartVec;
}

// ================================================================================

const vector<ClothParticle*> Cloth::getFlexParticles(int i, int j) {

  //Check if valid index
  assert(0 <= i && i < nx);
  assert(0 <= j && j < nx);

  vector<ClothParticle*> flexPartVec;
  ClothParticle * ptr = NULL; 

  // Get top
  if(i - 2 >= 0){
    ptr = &getParticle(i-2,j);
    flexPartVec.push_back(ptr);
  }

  // Get bot
  if(i + 2 < nx){
    ptr = &getParticle(i+2,j);
    flexPartVec.push_back(ptr);
  }


  // Get left
  if(j - 2 >= 0){
    ptr = &getParticle( i  , j - 2);
    flexPartVec.push_back(ptr);
  }

  // Get right
  if(j + 2 < ny ){
    ptr = &getParticle( i  , j + 2);
    flexPartVec.push_back(ptr);
  }

  return flexPartVec;
}

// ================================================================================

const Vec3f Cloth::getSpringForce(ClothParticle* a, ClothParticle* b) {

  //TODO Calc spring const between these two
  Vec3f aOrginalPos = a->getOriginalPosition();
  Vec3f bOrginalPos = b->getOriginalPosition();
  double restLength = aOrginalPos.Distance3f(bOrginalPos);
  Vec3f p_i = a->getPosition();
  Vec3f p_j = b->getPosition();

  // JUMP
  double displace = k_structural * (p_i.Distance3f(p_j) - restLength);
  Vec3f ratio = (p_j - p_i) * (1/p_i.Distance3f(p_j));
  return displace * ratio;
}

// ================================================================================

void Cloth::Animate() {

  // Get current time and update global

  // For each particle in the system update the states
  for( int i = 0; i < nx; i++){

    for( int j = 0; j < ny; j++){

      // Trying to do eulers method
      ClothParticle* curP = &getParticle(i,j);

      // Unless fixed
      if((curP->isFixed()))
        continue;

      // Calculating Forces///////////////////////////////
      // * Gravity
      // * Spring - Structural,Shear,Bend
      // * Dampening 

      Vec3f f_damp = -1* damping * curP->getVelocity();
      Vec3f f_gravity = curP->getMass() * args->gravity;
      // f_spring = addition of all forces
      Vec3f f_spring;

      // Structural
      vector<ClothParticle*> adjPartVec = getAdjParticles(i,j);
      for(unsigned int v = 0; v < adjPartVec.size(); v++)
        f_spring = f_spring + getSpringForce(curP,adjPartVec[v]);
      // Shear 
      vector<ClothParticle*> shearVec = getShearParticles(i,j);
      for(unsigned int v = 0; v < shearVec.size(); v++)
        f_spring = f_spring + getSpringForce(curP,shearVec[v]);
      // Flex
      vector<ClothParticle*> flexVec = getFlexParticles(i,j);
      for(unsigned int v = 0; v < flexVec.size(); v++)
        f_spring = f_spring + getSpringForce(curP,flexVec[v]);

      // Accleration
      curP->setAcceleration((1/curP->getMass()) * (f_spring + f_gravity + f_damp ));

      // Update Velocity
      curP->setVelocity(curP->getVelocity() + (args->timestep * curP->getAcceleration()));

      // Update Position
      curP->setPosition(curP->getPosition() + args->timestep * curP->getVelocity());
    }
  }

  // Get the fixed distance for  structure particles
  double restStructLength = getParticle(0,0).getOriginalPosition().Distance3f(getParticle(0,1).getOriginalPosition());

  // For each particle in the system check if it is overstreched
  for(unsigned int i = 0; i < nx; i++){

    for(unsigned int j = 0; j < ny; j++){

      // Collecting surrounding nodes 
      ClothParticle* curP = &getParticle(i,j);
      vector<ClothParticle*> adjPartVec = getAdjParticles(i,j);

      for(unsigned int v = 0; v < adjPartVec.size(); v++){
        // For edge check if oversteched
        ClothParticle * other = &getParticle(i,j);
        double distance = curP->getPosition().Distance3f(other->getPosition());
        if(distance > (provot_structural_correction * restStructLength)){

          // Which are fixed? none, one or both?
          if(curP->isLoose() && other->isLoose()){

          }else if( curP->isLoose() && other->isFixed()){

          }else if( curP->isFixed() && other->isLoose()){

          }else{
            // Then both are fixed, in which case do nothing.
            assert(curP->isFixed() && other->isLoose());
          }
        }
      }
    }
  }
  // redo VBOs for rendering
  setupVBOs();
}

// ================================================================================
