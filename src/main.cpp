#include <iostream>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

const char* vertex_src = R"raw(#version 330 core
layout(location = 0) in vec2 position;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
})raw";

const char* fragment_src = R"raw(#version 330 core
out vec4 color;

void main() {
    color = vec4(0.0, 1.0, 0.0, 1.0);
})raw";

class LineRenderer {
public:
    LineRenderer();
    void drawLines(glm::vec2* points, unsigned int npoints);
private:
    int program;
};

int main()
{
    std::cout << "Hello World!" << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        std::exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(600, 600, "Font viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        std::exit(1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        std::exit(1);
    }

    std::vector<glm::vec2> points(2);
    points[0] = glm::vec2(-0.5, 0.);
    points[1] = glm::vec2(0.5, 0.);

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size()*sizeof(glm::vec2), points.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_src, nullptr);
    glCompileShader(vertex_shader);

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_src, nullptr);
    glCompileShader(fragment_shader);

    int ret;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        std::cerr << "vertex shader compilation failed" << std::endl;
        std::exit(1);
    }
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ret);
    if (!ret) {
        std::cerr << "fragment shader compilation failed" << std::endl;
        std::exit(1);
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glUseProgram(program);

    glClearColor(1, 0, 0, 1);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_LINE_STRIP, 0, 2);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
