#include "background.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <cmath>
#include <common/shader.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Background::Background() :
    VAO(0),
    VBO(0),
    EBO(0),
    textureID(0),
    shaderProgram(0),
    viewLoc(0),
    projectionLoc(0),
    indicesCount(0)
{}

Background::~Background() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO); 
    if (textureID != 0) glDeleteTextures(1, &textureID);
    if (shaderProgram != 0) glDeleteProgram(shaderProgram);
}

bool Background::Initialize(const char* imagePath) {
    // Create and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Simple quad vertices
    float vertices[] = {
        // positions          // texture coords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f
    };

    // Create and bind VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Load shaders
    shaderProgram = LoadShaders("BackgroundVertexShader.vertexshader", "BackgroundFragmentShader.fragmentshader");
    if (shaderProgram == 0) {
        std::cout << "Failed to load background shaders" << std::endl;
        return false;
    }

    // Load texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Load image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        return true;
    }

    std::cout << "Failed to load texture: " << imagePath << std::endl;
    return false;
}

void Background::Draw(const glm::mat4& view, const glm::mat4& projection) {
    // Save states
    GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);

    // Disable depth testing
    glDisable(GL_DEPTH_TEST);

    // Use shader and bind texture
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set uniform
    glUniform1i(glGetUniformLocation(shaderProgram, "backgroundTexture"), 0);

    // Draw background quad
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Restore state
    if (depthTestWasEnabled)
        glEnable(GL_DEPTH_TEST);
}

bool Background::LoadTexture(const char* path) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        std::cout << "Background texture loaded successfully: " << width << "x" << height << " channels: " << nrChannels << std::endl;
        return true;
    }
    if (!data) {
        std::cout << "Failed to load texture at path: " << path << std::endl;
        std::cout << "Error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    std::cout << "Failed to load texture: " << path << std::endl;
    return false;
}

bool Background::CreateShaders() {
    shaderProgram = LoadShaders(
        "BackgroundVertexShader.vertexshader",
        "BackgroundFragmentShader.fragmentshader"
    );

    if (shaderProgram == 0) {
        std::cout << "Failed to create background shader program" << std::endl;
        return false;
    }

    return true;
}