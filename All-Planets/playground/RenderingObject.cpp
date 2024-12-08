#include "RenderingObject.h"
#include <common/texture.hpp>
#include <playground/parse_stl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RenderingObject::RenderingObject() : texture_present(false), M(glm::mat4(1.0f)) {
    uvbufferdata = std::vector<glm::vec2>();  // Initialize empty vector
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

void RenderingObject::SetTexture(std::vector<glm::vec2> uvbufferdata, std::string bmpPath) {
    texture_present = true;

    // Generate and bind texture
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Load texture from file
    texID = loadBMP_custom(bmpPath.c_str());

    // Set texture parameters
    glBindTexture(GL_TEXTURE_2D, texID);

    // Change back to GL_REPEAT for proper wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set up UV buffer
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvbufferdata.size() * sizeof(glm::vec2), &uvbufferdata[0], GL_STATIC_DRAW);

    // Enable texture coordinates attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

void RenderingObject::DrawObject()
{
    if (VertexBufferSize == 0) {
        return;  // Don't draw if there are no vertices
    }
  
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

  if (texture_present && !uvbufferdata.empty())
  {
      // Bind texture before setting up the attribute
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texID);

      // Enable and bind UV coordinates
      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
      glVertexAttribPointer(
          2,                  // attribute 2
          2,                  // size (u, v)
          GL_FLOAT,          // type
          GL_FALSE,          // normalized?
          0,                 // stride
          (void*)0           // array buffer offset
      );
  }

  // Draw
  glDrawArrays(GL_TRIANGLES, 0, VertexBufferSize / sizeof(glm::vec3));

  // Disable vertex arrays
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  if (texture_present) {
      glDisableVertexAttribArray(2);
  }
}

void RenderingObject::LoadSTL(std::string stl_file_name) {
    auto info = stl::parse_stl(stl_file_name);
    std::vector<stl::triangle> triangles = info.triangles;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    uvbufferdata.clear();  // Clear any existing UV data

    // Calculate bounding box to determine scaling
    float min_x = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float min_y = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::lowest();
    float min_z = std::numeric_limits<float>::max();
    float max_z = std::numeric_limits<float>::lowest();

    // First pass: find bounding box
    for (auto t : triangles) {
        min_x = std::min({ min_x, t.v1.x, t.v2.x, t.v3.x });
        max_x = std::max({ max_x, t.v1.x, t.v2.x, t.v3.x });
        min_y = std::min({ min_y, t.v1.y, t.v2.y, t.v3.y });
        max_y = std::max({ max_y, t.v1.y, t.v2.y, t.v3.y });
        min_z = std::min({ min_z, t.v1.z, t.v2.z, t.v3.z });
        max_z = std::max({ max_z, t.v1.z, t.v2.z, t.v3.z });
    }

    // Calculate scaling factor
    float scale_x = max_x - min_x;
    float scale_y = max_y - min_y;
    float scale_z = max_z - min_z;
    float max_scale = std::max({ scale_x, scale_y, scale_z });
    float scaling_factor = 150.0f / max_scale;

    // Calculate center
    float center_x = (min_x + max_x) / 2.0f;
    float center_y = (min_y + max_y) / 2.0f;
    float center_z = (min_z + max_z) / 2.0f;

    for (auto t : triangles) {
        // Add vertices
        glm::vec3 v1((t.v1.x - center_x) * scaling_factor,
            (t.v1.y - center_y) * scaling_factor,
            (t.v1.z - center_z) * scaling_factor);
        glm::vec3 v2((t.v2.x - center_x) * scaling_factor,
            (t.v2.y - center_y) * scaling_factor,
            (t.v2.z - center_z) * scaling_factor);
        glm::vec3 v3((t.v3.x - center_x) * scaling_factor,
            (t.v3.y - center_y) * scaling_factor,
            (t.v3.z - center_z) * scaling_factor);

        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);

        // Normalize vertices for UV calculation
        glm::vec3 n1 = glm::normalize(v1);
        glm::vec3 n2 = glm::normalize(v2);
        glm::vec3 n3 = glm::normalize(v3);

        // Generate UV coordinates
        glm::vec2 uv1 = generateUV(n1);
        glm::vec2 uv2 = generateUV(n2);
        glm::vec2 uv3 = generateUV(n3);

        // Fix texture seam if needed
        if (isSeamVertex(n1) || isSeamVertex(n2) || isSeamVertex(n3)) {
            float u1 = uv1.x, u2 = uv2.x, u3 = uv3.x;
            if (std::abs(u1 - u2) > 0.5f) {
                if (u1 < u2) u1 += 1.0f;
                else u2 += 1.0f;
            }
            if (std::abs(u2 - u3) > 0.5f) {
                if (u2 < u3) u2 += 1.0f;
                else u3 += 1.0f;
            }
            if (std::abs(u3 - u1) > 0.5f) {
                if (u3 < u1) u3 += 1.0f;
                else u1 += 1.0f;
            }
            uv1.x = u1;
            uv2.x = u2;
            uv3.x = u3;
        }

        // Store UV coordinates directly in member variable
        uvbufferdata.push_back(uv1);
        uvbufferdata.push_back(uv2);
        uvbufferdata.push_back(uv3);
    }

    this->SetVertices(vertices);
    computeVertexNormalsOfTriangles(vertices, normals);
    this->SetNormals(normals);
}
bool RenderingObject::isSeamVertex(const glm::vec3& normalized) {
    // Check if vertex is near the texture seam
    float angle = atan2(normalized.z, normalized.x);
    float threshold = 0.005f; // Smaller threshold for more precise seam detection
    return std::abs(std::abs(angle) - M_PI) < threshold;
}

glm::vec2 RenderingObject::generateUV(const glm::vec3& vertex) {
    // Normalize the vertex
    glm::vec3 normalized = glm::normalize(vertex);

    // Calculate spherical coordinates
    float u = 0.5f + (atan2(normalized.z, normalized.x) / (2.0f * M_PI));
    float v = 0.5f + (asin(normalized.y) / M_PI);

    // Ensure UV coordinates are properly wrapped
    u = 1.0f - u; // Flip the U coordinate to fix the mirroring

    // More precise wrapping
    if (u > 1.0f) u -= floor(u);
    if (u < 0.0f) u += ceil(-u);

    v = glm::clamp(v, 0.0f, 1.0f);

    return glm::vec2(u, v);
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