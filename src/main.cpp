#include <iostream>
#include <vector>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#define GLAD_GL_IMPLEMENTATION
#include "glad.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

static const char* vertex_src = R"raw(#version 330 core
layout(location = 0) in vec2 position;

void main() {
    gl_Position = vec4(2.0 * position - vec2(1.), 0.0, 1.0);
})raw";

static const char* fragment_src = R"raw(#version 330 core
out vec4 color;

void main() {
    color = vec4(0.0, 0.0, 0.0, 1.0);
})raw";

struct LineStrip {
    unsigned int vao, vbo;
    unsigned int n_points;
};

class LineRenderer {
public:
    LineRenderer();

    void drawLineStrip(const LineStrip& strip);
    LineStrip createLineStrip(const glm::vec2* points, unsigned int npoints);

private:
    unsigned int program;
};

LineRenderer::LineRenderer() {
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

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
}

LineStrip LineRenderer::createLineStrip(const glm::vec2 *points, unsigned int npoints) {
    LineStrip strip;
    strip.n_points = npoints;
    glGenVertexArrays(1, &strip.vao);
    glBindVertexArray(strip.vao);

    glGenBuffers(1, &strip.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, strip.vbo);
    glBufferData(GL_ARRAY_BUFFER, npoints*sizeof(glm::vec2), points, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return strip;
}

void LineRenderer::drawLineStrip(const LineStrip& strip) {
    glUseProgram(program);
    glBindVertexArray(strip.vao);
    glDrawArrays(GL_LINE_STRIP, 0, (int)strip.n_points);
    glBindVertexArray(0);
}

struct OutlineState {
    std::vector<std::vector<glm::vec2>> lines;
    float ascender, descender, bearing_x;
};

int move_to(const FT_Vector* to, void* user) {
    OutlineState* state = static_cast<OutlineState*>(user);
    state->lines.push_back(std::vector<glm::vec2>());
    state->lines.back().push_back(glm::vec2(to->x - state->bearing_x, to->y - state->descender) / (state->ascender - state->descender));
    return 0;
}

int line_to(const FT_Vector* to, void* user) {
    OutlineState* state = static_cast<OutlineState*>(user);
    state->lines.back().push_back(glm::vec2(to->x - state->bearing_x, to->y - state->descender) / (state->ascender - state->descender));
    return 0;
}

int conic_to(const FT_Vector* control, const FT_Vector* to, void* user) {
    OutlineState* state = static_cast<OutlineState*>(user);
    glm::vec2 w0 = state->lines.back().back() * (state->ascender - state->descender) + glm::vec2(state->bearing_x, state->descender);
    glm::vec2 w1 = glm::vec2(control->x, control->y);
    glm::vec2 w2 = glm::vec2(to->x, to->y);

    unsigned int N = 30;
    for (unsigned int i = 0; i < N; i++) {
        float t = (float)i / (float)(N-1);
        float mt = 1.f - t;
        glm::vec2 p = mt * mt * w0 + 2 * t * mt * w1 + t * t * w2;
        p.x -= state->bearing_x;
        p.y -= state->descender;
        state->lines.back().push_back(p / (state->ascender - state->descender));
    }

    return 0;
}

int cubic_to(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user) {
    OutlineState* state = static_cast<OutlineState*>(user);
    glm::vec2 w0 = state->lines.back().back() * (state->ascender - state->descender) + glm::vec2(state->bearing_x, state->descender);
    glm::vec2 w1 = glm::vec2(control1->x, control1->y);
    glm::vec2 w2 = glm::vec2(control2->x, control2->y);
    glm::vec2 w3 = glm::vec2(to->x, to->y);

    const unsigned int N = 30;
    for (unsigned int i = 0; i < N; i++) {
        float t = (float)i / (float)(N-1);
        float mt = 1.f - t;

        glm::vec2 p = mt*mt*mt*w0 + 3.f*t*mt*mt*w1 + 3.f*t*t*mt*w2 + t*t*t*w3;
        p.x -= state->bearing_x;
        p.y -= state->descender;
        state->lines.back().push_back(p / (state->ascender - state->descender));
    }

    return 0;
}

struct Context {
    LineRenderer& renderer;
    FT_Face& face;
    std::vector<LineStrip>& strips;
};

void load_character(std::vector<LineStrip>& strips, FT_Face& face, LineRenderer& renderer, unsigned int codepoint) {

    strips.clear();
    unsigned int glyph_index = FT_Get_Char_Index(face, codepoint);

    FT_Error err = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);
    if (err) {
        std::cerr << "Failed to load glyph" << std::endl;
        std::exit(1);
    }

    assert(face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);

    FT_Outline_Funcs outline_funcs;
    outline_funcs.move_to = move_to;
    outline_funcs.line_to = line_to;
    outline_funcs.conic_to = conic_to;
    outline_funcs.cubic_to = cubic_to;
    outline_funcs.shift = 0;
    outline_funcs.delta = 0;

    OutlineState st;
    st.ascender = face->ascender;
    st.descender = face->descender;
    st.bearing_x = face->glyph->metrics.horiBearingX;

    FT_Outline_Decompose(&face->glyph->outline, &outline_funcs, &st);
    glClearColor(1, 1, 1, 1);

    for (const auto& line: st.lines) {
        LineStrip strip = renderer.createLineStrip(line.data(), line.size());
        strips.push_back(strip);
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    Context* ctx = static_cast<Context*>(glfwGetWindowUserPointer(window));
    load_character(ctx->strips, ctx->face, ctx->renderer, codepoint);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <font file>" << std::endl;
        std::exit(1);
    }

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

    glfwSetCharCallback(window, character_callback);
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        std::exit(1);
    }

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0);

    LineRenderer renderer;

    FT_Library ft_lib;
    FT_Error err = FT_Init_FreeType(&ft_lib);
    if (err) {
        std::cerr << "Failed to init FreeType" << std::endl;
        std::exit(1);
    }

    FT_Face face;
    err = FT_New_Face(ft_lib, argv[1], 0, &face);
    if (err) {
        std::cerr << "Failed to load the font" << std::endl;
        std::exit(1);
    }

    assert(FT_IS_SCALABLE(face));

    std::cout << "Name: " << face->family_name << " " << face->style_name << std::endl;

    std::vector<LineStrip> strips;
    Context ctx{.renderer = renderer, .face = face, .strips = strips};

    glfwSetWindowUserPointer(window, &ctx);

    load_character(strips, face, renderer, 'B');

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        for (const LineStrip& strip : strips) {
            renderer.drawLineStrip(strip);
        }
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
