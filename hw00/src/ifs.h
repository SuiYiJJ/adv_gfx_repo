#ifndef _IFS_H_
#define _IFS_H_

#include <cassert>
#include <vector>
#include <string>

#include "glCanvas.h"

class ArgParser;

// ====================================================================
// ====================================================================
// class to manage the data and drawing for an "iterated function system"

class IFS {

  // ASSIGNMENT: add whatever additional variables & functions you need

public:

  // CONSTRUCTOR
  IFS(ArgParser *a);

  // HELPER FUNCTIONS FOR RENDERING
  void setupVBOs();
  void drawVBOs(GLuint MatrixID,const glm::mat4 &MVP);
  void cleanupVBOs();

private:

  // private helper functions for VBOs
  void setupPoints();
  void drawPoints() const;
  void cleanupPoints();
  void setupCube();
  void drawCube() const;
  void cleanupCube();

  // REPRESENTATION
  ArgParser *args;
  GLuint VaoId;
  GLuint VboId;
};

// ====================================================================
// ====================================================================

// some helper stuff for VBOs
#define NUM_CUBE_TRIANGLES 12 
#define NUM_CUBE_VERTS NUM_CUBE_TRIANGLES * 3

class VertexPosColor {
public:
  // by default vertices are set to black
  VertexPosColor(const glm::vec4 &pos=glm::vec4(0,0,0,1), 
                 const glm::vec4 &color=glm::vec4(0,0,0,1)) :
    XYZW(pos),RGBA(color) {}
  glm::vec4 XYZW;
  glm::vec4 RGBA;
};


// ====================================================================
// ====================================================================

#endif
