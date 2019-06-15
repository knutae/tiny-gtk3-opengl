#include <stdbool.h>
#include <stdio.h>
#include <GLES3/gl32.h>
#include <gtk/gtk.h>

const char * VERTEX_SHADER =
    "#version 150\n"
    "out vec2 uv;"
    "in vec2 aVertexPosition;"
    "void main() {"
    "    uv = aVertexPosition;"
    "    gl_Position = vec4(aVertexPosition, 0, 1);"
    "}";

const char * FRAGMENT_SHADER =
    "#version 150\n"
    "out vec4 fragcolor;"
    "in vec2 uv;"
    "void main() {"
    "    fragcolor = vec4(abs(sin(uv.x)), abs(sin(uv.y)), abs(sin(uv.x-uv.y)), 1.0);"
    "}";

GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    GLint sourceLength = strlen(source);
    glShaderSource(shader, 1, &source, &sourceLength);
    glCompileShader(shader);
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length > 1) {
        char log[length];
        glGetShaderInfoLog(shader, length, NULL, log);
        printf("glCompileShader error.\n%s\n", log);
        exit(1);
    }
    return shader;
}

void realize(GtkGLArea *glarea) {
    gtk_gl_area_make_current(glarea);
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char log[length];
        glGetProgramInfoLog(program, length, NULL, log);
        printf("glLinkProgram error.\n%s\n", log);
        exit(1);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glUseProgram(program);

    GLuint screen_quad_vbo;
    GLuint screen_quad_vba;
    glGenBuffers(1, &screen_quad_vbo);
    glGenVertexArrays(1, &screen_quad_vba);
    glBindVertexArray(screen_quad_vba);

    float vertices[] = {
        -1, -1,
         1, -1,
        -1,  1,
         1,  1,
    };
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vertexPosAttrib = glGetAttribLocation(program, "aVertexPosition");
    glVertexAttribPointer(vertexPosAttrib, 2, GL_FLOAT, false, 0, NULL);
    glEnableVertexAttribArray(vertexPosAttrib);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render(GtkGLArea *glarea, GdkGLContext *context) {
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *glarea = gtk_gl_area_new();
    gtk_container_add(GTK_CONTAINER(window), glarea);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(glarea, "realize", G_CALLBACK(realize), NULL);
    g_signal_connect(glarea, "render", G_CALLBACK(render), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}