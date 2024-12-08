#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class Background {
public:
    Background();
    ~Background();
    bool Initialize(const char* imagePath);
    void Draw(const glm::mat4& view, const glm::mat4& projection);

private:
    GLuint VAO, VBO, EBO;  // Added EBO
    GLuint textureID;
    GLuint shaderProgram;
    GLuint viewLoc, projectionLoc;
    unsigned int indicesCount;
    bool LoadTexture(const char* path);
    bool CreateShaders();
};

#endif