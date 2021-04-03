#include <iostream>
#define GLAD_GL_IMPLEMENTATION
#include "glad.h"
#include "GLFW/glfw3.h"

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

    GLFWwindow* window = glfwCreateWindow(800, 450, "Font viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        std::exit(1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        std::exit(1);
    }

    glClearColor(1, 0, 0, 1);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
