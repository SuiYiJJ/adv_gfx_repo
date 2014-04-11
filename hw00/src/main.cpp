// Graphics Library Includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "argparser.h"
#include "ifs.h"
#include "glCanvas.h"
#include "camera.h"

// ====================================================================
// ====================================================================

int main(int argc, char *argv[]) {
  // parse the command line arguments
  ArgParser args(argc, argv);
  // create the IFS object
  IFS ifs(&args);

  GLCanvas::initialize(&args,&ifs);  
  
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); 
  glEnable(GL_CULL_FACE);

  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders( args.path+"/ifs.vertexshader", args.path+"/ifs.fragmentshader" );
  
  // Get a handle for our "MVP" uniform
  GLuint MatrixID = glGetUniformLocation(programID, "MVP");

  while (!glfwWindowShouldClose(GLCanvas::window))  {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programID);

    GLCanvas::camera->glPlaceCamera();

    // Build the matrix to position the camera based on keyboard and mouse input
    glm::mat4 ProjectionMatrix = GLCanvas::camera->getProjectionMatrix();
    glm::mat4 ViewMatrix = GLCanvas::camera->getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    
    // pass the matrix to the draw routines (for further editing)
    ifs.drawVBOs(MatrixID,MVP);

    // Swap buffers
    glfwSwapBuffers(GLCanvas::window);
    glfwPollEvents();  
  }
  
  ifs.cleanupVBOs();
  glDeleteProgram(programID);
  
  // Close OpenGL window and terminate GLFW
  glfwDestroyWindow(GLCanvas::window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

// ====================================================================
// ====================================================================

