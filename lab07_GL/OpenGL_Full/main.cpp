#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "MarchingCubes.h"
#include "GridFromTiff.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <filesystem>

// Orbital camera variables
float theta = 0.0f;
float phi = 0.0f;
float radius = 2.0f;
float roll = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
glm::vec3 gridCenter = glm::vec3(0.0f, 0.0f, 0.0f);
// to switch the rendering of the organs
std::vector<bool> organVisibility(26, true);

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// Process input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float rollSpeed = 0.01f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        roll -= rollSpeed;
    else if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        roll += rollSpeed;
    else if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        roll = 0.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= GLFW_KEY_A && key < GLFW_KEY_X && action == GLFW_PRESS) {
        int organIndex = key - GLFW_KEY_A;
        if (organIndex < organVisibility.size()) {
            organVisibility[organIndex] = !organVisibility[organIndex];
        }
    }
}

// Mouse movement callback for orbital rotation
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

    float sensitivity = 0.003f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    theta += xoffset;
    phi += yoffset;

    if (phi > M_PI/2 - 0.05f) phi = M_PI/2 - 0.05f;
    if (phi < -M_PI/2 + 0.05f) phi = -M_PI/2 + 0.05f;
}

// Scroll callback for zooming
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomFactor = 1.0f + (yoffset * 0.5f);
    radius /= zoomFactor;

    // zoom limits
    if (radius < 0.001f) radius = 0.001f;
    if (radius > 100.0f) radius = 100.0f;
}

// Callback to adjust the viewport when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Compile shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

// Create shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL context version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Marching Cubes Visualization", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // Capture the mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set up OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Create shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Define grid
    int gridX, gridY, gridZ, organNum;
    organNum = 0;
    std::vector<std::vector<bool>> organGrids;
    std::vector<std::vector<float>> organCenters;
    std::vector<std::vector<float>> organTriPoints;
    std::vector<glm::vec3> organColors;
    GridFromTiff gridFromTiff;
    MarchingCubes mc;

    std::string folderPath = "../tiff";
    try {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();
                std::replace(filePath.begin(), filePath.end(), '\\', '/');
                organGrids.push_back(std::vector<bool>());
                organTriPoints.push_back(std::vector<float>());
                organCenters.push_back(std::vector<float>());
                gridFromTiff.run(filePath, organGrids[organNum], gridX, gridY, gridZ, organCenters[organNum]);
                mc.run(organTriPoints[organNum], organGrids[organNum], gridX, gridY, gridZ);
                organColors.push_back(glm::vec3((organNum % 3) / 2.0f, ((organNum / 3) % 3) / 2.0f, ((organNum / 9) % 3) / 2.0f));
                organNum++;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // Create VAO and VBO for triPoints
    std::vector<GLuint> organVAOs(organNum), organVBOs(organNum);
    for (int i = 0; i < organNum; ++i) {
        glGenVertexArrays(1, &organVAOs[i]);
        glGenBuffers(1, &organVBOs[i]);
        glBindVertexArray(organVAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, organVBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, organTriPoints[i].size() * sizeof(float), organTriPoints[i].data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    // Axis vertices
    std::vector<float> axisVertices = {
        // X-axis (red)
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        // Y-axis (green)
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        // Z-axis (blue)
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f
    };

    // Create VAO and VBO for axes
    GLuint axisVAO, axisVBO;
    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(float), axisVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        glUseProgram(shaderProgram);

        //Set gridCenter based on the organs visibility
        int visibleOrgans = 0;
        gridCenter = glm::vec3(0.0f, 0.0f, 0.0f);
        for(int i = 0; i < organNum; ++i) {
            if(organVisibility[i]) {
                gridCenter.x += organCenters[i][0];
                gridCenter.y += organCenters[i][1];
                gridCenter.z += organCenters[i][2];
                visibleOrgans++;
            }
        }
        if (visibleOrgans > 0) {
            gridCenter.x /= visibleOrgans;
            gridCenter.y /= visibleOrgans;
            gridCenter.z /= visibleOrgans;
        }

        // Calculate orbital camera position
        float x = gridCenter.x + radius * cos(phi) * cos(theta);
        float y = gridCenter.y + radius * sin(phi);
        float z = gridCenter.z + radius * cos(phi) * sin(theta);
        glm::vec3 cameraPos(x, y, z);

        // Calculate camera orientation, always look at grid center
        glm::vec3 forward = glm::normalize(gridCenter - cameraPos);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // Create a base right vector
        glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
        // If forward is parallel to worldUp, use a different reference
        if (glm::length(right) < 0.1f) {
            right = glm::normalize(glm::cross(forward, glm::vec3(1.0f, 0.0f, 0.0f)));
        }

        // Create base up vector
        glm::vec3 up = glm::normalize(glm::cross(right, forward));

        // Apply roll rotation around the forward vector (view direction)
        glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), roll, forward);
        up = glm::vec3(rollMatrix * glm::vec4(up, 0.0f));

        // Create view matrix
        glm::mat4 view = glm::lookAt(cameraPos, gridCenter, up);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);

        // Set uniform matrices
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Render triangles from each organ
        for (int i = 0; i < organNum; ++i) {
            if(organVisibility[i]) {
                glUniform3f(glGetUniformLocation(shaderProgram, "color"), organColors[i].r, organColors[i].g, organColors[i].b);
                glBindVertexArray(organVAOs[i]);
                glDrawArrays(GL_TRIANGLES, 0, organTriPoints[i].size() / 3);
            }
        }

        // Render axes
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f); // Red for X-axis
        glBindVertexArray(axisVAO);
        glDrawArrays(GL_LINES, 0, 2);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 1.0f, 0.0f); // Green for Y-axis
        glDrawArrays(GL_LINES, 2, 2);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f); // Blue for Z-axis
        glDrawArrays(GL_LINES, 4, 2);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (int i = 0; i < organNum; ++i) {
        glDeleteVertexArrays(1, &organVAOs[i]);
        glDeleteBuffers(1, &organVBOs[i]);
    }
    glDeleteVertexArrays(1, &axisVAO);
    glDeleteBuffers(1, &axisVBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}