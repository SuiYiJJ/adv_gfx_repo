#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include "image.h"
#include "furniture.h"
#include "MersenneTwister.h"

// ===================================================================================================
// ===================================================================================================
const unsigned int  TEXTURE_SIZE_X = 512;
const unsigned int  TEXTURE_SIZE_Y = 512;
const unsigned int  DESK_WIDTH     = 35;
const unsigned int  DESK_LENGTH    = 35;
const unsigned int  BLACK_THRESH   = (1.0/4.0) * DESK_LENGTH * DESK_WIDTH;
// ===================================================================================================
// ===================================================================================================
void loadFurnitureState(std::ifstream & in_str,  std::vector<Furniture> & fVec, int & itemCount);
void saveFurnitureState(std::ofstream & out_str, std::vector<Furniture> & fVec, int & itemCount);
double calculateCost(std::vector<Furniture> & fVec, Image<Color> &  texture);
void setupInitalRandomPos(std::vector<Furniture> &  fVec, Image<Color> & texture);
Color getApproxColor(Furniture& item, Image<Color>& texture);
bool insideSpace(Furniture& item, Image<Color>& texture);
bool inRange(float x, float y);
// ===================================================================================================
// ===================================================================================================

void usage(char* executable) {
  std::cerr << " usage " << executable << " floor_image.ppm states.st"<< std::endl;
}

// ===================================================================================================
// ===================================================================================================

int main(int argc, char* argv[]) {
  // spit out error if less then two elements
  if (argc < 2) { usage(argv[0]); exit(1); }
  
    // Load ppm image of floor textures to use
    Image<Color> floor_texture;
    std::string image_file = std::string(argv[1]);
    std::cout << "Loading " << argv[1] << std::endl;
    floor_texture.Load(image_file);

    // Load in location of furniture
    std::ifstream in_str(argv[2]);
    
    // Is this file is good
    if (!in_str.good()) {
        std::cerr << "Can't open " << argv[2] << " to read.\n";
        exit(1);
    }

    //Load into vector as furnature objects
    std::vector<Furniture> furnitureVec;
    int itemCount = 0;
    loadFurnitureState(in_str,furnitureVec, itemCount);

    // Randomly moves my furniture until they are in good starting positions
    setupInitalRandomPos(furnitureVec,floor_texture);


    // ALL OTHER CODE WILL GO HERE

    // Save back file
    std::ofstream out_str(argv[2]);
    if (!out_str.good()) {
      std::cerr << "Can't open " << argv[4] << " to write.\n";
      exit(1);
    } 

    saveFurnitureState(out_str,furnitureVec, itemCount);
}//main

// ===================================================================================================
// ===================================================================================================

double calculateCost(std::vector<Furniture> & fVec, Image<Color>& texture){
  // This function will calculate the total cost of a function
}

void setupInitalRandomPos(std::vector<Furniture> &  fVec, Image<Color>& texture){
  // Genereate Random Position
  MTRand mtrand;
  bool legalPosition = false;
  unsigned int curIndex = 0;

  while(curIndex < fVec.size()){
    int start_x = mtrand.randInt(TEXTURE_SIZE_X);
    int start_y = mtrand.randInt(TEXTURE_SIZE_Y);
    double start_rot = mtrand.rand(2* M_PI);

    fVec[curIndex].setPos(start_x, start_y);
    fVec[curIndex].setAngle(start_rot);

    if(insideSpace(fVec[curIndex], texture)){
      curIndex++;
      continue;
    }
  }
}

void saveFurnitureState(std::ofstream & out_str, std::vector<Furniture> & fVec, int & itemCount){

  // number of objects
  out_str << "num_obj " << fVec.size() << std::endl;

  for(int i = 0; i < fVec.size(); i++){
  
    //print out type
    out_str << fVec[i].getType() << std::endl;

    // print out pos line
    out_str << "pos " << fVec[i].getPos().x << " " <<fVec[i].getPos().y << std::endl;

    // print out angle
    out_str << "rot " << fVec[i].getAngle() << std::endl;
  }
  
  out_str << "end" << std::endl;
  
}

bool inRange(float x, float y){
  return (0 <= x && x<= TEXTURE_SIZE_X) && ( 0 <= y && y<= TEXTURE_SIZE_Y);
}

Color getApproxColor(Furniture& item, Image<Color>& texture){

  //Calcuate Bounding Box and Rotate According to Angle (Radians)
  float x = item.getPos().x;
  float y = item.getPos().y;

  int max_x, max_y, min_x, min_y, totalPixels;

  max_x = (int) (x + DESK_LENGTH/2.0);
  min_x = (int) (x - DESK_WIDTH/2.0);
  
  max_y = (int) (x + DESK_LENGTH/2.0);
  min_y = (int) (x - DESK_WIDTH/2.0);


  totalPixels = DESK_LENGTH * DESK_WIDTH;
  // Make a black pixel that I will add to
  Color colorSum(0,0,0);


  // Check if we are inrange of our texture image
  if(!(inRange(max_x,max_y) && inRange(min_x, min_y))){
     return colorSum;
  }

  // for all pixles in that square range add to colorSum
  for(int x = min_x; x < max_x; x++){
    for(int y = min_y; y < max_y; y++){
      Color curPixel = texture.GetPixel(x,y);
      colorSum.r += curPixel.r;
      colorSum.g += curPixel.g;
      colorSum.b += curPixel.b;
    }
  }//for

  // Find the average color
  colorSum.r += colorSum.r / totalPixels;
  colorSum.g += colorSum.g / totalPixels;
  colorSum.b += colorSum.b / totalPixels;

  return colorSum;
}

bool insideSpace(Furniture& item, Image<Color>& texture){
  // Need to search inside the image for an approximate size
  // Hack: We will approximate desk length and width

  float x = item.getPos().x;
  float y = item.getPos().y;

  int max_x, max_y, min_x, min_y;

  max_x = (int) (x + DESK_LENGTH/2.0);
  min_x = (int) (x - DESK_WIDTH/2.0);
  
  max_y = (int) (x + DESK_LENGTH/2.0);
  min_y = (int) (x - DESK_WIDTH/2.0);

  // Make a black pixel that I will add to
  int totalBadPixels = 0;

  // Check if we are inrange of our texture image
  if(!(inRange(max_x,max_y) && inRange(min_x, min_y))){
     return false;
  }

  // for all pixles in that square range add to colorSum
  for(int x = min_x; x < max_x; x++){
    for(int y = min_y; y < max_y; y++){
      Color curPixel = texture.GetPixel(x,y);
      if(curPixel.isBlack())
        totalBadPixels++;
    }
  }//for

  // If we have too many black pixels
  return totalBadPixels <= BLACK_THRESH;
}

void loadFurnitureState(std::ifstream & in_str, 
    std::vector<Furniture> & fVec, int & itemCount){

  // function will load in furniture into the fVec
  std::string command;

  while(in_str >> command){
    
    if(command == "num_obj"){
      // total numbers of items we will use
      in_str >> itemCount;
    
    }else if(command == "desk"){
      // make default item, and push into vector
      Furniture item;
      // set item number
      item.setID(fVec.size());
      item.setType(command);

      // push
      fVec.push_back(item);
    
    }else if(command == "pos"){
      // get pos
      float x,y;
      in_str >> x >> y;
      fVec.back().setPos(x,y);
    
    }else if(command == "rot"){

      double rad;
      in_str >> rad;
      // conver to radians
      rad = rad * (M_PI / 180);
      fVec.back().setAngle(rad);
    
    }else if(command == "end"){
      // We are done reading in the inputs 
      return;
    }
  }//while
}//load
