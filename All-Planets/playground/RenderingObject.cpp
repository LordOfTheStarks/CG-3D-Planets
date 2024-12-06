#include "RenderingObject.h"
#include <common/texture.hpp>
#include <playground/parse_stl.h>

RenderingObject::RenderingObject() : texture_present(false), M(glm::mat4(1.0f))
{
  
}
RenderingObject::~RenderingObject() {}

void RenderingObject::InitializeVAO()
{
  glGenVertexArrays(1, &VertexArrayID);
}

void RenderingObject::SetVertices(std::vector< glm::vec3 > vertices)
{
  glBindVertexArray(VertexArrayID);
  glGenBuffers(1, &vertexbuffer);
  VertexBufferSize = vertices.size() * sizeof(glm::vec3);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, VertexBufferSize, &vertices[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void RenderingObject::SetNormals(std::vector< glm::vec3 > normals)
{
  glBindVertexArray(VertexArrayID);
  glGenBuffers(1, &normalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void RenderingObject::SetTexture(std::vector< glm::vec2 > uvbufferdata, GLubyte texturedata[])
{
  texture_present = true;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTextureStorage2D(texID, 4, GL_R8, 8, 8);
  glTextureSubImage2D(texID,           //Texture
    0,                            //First mipmap level
    0, 0,                         //x and y offset
    8, 8,                         //width and height
    GL_RED, GL_UNSIGNED_BYTE,     //format and data type
    texturedata         //data
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvbufferdata.size() * sizeof(glm::vec2), &uvbufferdata[0], GL_STATIC_DRAW);
}

void RenderingObject::SetTexture(std::vector< glm::vec2 > uvbufferdata, std::string bmpPath)
{
  texture_present = true;
  glGenTextures(1, &texID);
  texID = loadBMP_custom(bmpPath.c_str());
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, uvbufferdata.size() * sizeof(glm::vec2), &uvbufferdata[0], GL_STATIC_DRAW);
}

void RenderingObject::DrawObject()
{
  
  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(
    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  // 2nd attribute buffer : color
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
  glVertexAttribPointer(
    1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  if (texture_present)
  {
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(textureSamplerID, 0);

    // 2nd attribute buffer : texture
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
      2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      2,                                // size : U+V => 2
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
    );
  }

  // Draw the triangle !
  glDrawArrays(GL_TRIANGLES, 0, VertexBufferSize); // 3 indices starting at 0 -> 1 triangle

  glDisableVertexAttribArray(0);
}

void RenderingObject::LoadSTL(std::string stl_file_name)
{
  //load stl file and construct vertex buffer
  auto info = stl::parse_stl(stl_file_name);
  std::vector<stl::triangle> triangles = info.triangles;
  std::vector< glm::vec3 > vertices;
  std::vector< glm::vec3 > normals;
  for (auto t : info.triangles) {
    vertices.push_back(glm::vec3(t.v1.x, t.v1.y, t.v1.z));
    vertices.push_back(glm::vec3(t.v2.x, t.v2.y, t.v2.z));
    vertices.push_back(glm::vec3(t.v3.x, t.v3.y, t.v3.z));
  }
  this->SetVertices(vertices);
  computeVertexNormalsOfTriangles(vertices, normals);
  this->SetNormals(normals);

}

std::vector<glm::vec3> RenderingObject::getAllTriangleNormalsForVertex(stl::point vertex, std::vector<stl::triangle> triangles)
{
  std::vector<glm::vec3> returnValue;
  for (auto t : triangles)
  {
    if (t.v1.equals(vertex) || t.v2.equals(vertex) || t.v3.equals(vertex))
    {
      glm::vec3 normal = { t.normal.x,t.normal.y,t.normal.z };
      returnValue.push_back(normal);
    }
  }
  return returnValue;
}

glm::vec3 RenderingObject::computeMeanVector(std::vector<glm::vec3> vec)
{
  glm::vec3 mean = { 0,0,0 };
  for (auto v : vec)
  {
    mean += v;
  }
  normalize(mean);
  return mean;
}

void RenderingObject::computeVertexNormalsOfTriangles(std::vector< glm::vec3 >& vertices, std::vector< glm::vec3 >& normals)
{
  std::vector<stl::triangle> triangles;
  //compute triangles with normals from vertices
  for (int i = 0; i < vertices.size(); i = i + 3)
  {
    glm::vec3 edge1 = vertices.at(i + 1) - vertices.at(i);
    glm::vec3 edge2 = vertices.at(i + 2) - vertices.at(i);
    glm::vec3 triangleNormal = glm::normalize(glm::cross(edge1, edge2));
    stl::point v1 = { vertices.at(i)[0],vertices.at(i)[1],vertices.at(i)[2] };
    stl::point v2 = { vertices.at(i + 1)[0],vertices.at(i + 1)[1],vertices.at(i + 1)[2] };
    stl::point v3 = { vertices.at(i + 2)[0],vertices.at(i + 2)[1],vertices.at(i + 2)[2] };
    stl::point normal = { triangleNormal[0],triangleNormal[1],triangleNormal[2] };

    stl::triangle currentTriangle = { normal,v1,v2,v3 };
    triangles.push_back(currentTriangle);
  }

  //compute vertex normals
  for (glm::vec3 vertice : vertices)
  {
    stl::point verticeConv = { vertice[0],vertice[1],vertice[2] };
    normals.push_back(computeMeanVector(getAllTriangleNormalsForVertex(verticeConv, triangles)));
  }

}