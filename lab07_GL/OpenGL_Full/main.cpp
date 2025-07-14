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
float theta = 0.0f;        // Horizontal rotation (around Y-axis)
float phi = 0.0f;          // Vertical rotation (elevation)
float radius = 2.0f;      // Distance from center (increased for better view)
float roll = 0.0f;         // Roll rotation around view direction
float lastX = 400, lastY = 300;
bool firstMouse = true;
glm::vec3 gridCenter = glm::vec3(5.0f, 5.0f, 5.0f); // Center of the grid

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

    // Clockwise and counterclockwise rotation
    float rollSpeed = 0.01f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        roll -= rollSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        roll += rollSpeed;

    // Reset roll with R key
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        roll = 0.0f;
}

// Mouse movement callback for orbital rotation
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    // Sensitivity smooth and responsive
    float sensitivity = 0.003f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Horizontal mouse movement rotates around Y axis (theta)
    theta += xoffset;

    // Vertical mouse movement changes elevation (phi)
    phi += yoffset;

    // Clamp phi to prevent flipping over the poles
    if (phi > M_PI/2 - 0.05f) phi = M_PI/2 - 0.05f;
    if (phi < -M_PI/2 + 0.05f) phi = -M_PI/2 + 0.05f;
}

// Scroll callback for zooming
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomFactor = 1.0f + (yoffset * 0.1f);
    radius /= zoomFactor;

    // Clamp zoom limits
    //if (radius < 2.0f) radius = 2.0f;
    //if (radius > 100.0f) radius = 100.0f;
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

    // Capture the mouse for smooth interaction
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

    // Define grid and run Marching Cubes
    int gridX, gridY, gridZ, organNum;
    organNum = 0;
    std::vector<std::vector<bool>> organGrids;
    GridFromTiff gridFromTiff;
    organGrids.push_back(std::vector<bool>());
    gridFromTiff.run("../skeletonMasks.tiff", organGrids[organNum], gridX, gridY, gridZ);
    //recalculate center of object, which is the center of all the points in true
    int count = 0;
    for (int x = 0; x < gridX; ++x) {
        for (int y = 0; y < gridY; ++y) {
            for (int z = 0; z < gridZ; ++z) {
                if (organGrids[organNum][INDEX(x, y, z, gridX, gridY)]) {
                    gridCenter.x += x;
                    gridCenter.y += y;
                    gridCenter.z += z;
                    count++;
                }
            }
        }
    }
    if (count > 0) {
        gridCenter.x /= count;
        gridCenter.y /= count;
        gridCenter.z /= count;
        gridCenter.x = (gridCenter.x / gridX) * 2.0f - 1.0f; // Scale to [-1, 1]
        gridCenter.y = (gridCenter.y / gridY) * 2.0f - 1.0f; // Scale to [-1, 1]
        gridCenter.z = (gridCenter.z / gridZ) * 2.0f - 1.0f; // Scale to [-1, 1]
    } else {
        std::cerr << "No points found in the grid." << std::endl;
        return -1;
    }
    // Marching Cubes
    std::vector<std::vector<float>> organTriPoints;
    std::vector<glm::vec3> organColors;
    MarchingCubes mc;
    organTriPoints.push_back(std::vector<float>());
    mc.run(organTriPoints[organNum], organGrids[organNum], gridX, gridY, gridZ);
    organColors.push_back(glm::vec3(1.0f, 0.8f, 0.6f));
    organNum++;
    std::string folderPath = "../tiff"; // Path to the folder
    try {
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();
                std::replace(filePath.begin(), filePath.end(), '\\', '/');
                organGrids.push_back(std::vector<bool>());
                organTriPoints.push_back(std::vector<float>());
                gridFromTiff.run(filePath, organGrids[organNum], gridX, gridY, gridZ);
                mc.run(organTriPoints[organNum], organGrids[organNum], gridX, gridY, gridZ);
                organColors.push_back(glm::vec3(static_cast<float>(organNum) / 16.0f, 0.5f, 0.5f));
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
    /*
    GLuint triVAO, triVBO;
    glGenVertexArrays(1, &triVAO);
    glGenBuffers(1, &triVBO);
    glBindVertexArray(triVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triVBO);
    glBufferData(GL_ARRAY_BUFFER, triPoints.size() * sizeof(float), triPoints.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
*/
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
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), organColors[i].r, organColors[i].g, organColors[i].b);
            glBindVertexArray(organVAOs[i]);
            glDrawArrays(GL_TRIANGLES, 0, organTriPoints[i].size() / 3);
        }
        /*
        if (triPoints.size() > 0) {
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f);
            glBindVertexArray(triVAO);
            glDrawArrays(GL_TRIANGLES, 0, triPoints.size() / 3);
        }*/

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