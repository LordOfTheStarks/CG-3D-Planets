#ifndef PLAYGROUND_H
#define PLAYGROUND_H

// Include GLEW
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>

// Include GLFW
#include <glfw3.h>

#include <vector>
#include <playground/parse_stl.h>

#include "RenderingObject.h"

// Camera variables
extern glm::vec3 camera_position;
extern glm::vec3 camera_target;
extern glm::vec3 camera_up;
extern float camera_fov;
extern float camera_speed;
extern float mouse_sensitivity;
extern const float EARTH_SUN_DISTANCE;

//program ID of the shaders, required for handling the shaders with OpenGL
GLuint programID;

//global variables to handle the MVP matrix
GLuint View_Matrix_ID;
glm::mat4 V;
GLuint Projection_Matrix_ID;
glm::mat4 P;
GLuint Model_Matrix_ID;
GLuint SunPosition_worldspace_ID;
GLuint IsSun_ID;

RenderingObject ground;
RenderingObject earth;
RenderingObject moon;
RenderingObject sun;

float curr_x;
float curr_y;
float curr_angle;

float cam_z;


int main( void ); //<<< main function, called at startup
void updateAnimationLoop(); //<<< updates the animation loop
bool initializeWindow(); //<<< initializes the window using GLFW and GLEW
bool initializeMVPTransformation();
bool initializeVertexbuffer(); //<<< initializes the vertex buffer array and binds it OpenGL
bool cleanupVertexbuffer(); //<<< frees all recources from the vertex buffer
bool closeWindow(); //<<< Closes the OpenGL window and terminates GLFW

void updataMovingObjectTransformation();

// Camera functions
void updateCamera(GLFWwindow* window);
void handleKeyInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void handleTimeControls(GLFWwindow* window);


#endif
