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

  if (glfwGetKey(window, GLFW_KEY_W)) curr_y+=0.01;
  else if (glfwGetKey(window, GLFW_KEY_S)) curr_y-=0.01;
  else if (glfwGetKey(window, GLFW_KEY_A)) curr_x-=0.01;
  else if (glfwGetKey(window, GLFW_KEY_D)) curr_x+=0.01;
  else if (glfwGetKey(window, GLFW_KEY_R)) curr_angle += 0.01;
  else if (glfwGetKey(window, GLFW_KEY_O)) cam_z += 0.01;
  else if (glfwGetKey(window, GLFW_KEY_L)) cam_z -= 0.01;
  initializeMVPTransformation();

  // Send our transformation to the currently bound shader, 
  // in the "MVP" uniform
  glUniformMatrix4fv(View_Matrix_ID, 1, GL_FALSE, &V[0][0]);
  glUniformMatrix4fv(Projection_Matrix_ID, 1, GL_FALSE, &P[0][0]);
  glUniformMatrix4fv(Model_Matrix_ID, 1, GL_FALSE, &ground.M[0][0]);
  ground.DrawObject();
  
  obj3.DrawObject();

  updataMovingObjectTransformation();
  glUniformMatrix4fv(Model_Matrix_ID, 1, GL_FALSE, &bunny.M[0][0]);
  bunny.DrawObject();

  // Swap buffers
  glfwSwapBuffers(window);
  glfwPollEvents();
}

void updataMovingObjectTransformation()
{
  bunny.M = glm::rotate(glm::mat4(1.0f), curr_angle, { 1.0f,0.0f,0.0f });
  bunny.M = glm::translate(bunny.M, { curr_x,curr_y,0.0f });
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
    
  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  P = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 10000.0f);
   
  // Camera matrix
  V = glm::lookAt(
    glm::vec3(0, 0, cam_z), // Camera is at (4,3,-3), in World Space
    glm::vec3(0, 0, 0), // and looks at the origin
    glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  
  return true;
  
}

bool initializeVertexbuffer()
{
  //####################### FIRST OBJECT: GROUND ###################
  ground = RenderingObject();
  ground.InitializeVAO();
  
  //create vertex data
  std::vector< glm::vec3 > vertices = std::vector< glm::vec3 >();
  vertices.push_back({-120,-50,-120});
  vertices.push_back({-120,-50,120 });
  vertices.push_back({ 120,-50,120 });
  vertices.push_back({ -120,-50,-120 });
  vertices.push_back({ 120,-50,120 });
  vertices.push_back({ 120,-50,-120 });
  ground.SetVertices(vertices);
  
  //create normal data
  std::vector< glm::vec3 > normals = std::vector< glm::vec3 >();
  ground.computeVertexNormalsOfTriangles(vertices, normals);
  ground.SetNormals(normals);

  //create texture data
  ground.textureSamplerID = glGetUniformLocation(programID, "myTextureSampler");
  float scaling = 5.0f;
  std::vector< glm::vec2 > uvbufferdata;
  uvbufferdata.push_back({ 0.0f, 0.0f });
  uvbufferdata.push_back({ 0.0f, scaling });
  uvbufferdata.push_back({ scaling, scaling });
  uvbufferdata.push_back({ 0.0f,0.0f });
  uvbufferdata.push_back({ scaling,scaling });
  uvbufferdata.push_back({ scaling,0.0f });
  ground.SetTexture(uvbufferdata, "brick_2.bmp"); 

  //####################### SECOND OBJECT: BUNNY ###################
  bunny = RenderingObject();
  bunny.InitializeVAO();
  bunny.LoadSTL("sphere.stl");

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


