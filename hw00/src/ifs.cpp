#include <cassert>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#include "ifs.h"
#include "argparser.h"

// ====================================================================
// ====================================================================

IFS::IFS(ArgParser *a) : args(a) {

  // open the file
  std::ifstream input((args->path+"/"+args->input_file).c_str());
  if (!input.good()) {
    std::cout << "ERROR: must specify valid input file" << args->path+"/"+args->input_file << std::endl;
    exit(1);
  }

  // read the number of transforms
  int num_transforms; 
  input >> num_transforms;
    
  // read in the transforms
  for (int i = 0; i < num_transforms; i++) {
    float probability; 
    input >> probability;
    float float_data[16];
    for (int row = 0; row < 4; row++) {
      for (int col = 0; col < 4; col++) {
        input >> float_data[col*4+row];
      }
    } 
    glm::mat4 m = glm::make_mat4(float_data);


    // ASSIGNMENT: do something with each probability & matrix

    
  }  
}

// ====================================================================
// ====================================================================

void IFS::setupVBOs() {
  HandleGLError("enter setupVBOs");
  if (args->cubes) {
    setupCube();
  } else {
    setupPoints();
  }
  HandleGLError("leaving setupVBOs");
}


// NOTE: The matrix that is passed in contains the transformations for
// the current camera/view position.  When drawing the objects in the
// scene you can apply additional transformations by multiplying your
// own matrices with this matrix before calling draw.

void IFS::drawVBOs(GLuint MatrixID,const glm::mat4 &m) {
  HandleGLError("enter drawVBOs");
  if (args->cubes) {



    // ASSIGNMENT: don't just draw one cube...
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &m[0][0]);
    drawCube();



  } else {
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &m[0][0]);
    drawPoints();
  }
  HandleGLError("leaving drawVBOs");
}

void IFS::cleanupVBOs() {
  HandleGLError("enter cleanupVBOs");
  if (args->cubes) {
    cleanupCube();
  } else {
    cleanupPoints();
  }
  HandleGLError("leaving cleanupVBOs");
}

// ====================================================================
// ====================================================================

void IFS::setupPoints() {
  HandleGLError("enter setupPoints");

  // allocate space for the data
  VertexPosColor* points = new VertexPosColor[args->points];

  // generate a block of random data
  for (int i = 0; i < args->points; i++) {
    double x = args->mtrand.rand();
    double y = args->mtrand.rand();
    double z = args->mtrand.rand();
    glm::vec4 pt(x,y,z,1);


    // ASSIGNMENT: manipulate point


    points[i] = VertexPosColor(pt);
  }

  // create a pointer for the VBO
  glGenVertexArrays(1, &VaoId);
  glBindVertexArray(VaoId);
  glGenBuffers(1, &VboId);
  glBindBuffer(GL_ARRAY_BUFFER,VboId); 
  glBufferData(GL_ARRAY_BUFFER,sizeof(VertexPosColor) * args->points,points,GL_STATIC_DRAW); 
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexPosColor), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexPosColor), (GLvoid*)sizeof(glm::vec4));
  
  // increase the size of the points slightly
  glPointSize(2.0);

  delete [] points;
  HandleGLError("leaving setupPoints");
}


void IFS::drawPoints() const {
  HandleGLError("enter drawPoints");
  glDrawArrays(GL_POINTS, 0, args->points);
  HandleGLError("leaving drawPoints");
}


void IFS::cleanupPoints() {
  glDeleteBuffers(1, &VaoId);
  glDeleteBuffers(1, &VboId);
}

// ====================================================================
// ====================================================================

void IFS::setupCube() {

  HandleGLError("enter setupCube");
  
  VertexPosColor Vertices[] =
    {
      // back face, cyan
      VertexPosColor(glm::vec4(0,0,0, 1.0f),glm::vec4(0,1,1, 1.0f)),
      VertexPosColor(glm::vec4(0,1,0, 1.0f),glm::vec4(0,1,1, 1.0f)),
      VertexPosColor(glm::vec4(1,1,0, 1.0f),glm::vec4(0,1,1, 1.0f)),
      
      VertexPosColor(glm::vec4(0,0,0, 1.0f),glm::vec4(0,1,1, 1.0f)),
      VertexPosColor(glm::vec4(1,1,0, 1.0f),glm::vec4(0,1,1, 1.0f)),
      VertexPosColor(glm::vec4(1,0,0, 1.0f),glm::vec4(0,1,1, 1.0f)),
      
      // front face, red
      VertexPosColor(glm::vec4(0,0,1, 1.0f),glm::vec4(1,0,0, 1.0f)),
      VertexPosColor(glm::vec4(1,1,1, 1.0f),glm::vec4(1,0,0, 1.0f)),
      VertexPosColor(glm::vec4(0,1,1, 1.0f),glm::vec4(1,0,0, 1.0f)),
      
      VertexPosColor(glm::vec4(0,0,1, 1.0f),glm::vec4(1,0,0, 1.0f)),
      VertexPosColor(glm::vec4(1,0,1, 1.0f),glm::vec4(1,0,0, 1.0f)),
      VertexPosColor(glm::vec4(1,1,1, 1.0f),glm::vec4(1,0,0, 1.0f)),
      
      // bottom face, purple
      VertexPosColor(glm::vec4(0,0,0, 1.0f),glm::vec4(1,0,1, 1.0f)),
      VertexPosColor(glm::vec4(1,0,1, 1.0f),glm::vec4(1,0,1, 1.0f)),
      VertexPosColor(glm::vec4(0,0,1, 1.0f),glm::vec4(1,0,1, 1.0f)),
      
      VertexPosColor(glm::vec4(0,0,0, 1.0f),glm::vec4(1,0,1, 1.0f)),
      VertexPosColor(glm::vec4(1,0,0, 1.0f),glm::vec4(1,0,1, 1.0f)),
      VertexPosColor(glm::vec4(1,0,1, 1.0f),glm::vec4(1,0,1, 1.0f)),
    
      // top face, green
      VertexPosColor(glm::vec4(0,1,0, 1.0f),glm::vec4(0,1,0, 1.0f)),
      VertexPosColor(glm::vec4(0,1,1, 1.0f),glm::vec4(0,1,0, 1.0f)),
      VertexPosColor(glm::vec4(1,1,1, 1.0f),glm::vec4(0,1,0, 1.0f)),
      
      VertexPosColor(glm::vec4(0,1,0, 1.0f),glm::vec4(0,1,0, 1.0f)),
      VertexPosColor(glm::vec4(1,1,1, 1.0f),glm::vec4(0,1,0, 1.0f)),
      VertexPosColor(glm::vec4(1,1,0, 1.0f),glm::vec4(0,1,0, 1.0f)),

      // left face, yellow
      VertexPosColor(glm::vec4(0,0,0, 1.0f),glm::vec4(1,1,0, 1.0f)),
      VertexPosColor(glm::vec4(0,0,1, 1.0f),glm::vec4(1,1,0, 1.0f)),
      VertexPosColor(glm::vec4(0,1,1, 1.0f),glm::vec4(1,1,0, 1.0f)),
      
      VertexPosColor(glm::vec4(0,0,0, 1.0f),glm::vec4(1,1,0, 1.0f)),
      VertexPosColor(glm::vec4(0,1,1, 1.0f),glm::vec4(1,1,0, 1.0f)),
      VertexPosColor(glm::vec4(0,1,0, 1.0f),glm::vec4(1,1,0, 1.0f)),

      // right face, blue
      VertexPosColor(glm::vec4(1,0,0, 1.0f),glm::vec4(0,0,1, 1.0f)),
      VertexPosColor(glm::vec4(1,1,1, 1.0f),glm::vec4(0,0,1, 1.0f)),
      VertexPosColor(glm::vec4(1,0,1, 1.0f),glm::vec4(0,0,1, 1.0f)),
      
      VertexPosColor(glm::vec4(1,0,0, 1.0f),glm::vec4(0,0,1, 1.0f)),
      VertexPosColor(glm::vec4(1,1,0, 1.0f),glm::vec4(0,0,1, 1.0f)),
      VertexPosColor(glm::vec4(1,1,1, 1.0f),glm::vec4(0,0,1, 1.0f))
    };
  
  glGenVertexArrays(1, &VaoId);
  glBindVertexArray(VaoId);
  glGenBuffers(1, &VboId);
  glBindBuffer(GL_ARRAY_BUFFER, VboId);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexPosColor), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexPosColor), (GLvoid*)sizeof(glm::vec4));
  
  HandleGLError("leaving setupCcube");
}


void IFS::drawCube() const {
  HandleGLError("in drawCube");
  glDrawArrays(GL_TRIANGLES, 0, NUM_CUBE_VERTS);
  HandleGLError("leaving drawCube");
}


void IFS::cleanupCube() {
  glDeleteBuffers(1, &VaoId);
  glDeleteBuffers(1, &VboId);
}

// ====================================================================
// ====================================================================

