#include <stdbool.h>
#include <stdio.h>
#include <GLES3/gl32.h>
#include <gtk/gtk.h>

const char * VERTEX_SHADER =
    "#version 450\n"
    "out vec2 uv;"
    "layout (location=0) in vec2 aVertexPosition;"
    "void main() {"
    "    uv = aVertexPosition;"
    "    gl_Position = vec4(aVertexPosition, 0, 1);"
    "}";

const char * FRAGMENT_SHADER =
    "#version 450\n"
    "out vec4 fragcolor;"
    "in vec2 uv;"
    "layout (location=0) uniform float uTime;"
    "void main() {"
    "    fragcolor = vec4("
    "        abs(sin(uv.x + uTime * 0.123)),"
    "        abs(sin(uv.y - uTime * 0.321)),"
    "        abs(sin(uv.x-uv.y + uTime * 0.012)),"
    "        1.0);"
    "}";

static GdkFrameClock * frame_clock;
static gint64 start_time;

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

    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, NULL);
    glEnableVertexAttribArray(0);

    GdkWindow * gdk_window = gtk_widget_get_window(GTK_WIDGET(glarea));
    frame_clock = gdk_window_get_frame_clock(gdk_window);
    start_time = gdk_frame_clock_get_frame_time(frame_clock);
    g_signal_connect_swapped(frame_clock, "update", G_CALLBACK(gtk_gl_area_queue_render), glarea);
    gdk_frame_clock_begin_updating(frame_clock);
}

void render(GtkGLArea *glarea, GdkGLContext *context) {
    double time = (gdk_frame_clock_get_frame_time(frame_clock) - start_time) / 1000000.0;
    glUniform1f(0, time);
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