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

// Camera variables
float yaw = -90.0f;    // Horizontal rotation
float pitch = 0.0f;     // Vertical rotation
float lastX = 1024.0f / 2.0f;   // Screen center X
float lastY = 768.0f / 2.0f;    // Screen center Y
bool firstMouse = true;

glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 500.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
float camera_fov = 45.0f;
float camera_speed = 50.0f;  // Adjust this value to change movement speed
float mouse_sensitivity = 0.1f;

void handleKeyInput(GLFWwindow* window) {
    float deltaTime = 1.0f / 60.0f;

    // Calculate the camera's right vector
    glm::vec3 right = glm::normalize(glm::cross(camera_front, camera_up));

    float actualSpeed = camera_speed * deltaTime;

    // Forward/Backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_position += actualSpeed * camera_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_position -= actualSpeed * camera_front;

    // Left/Right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_position -= right * actualSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_position += right * actualSpeed;

    // Up/Down
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_position += camera_up * actualSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera_position -= camera_up * actualSpeed;

    // Update camera target based on position and front vector
    camera_target = camera_position + camera_front;
}

void updateCamera(GLFWwindow* window) {
    handleKeyInput(window);

    // Update view matrix
    V = glm::lookAt(camera_position, camera_target, camera_up);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update camera front vector
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_front = glm::normalize(direction);

    // Update camera target
    camera_target = camera_position + camera_front;
}
// Constants for real-world time periods (in days for better readability)
const float DAYS_PER_EARTH_ROTATION = 1.0f;          // Earth rotates once per day
const float DAYS_PER_EARTH_YEAR = 365.25f;          // Days per Earth year
const float DAYS_PER_MOON_ORBIT = 27.322f;          // Moon's sidereal period in days
const float PI = 3.14159f;

// Convert days to seconds for internal calculations
const float SECONDS_PER_DAY = 86400.0f;  // 24 * 60 * 60

// Visualization parameters
const float TIME_SCALE = 10000.0f;               // Simulation runs 1000x faster than real time
const float MOON_ORBIT_DISTANCE = 200.0f;       // Visible distance between Earth and Moon
const float MOON_SCALE = 0.27f;                 // Moon's size relative to Earth

const float SUN_SCALE = 2.0f;  // Sun is about 109 times larger than Earth
const float SUN_ORBIT_DISTANCE = 0.0f;  // Sun stays at center

// Rotation and orbit angles
float earth_rotation_angle = 0.0f;
float earth_orbit_angle = 0.0f;
float moon_rotation_angle = 0.0f;
float moon_orbit_angle = 0.0f;

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

void updateAnimationLoop() {
    updateCamera(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programID);

    // Calculate time step (assuming 60 FPS)
    float deltaTime = 1.0f / 60.0f;
    float simulatedDays = (deltaTime * TIME_SCALE) / SECONDS_PER_DAY;

    // Set common matrices
    glUniformMatrix4fv(View_Matrix_ID, 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(Projection_Matrix_ID, 1, GL_FALSE, &P[0][0]);

    // Set sun position for lighting calculations
    glm::vec3 sunPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glUniform3fv(SunPosition_worldspace_ID, 1, &sunPosition[0]);

    // Draw Sun
    sun.M = glm::mat4(1.0f);
    sun.M = glm::scale(sun.M, glm::vec3(SUN_SCALE));
    glUniformMatrix4fv(Model_Matrix_ID, 1, GL_FALSE, &sun.M[0][0]);
    glUniform1i(IsSun_ID, 1);  // This is the sun
    sun.DrawObject();

    // Earth's rotation and orbit
    earth_rotation_angle += (2.0f * PI * simulatedDays) / DAYS_PER_EARTH_ROTATION;
    earth_orbit_angle += (2.0f * PI * simulatedDays) / DAYS_PER_EARTH_YEAR;

    // Calculate Earth's position
    float earth_orbit_radius = 400.0f; // Increased for better scale
    float earth_x = earth_orbit_radius * cos(earth_orbit_angle);
    float earth_z = earth_orbit_radius * sin(earth_orbit_angle);

    // Update Earth's transformation
    earth.M = glm::mat4(1.0f);
    earth.M = glm::translate(earth.M, glm::vec3(earth_x, 0.0f, earth_z));
    earth.M = earth.M * glm::rotate(glm::mat4(1.0f), glm::radians(23.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    earth.M = earth.M * glm::rotate(glm::mat4(1.0f), earth_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));

    // Draw Earth
    glUniformMatrix4fv(Model_Matrix_ID, 1, GL_FALSE, &earth.M[0][0]);
    glUniform1i(IsSun_ID, 0);  // This is not the sun
    earth.DrawObject();

    // Moon's orbit around Earth
    moon_orbit_angle += (2.0f * PI * simulatedDays) / DAYS_PER_MOON_ORBIT;

    // Calculate Moon's position relative to Earth
    float moon_x = earth_x + MOON_ORBIT_DISTANCE * cos(moon_orbit_angle);
    float moon_z = earth_z + MOON_ORBIT_DISTANCE * sin(moon_orbit_angle);

    // Update Moon's transformation with tidal locking
    moon.M = glm::mat4(1.0f);
    moon.M = glm::translate(moon.M, glm::vec3(moon_x, 0.0f, moon_z));
    moon.M = glm::scale(moon.M, glm::vec3(MOON_SCALE));

    // Calculate Moon's rotation to maintain tidal locking
    glm::vec3 moon_to_earth = glm::normalize(glm::vec3(earth_x, 0.0f, earth_z) - glm::vec3(moon_x, 0.0f, moon_z));
    float moon_facing_angle = atan2(moon_to_earth.z, moon_to_earth.x) + PI / 2.0f;
    moon.M = moon.M * glm::rotate(glm::mat4(1.0f), moon_facing_angle, glm::vec3(0.0f, 1.0f, 0.0f));

    // Draw Moon
    glUniformMatrix4fv(Model_Matrix_ID, 1, GL_FALSE, &moon.M[0][0]);
    glUniform1i(IsSun_ID, 0);  // This is not the sun
    moon.DrawObject();

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

  // Set up input handling
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // Hide and capture cursor
  glfwSetCursorPosCallback(window, mouse_callback);  // Add this line

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  return true;
}

bool initializeMVPTransformation()
{
    // Get handles for uniforms.
    Model_Matrix_ID = glGetUniformLocation(programID, "M");
    Projection_Matrix_ID = glGetUniformLocation(programID, "P");
    View_Matrix_ID = glGetUniformLocation(programID, "V");

    SunPosition_worldspace_ID = glGetUniformLocation(programID, "SunPosition_worldspace");
    IsSun_ID = glGetUniformLocation(programID, "isSun");

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

bool initializeVertexbuffer() {

    // Sun object
    sun = RenderingObject();
    sun.InitializeVAO();
    sun.LoadSTL("sphere.stl");
    std::vector<glm::vec2> sunUV = sun.GetUVBuffer();
    if (!sunUV.empty()) {
        sun.SetTexture(sunUV, "2k_sun.bmp");
    }

    // Earth object
    earth = RenderingObject();
    earth.InitializeVAO();
    earth.LoadSTL("sphere.stl");

    std::vector<glm::vec2> earthUV = earth.GetUVBuffer();
    if (!earthUV.empty()) {
        earth.SetTexture(earthUV, "2k_earth_daymap.bmp");
    }

    // Moon object
    moon = RenderingObject();
    moon.InitializeVAO();
    moon.LoadSTL("sphere.stl");

    // Set moon texture after loading STL
    std::vector<glm::vec2> moonUV = moon.GetUVBuffer();
    if (!moonUV.empty()) {
        moon.SetTexture(moonUV, "2k_moon.bmp");
    }

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