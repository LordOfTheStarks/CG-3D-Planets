#ifndef RENDERING_OBJECT_H
#define RENDERING_OBJECT_H
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "playground/parse_stl.h"


class RenderingObject
{
public:
	RenderingObject();
	virtual ~RenderingObject();

  void InitializeVAO();
  void SetVertices(std::vector< glm::vec3 >);
  void SetNormals(std::vector< glm::vec3 >);
  void SetTexture(std::vector< glm::vec2 >, GLubyte texturedata[]);
  void SetTexture(std::vector< glm::vec2 >, std::string bmpPath);
  void DrawObject();
  void LoadSTL(std::string);

  //vertex array object (VAO)
  GLuint VertexArrayID;
  int VertexBufferSize;

  //vertices VBO / 3D object
  GLuint vertexbuffer;

  //normals VBO
  GLuint normalbuffer;
  
  //texture
  GLuint uvbuffer;
  GLuint texID;
  GLuint textureSamplerID;
  bool texture_present;

  //Model matrix: moves object from model to world space
  glm::mat4 M;

  /**
  * Computes the normals of triangles by computing the cross product.
  * @param[in] vertices   Vector of vertices, 3 vertices represent one triangle.
  * @param[out] normals   Vector of normals, needs to be empty and is filled by the function.
  */
  void computeVertexNormalsOfTriangles(std::vector< glm::vec3 >& vertices, std::vector< glm::vec3 >& normals);

protected:

  std::vector<glm::vec3> getAllTriangleNormalsForVertex(stl::point vertex, std::vector<stl::triangle> triangles);
  glm::vec3 computeMeanVector(std::vector<glm::vec3>);
  

};

#endif
