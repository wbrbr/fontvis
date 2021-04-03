#include <iostream>
#include <vector>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H
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

struct OutlineState {
    std::vector<std::vector<glm::vec2>> lines;
};

int move_to(const FT_Vector* to, void* user) {
    std::cerr << "move_to" << std::endl;
    OutlineState* state = static_cast<OutlineState*>(user);
    state->lines.push_back(std::vector<glm::vec2>());
    state->lines.back().push_back(glm::vec2(to->x, to->y));
    return 0;
}

int line_to(const FT_Vector* to, void* user) {
    std::cerr << "line_to" << std::endl;
    OutlineState* state = static_cast<OutlineState*>(user);
    state->lines.back().push_back(glm::vec2(to->x, to->y));
    return 0;
}

int conic_to(const FT_Vector* control, const FT_Vector* to, void* user) {
    (void)control;
    std::cerr << "Warning: conic curve" << std::endl;
    OutlineState* state = static_cast<OutlineState*>(user);
    state->lines.back().push_back(glm::vec2(to->x, to->y));
    return 0;
}

int cubic_to(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user) {
    (void)control1;
    (void)control2;
    std::cerr << "Warning: cubic curve" << std::endl;
    OutlineState* state = static_cast<OutlineState*>(user);
    state->lines.back().push_back(glm::vec2(to->x, to->y));
    return 0;
}

int main()
{
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


    FT_Library ft_lib;
    FT_Error err = FT_Init_FreeType(&ft_lib);
    if (err) {
        std::cerr << "Failed to init FreeType" << std::endl;
        std::exit(1);
    }

    FT_Face face;
    err = FT_New_Face(ft_lib, "font.ttf", 0, &face);
    if (err) {
        std::cerr << FT_Error_String(err) << std::endl;
        std::exit(1);
    }

    std::cout << "Num glyphs: " << face->num_glyphs << std::endl;

    err = FT_Set_Char_Size(face, 0, 16*64, 300, 300);
    if (err) {
        std::cerr << FT_Error_String(err) << std::endl;
        std::exit(1);
    }


    unsigned int glyph_index = FT_Get_Char_Index(face, 'A');

    err = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);

    assert(face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);

    std::cout << "N. points: " << face->glyph->outline.n_points << std::endl;

    FT_Outline_Funcs outline_funcs;
    outline_funcs.move_to = move_to;
    outline_funcs.line_to = line_to;
    outline_funcs.conic_to = conic_to;
    outline_funcs.cubic_to = cubic_to;
    outline_funcs.shift = 0;
    outline_funcs.delta = 0;

    OutlineState st;

    FT_Outline_Decompose(&face->glyph->outline, &outline_funcs, &st);
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
