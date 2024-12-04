#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>

// ID шейдерной программы
GLuint Program;
// ID атрибутов
GLint Attrib_vertex, Attrib_color;
// ID Vertex Buffer Object
GLuint VBO, CBO; //буфер вершин, буфер цвета

// Исходный код вершинного шейдера для градиентного закрашивания
const char* VertexShaderSourceGradient = R"(
#version 330 core
in vec2 coord;
in vec3 vertexColor;
out vec3 fragColor;
void main() {
    gl_Position = vec4(coord, 0.0, 1.0);
    fragColor = vertexColor;
}
)";

// Исходный код фрагментного шейдера для градиентного закрашивания
const char* FragShaderSourceGradient = R"(
#version 330 core
in vec3 fragColor;
out vec4 color;
void main() {
    color = vec4(fragColor, 1.0);
}
)";

// Исходный код старого фрагментного шейдера (синий)
const char* FragShaderSourceOld = R"(
#version 330 core
out vec4 color;
void main() {
    color = vec4(0, 0, 1, 1); // Синий цвет
}
)";

// Исходный код шейдера для красного цвета
const char* VertexShaderSourceRed = R"(
#version 330 core
in vec2 coord;
out vec3 fragColor;
void main() {
    gl_Position = vec4(coord, 0.0, 1.0);
    fragColor = vec3(1.0, 0.0, 0.0); // Красный цвет
}
)";

// Фрагментный шейдер для красного
const char* FragShaderSourceRed = R"(
#version 330 core
in vec3 fragColor;
out vec4 color;
void main() {
    color = vec4(fragColor, 1.0); // Красный цвет
}
)";

// Структура для передачи координат вершин
struct Vertex {
    GLfloat x;
    GLfloat y;
};

// Структура для передачи цвета вершин
struct Color {
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

enum ShaderMode {
    BLUE, // Синий
    GRADIENT, // Градиент
    RED // Красный
};

ShaderMode shaderMode = BLUE; // Стартовый режим - синий

void checkOpenGLerror() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}

void ShaderLog(unsigned int shader) {
    int infologLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1) {
        int charsWritten = 0;
        std::vector<char> infoLog(infologLen);
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
        std::cout << "InfoLog: " << infoLog.data() << std::endl;
    }
}

void ProgramLog(unsigned int program) {
    int infologLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1) {
        int charsWritten = 0;
        std::vector<char> infoLog(infologLen);
        glGetProgramInfoLog(program, infologLen, &charsWritten, infoLog.data());
        std::cout << "Program InfoLog: " << infoLog.data() << std::endl;
    }
}

void InitVBO() {
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &CBO);

    // Вершины нашего треугольника
    Vertex vertices[20] = {
        { -1.0f, 0.2f }, { -0.6f, 1.0f }, { -0.2f, 0.2f }, //TRIANGLE
        { 0.2f, 0.2f }, { 1.0f, 0.2f },  { 0.2f,  0.8f }, { 1.0f,  0.8f }, //QUAD
        {0.6f, -1.0f}, {0.2f, -0.8f}, {0.3f,-0.5f}, {0.6f,-0.3f}, {0.89f, -0.5f}, {0.99f, -0.8f}, //FAN
        {-1.0f, -0.5f}, {-0.57f,-0.2f}, {-0.14f,-0.5f}, {-0.3f, -1.0f}, {-0.83f, -1.0f}, {-1.0f, -0.5f} //PENTAGON
    };

    // Цвета вершин (для градиента)
    Color colors[24] = {
        {1.0f, 0.0f, 0.0f}, 
        {0.0f, 1.0f, 0.0f}, 
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f}, 
        {0.0f, 1.0f, 1.0f},  
        { 1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, 
        {0.0f, 0.0f, 1.0f}, 
        {1.0f, 1.0f, 0.0f}, 
        {1.0f, 0.0f, 1.0f}, 
        {0.0f, 1.0f, 1.0f},  
        { 1.0f, 0.0f, 0.0f}, 
        {0.0f, 1.0f, 0.0f}, 
        {0.0f, 0.0f, 1.0f}, 
        {1.0f, 1.0f, 0.0f}, 
        {1.0f, 0.0f, 1.0f}, 
        {0.0f, 1.0f, 1.0f},  
        { 1.0f, 0.0f, 0.0f}, 
        {0.0f, 1.0f, 0.0f}, 
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f}, 
        {1.0f, 0.0f, 1.0f}, 
        {0.0f, 1.0f, 1.0f}  
    };

    // Передаём вершины в буфер
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Передаём цвета в буфер
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    checkOpenGLerror();
}

void InitShader() {
    // Создаём вершинный шейдер и фрагментный шейдер в зависимости от режима
    GLuint vShader, fShader;

    switch (shaderMode) {
    case BLUE:
        vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &VertexShaderSourceRed, NULL);
        glCompileShader(vShader);
        ShaderLog(vShader);

        fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &FragShaderSourceOld, NULL);
        glCompileShader(fShader);
        ShaderLog(fShader);
        break;

    case GRADIENT:
        vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &VertexShaderSourceGradient, NULL);
        glCompileShader(vShader);
        ShaderLog(vShader);

        fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &FragShaderSourceGradient, NULL);
        glCompileShader(fShader);
        ShaderLog(fShader);
        break;

    case RED:
        vShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vShader, 1, &VertexShaderSourceRed, NULL);
        glCompileShader(vShader);
        ShaderLog(vShader);

        fShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fShader, 1, &FragShaderSourceRed, NULL);
        glCompileShader(fShader);
        ShaderLog(fShader);
        break;
    }

    // Проверка ошибок компиляции и линковки
    GLint success;
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cout << "Vertex shader compilation failed!" << std::endl;
    }
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::cout << "Fragment shader compilation failed!" << std::endl;
    }

    // Создаём программу и прикрепляем шейдеры к ней
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);
    glLinkProgram(Program);

    // Проверка на успешную линковку программы
    glGetProgramiv(Program, GL_LINK_STATUS, &success);
    if (!success) {
        std::cout << "Program linking failed!" << std::endl;
        ProgramLog(Program);
    }

    // Вытягиваем ID атрибутов
    Attrib_vertex = glGetAttribLocation(Program, "coord");
    Attrib_color = glGetAttribLocation(Program, "vertexColor");

    glUseProgram(Program);
    checkOpenGLerror();
}

void Draw() {
    glUseProgram(Program);

    // Привязка атрибутов
    glEnableVertexAttribArray(Attrib_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(Attrib_vertex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(Attrib_color);
    glBindBuffer(GL_ARRAY_BUFFER, CBO);
    glVertexAttribPointer(Attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof(Color), (void*)0);

    // Рисуем треугольник
    glDrawArrays(GL_TRIANGLES, 0, 3); ///////////////////////////////////////////////
    //рисуем квадрат
    glDrawArrays(GL_TRIANGLE_STRIP, 3, 4);
    //рисуем веер
    glDrawArrays(GL_TRIANGLE_FAN, 7, 6);
    //РИСУЕМ ПЯТИУГОЛЬНИК
    //glDrawArrays(GL_TRIANGLE_FAN, 13, 7);
    glDrawArrays(GL_POLYGON, 13, 6);

    glDisableVertexAttribArray(Attrib_vertex);
    glDisableVertexAttribArray(Attrib_color);

    checkOpenGLerror();
}

void ReleaseShader() {
    glUseProgram(0);
    checkOpenGLerror();
}

void Release() {
    ReleaseShader();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &CBO);
}

int main() {
    sf::Window window(sf::VideoMode(600, 600), "OpenGL Shader Switcher", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    glewInit();
    InitVBO();
    InitShader();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                // Переключение между шейдерами при нажатии Space
                shaderMode = static_cast<ShaderMode>((shaderMode + 1) % 3);
                ReleaseShader();
                InitShader();
            }
            else if (event.type == sf::Event::Resized) {
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Draw();
        window.display();
    }

    Release();
    return 0;
}
