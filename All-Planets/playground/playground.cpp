#include "playground.h"

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLM
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <playground/RenderingObject.h>

// Earth rotation parameters (based on real data)
const float EARTH_ROTATION_PERIOD = 23.934f; // hours
const float EARTH_ORBIT_PERIOD = 365.256f; // days

float earth_rotation_angle = 0.0f;
float earth_orbit_angle = 0.0f;

// Mars rotation parameters (based on real data)
const float MARS_ROTATION_PERIOD = 24.622f; // hours
const float MARS_ORBIT_PERIOD = 686.971f; // days

float mars_rotation_angle = 0.0f;
float mars_orbit_angle = 0.0f;

int main( void )
{
  //Initialize window
  bool windowInitialized = initializeWindow();
  if (!windowInitialized) return -1;

  //Initialize vertex buffer
  bool vertexbufferInitialized = initializeVertexbuffer();
  if (!vertexbufferInitialized) return -1;

  // Create and compile our GLSL program from the shaders
  programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

  initializeMVPTransformation();

  curr_x = 0;
  curr_y = 0;

  cam_z = 300;

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);


  initializeMVPTransformation();

	//start animation loop until escape key is pressed
	do{

    updateAnimationLoop();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	
  //Cleanup and close window
  cleanupVertexbuffer();
  glDeleteProgram(programID);
	closeWindow();
  
	return 0;
}

void updateAnimationLoop()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    // Rotation rate: one full rotation per 24 hours (in radians per frame)
    // Assuming 60 FPS
    float rotation_per_frame = (2 * 3.14159f) / (24 * 3600 * 60.0f);
    earth_rotation_angle += rotation_per_frame;

    // Orbit rate: one full orbit per 365.256 days
    float orbit_per_frame = (2 * 3.14159f) / (365.256f * 24 * 3600 * 60.0f);
    earth_orbit_angle += orbit_per_frame;

    // Reset transformation matrix
    earth.M = glm::mat4(1.0f);

    // Orbit around the sun (along a circular path)
    float orbit_radius = 300.0f;
    float x = orbit_radius * cos(earth_orbit_angle);
    float z = orbit_radius * sin(earth_orbit_angle);

    // First, translate to orbit position
    earth.M = glm::translate(earth.M, glm::vec3(x, 0.0f, z));

    // Then, rotate on its own axis 
    // Tilted axis: Earth's axial tilt is about 23.5 degrees
    glm::mat4 rotation_matrix = glm::rotate(
        glm::mat4(1.0f),
        earth_rotation_angle,
        glm::vec3(0.0f, 1.0f, 0.0f)  // Rotation around y-axis
    );

    // Combine the rotation with a slight tilt
    glm::mat4 tilt_matrix = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(23.5f),  // Earth's axial tilt
        glm::vec3(1.0f, 0.0f, 0.0f)  // Tilt around x-axis
    );

    earth.M = earth.M * tilt_matrix * rotation_matrix;

    // Send our transformation to the currently bound shader
    glUniformMatrix4fv(View_Matrix_ID, 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(Projection_Matrix_ID, 1, GL_FALSE, &P[0][0]);
    glUniformMatrix4fv(Model_Matrix_ID, 1, GL_FALSE, &earth.M[0][0]);

    // Draw Earth
    earth.DrawObject();

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}


void updataMovingObjectTransformation()
{
    earth.M = glm::rotate(glm::mat4(1.0f), curr_angle, { 1.0f,0.0f,0.0f });
    earth.M = glm::translate(earth.M, { curr_x,curr_y,0.0f });
}

bool initializeWindow()
{
  // Initialise GLFW
  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    getchar();
    return false;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(1024, 768, "Example: simple cube", NULL, NULL);
  if (window == NULL) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    getchar();
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    return false;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  return true;
}

bool initializeMVPTransformation()
{
    // Get a handle for our "MVP" uniform
    Model_Matrix_ID = glGetUniformLocation(programID, "M");
    Projection_Matrix_ID = glGetUniformLocation(programID, "P");
    View_Matrix_ID = glGetUniformLocation(programID, "V");

    if (Model_Matrix_ID == -1 || Projection_Matrix_ID == -1 || View_Matrix_ID == -1) {
        std::cerr << "Error: Invalid uniform location!" << std::endl;
        exit(-1);
    }


    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 10000 units
    P = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 10000.0f);

    // Camera matrix
    V = glm::lookAt(
        glm::vec3(0, 200, cam_z),  // Slightly raised camera
        glm::vec3(0, 0, 0),        // Looking at the origin
        glm::vec3(0, 1, 1)         // Head is up
    );

    return true;
}

bool initializeVertexbuffer()
{
    // Earth object
    earth = RenderingObject();
    earth.InitializeVAO();
    earth.LoadSTL("sphere.stl"); // Assuming you have a sphere.stl file

    // Create texture coordinates for sphere
    std::vector< glm::vec2 > uvbufferdata;
    for (int i = 0; i < 6; i++) {
        uvbufferdata.push_back({ 0.0f, 0.0f });
        uvbufferdata.push_back({ 1.0f, 0.0f });
        uvbufferdata.push_back({ 1.0f, 1.0f });
    }
    earth.SetTexture(uvbufferdata, "2k_earth_daymap.bmp");

    return true;
}

bool cleanupVertexbuffer()
{
  // Cleanup VBO
  glDeleteVertexArrays(1, &ground.VertexArrayID);
  return true;
}

bool closeWindow()
{
  glfwTerminate();
  return true;
}


