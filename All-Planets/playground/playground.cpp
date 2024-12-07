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

glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 500.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
float camera_fov = 45.0f;
float camera_speed = 100.0f;  // Adjust this value to change movement speed
float mouse_sensitivity = 0.1f;

void handleKeyInput(GLFWwindow* window) {
    float deltaTime = 1.0f / 60.0f; // Assuming 60 FPS, you can implement proper timing if needed

    // Forward/Backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_position += camera_speed * deltaTime * glm::normalize(camera_target - camera_position);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_position -= camera_speed * deltaTime * glm::normalize(camera_target - camera_position);

    // Left/Right strafing
    glm::vec3 right = glm::normalize(glm::cross(camera_target - camera_position, camera_up));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_position -= right * camera_speed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_position += right * camera_speed * deltaTime;

    // Up/Down
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_position += camera_up * camera_speed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera_position -= camera_up * camera_speed * deltaTime;
}

void updateCamera(GLFWwindow* window) {
    handleKeyInput(window);

    // Update view matrix
    V = glm::lookAt(
        camera_position,
        camera_target,
        camera_up
    );
}

// Earth rotation parameters (based on real data)
const float EARTH_ROTATION_PERIOD = 24.0f; // hours
const float EARTH_ORBIT_PERIOD = 365.0f; // days

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
    // Update camera
    updateCamera(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programID);

    // Set the texture sampler uniform
    glUniform1i(earth.textureSamplerID, 0);

    // Increase rotation speed (adjust multiplier as needed)
    float speedMultiplier = 5000.0f; // Makes it 5 times faster
    float rotation_per_frame = ((2 * 3.14159f) / (EARTH_ROTATION_PERIOD * 3600 * 60.0f)) * speedMultiplier;
    earth_rotation_angle += rotation_per_frame;

    // Reset transformation matrix
    earth.M = glm::mat4(1.0f);

    // Reduce orbit radius for better view
    float orbit_radius = 150.0f; // Was 300.0f
    float x = orbit_radius * cos(earth_orbit_angle);
    float z = orbit_radius * sin(earth_orbit_angle);

    // First translate to orbit position
    earth.M = glm::translate(earth.M, glm::vec3(x, 0.0f, z));

    // Apply rotation with proper axis
    glm::mat4 rotation_matrix = glm::rotate(
        glm::mat4(1.0f),
        earth_rotation_angle,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Apply axial tilt
    glm::mat4 tilt_matrix = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(23.5f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    earth.M = earth.M * tilt_matrix * rotation_matrix;

    // Update uniforms with new camera view
    glUniformMatrix4fv(View_Matrix_ID, 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(Projection_Matrix_ID, 1, GL_FALSE, &P[0][0]);
    glUniformMatrix4fv(Model_Matrix_ID, 1, GL_FALSE, &earth.M[0][0]);

    earth.DrawObject();

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
  // Set up input handling
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // Hide and capture cursor

  return true;
}

bool initializeMVPTransformation()
{
    // Get handles for uniforms.
    Model_Matrix_ID = glGetUniformLocation(programID, "M");
    Projection_Matrix_ID = glGetUniformLocation(programID, "P");
    View_Matrix_ID = glGetUniformLocation(programID, "V");

    // Adjust the field of view to 45 degrees for more natural perspective
    P = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 10000.0f);

    // Adjust camera position for better view
    V = glm::lookAt(
        glm::vec3(0, 0, 500),    // Camera position (removed Y offset)
        glm::vec3(0, 0, 0),      // Look at the origin
        glm::vec3(0, 1, 0)       // Head is up (fixed up vector)
    );

    return true;
}

bool initializeVertexbuffer()
{
    // Earth object
    earth = RenderingObject();
    earth.InitializeVAO();
    earth.LoadSTL("sphere.stl");

    // Remove the manual UV creation as it's now handled in LoadSTL
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