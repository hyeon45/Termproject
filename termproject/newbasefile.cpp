/**/#define _CRT_SECURE_NO_WARNINGS 
#define GLM_FORCE_RADIANS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include "filetobuf.h"

// stb_image ����
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ���̴� �ҽ� �ڵ�
GLchar* vertexSource, * fragmentSource; // ���̴� �ҽ� �ڵ�
GLuint vertexShader, fragmentShader;    // ���̴� ��ü
GLuint shaderProgramID;                 // ���̴� ���α׷�
GLuint vao_axes, vao_cube, vao_pyramid; // VAO��
GLuint vbo_axes, vbo_cube, vbo_pyramid; // VBO��
GLuint ebo_cube, ebo_pyramid;           // EBO��

int window_width = 1000, window_height = 800;

// ���� ���õ� ��ü (1: ����ü, 2: �簢��)
int currentObject = 1;

// ȸ�� ���� (�ʱⰪ)
float rotationX = 30.0f;
float rotationY = -30.0f;

// ȸ�� �ִϸ��̼� �ӵ�
float rotationSpeedX = 0.0f;
float rotationSpeedY = 0.0f;

// ���̾������� ��� ����
bool isWireframe = false;

// ���� ���� ����
bool cullFace = false;

// �ؽ�ó ID
GLuint cubeTextures[6];

GLfloat floorVertices[] = {
    // Position                  // Color          // TexCoords         // Normal
    -1000.0f, 0.0f,  1000.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,  0.0f,  1.0f,  0.0f, // ���� 0
     1000.0f, 0.0f,  1000.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,  0.0f,  1.0f,  0.0f, // ���� 1
     1000.0f, 0.0f, -1000.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,  0.0f,  1.0f,  0.0f, // ���� 2
    -1000.0f, 0.0f, -1000.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  0.0f,  1.0f,  0.0f  // ���� 3
};
GLuint floorIndices[] = {
    0, 1, 2,
    2, 3, 0
};

// ���ؽ� ���̴� ������ �Լ�
void make_vertexShaders() {
    vertexSource = filetobuf("vertex.glsl");
    if (!vertexSource) exit(EXIT_FAILURE);
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
    glCompileShader(vertexShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        fprintf(stderr, "ERROR: vertex shader ������ ����\n%s\n", errorLog);
        exit(EXIT_FAILURE);
    }
    free(vertexSource);
}

// �����׸�Ʈ ���̴� ������ �Լ�
void make_fragmentShaders() {
    fragmentSource = filetobuf("fragment.glsl");
    if (!fragmentSource) exit(EXIT_FAILURE);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
    glCompileShader(fragmentShader);
    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        fprintf(stderr, "ERROR: fragment shader ������ ����\n%s\n", errorLog);
        exit(EXIT_FAILURE);
    }
    free(fragmentSource);
}

// ���̴� ���α׷� ���� �Լ�
void make_shaderProgram() {
    make_vertexShaders();
    make_fragmentShaders();

    shaderProgramID = glCreateProgram();
    glAttachShader(shaderProgramID, vertexShader);
    glAttachShader(shaderProgramID, fragmentShader);
    glLinkProgram(shaderProgramID);

    GLint result;
    GLchar errorLog[512];
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
        fprintf(stderr, "ERROR: ���̴� ���α׷� ��ũ ����\n%s\n", errorLog);
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgramID);
}

// �ؽ�ó �ε� �Լ�
GLuint LoadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // �̹��� �ε�
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        // �ؽ�ó ���ε� �� ����
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // �ؽ�ó �Ķ���� ����
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// ���� �ʱ�ȭ �Լ�
void InitBuffer() {

    // Cube VAO
    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);

    // VBO for cube
    glGenBuffers(1, &vbo_cube);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    // EBO for cube
    glGenBuffers(1, &ebo_cube);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_cube);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// ��ü�� �� ��ȯ ����� �����ϴ� �Լ�
void SetModelTransform(glm::mat4 model) {
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), // ī�޶� ��ġ
        glm::vec3(0.0f, 0.0f, 0.0f), // ī�޶� Ÿ��
        glm::vec3(0.0f, 1.0f, 0.0f)); // �� ����
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)window_width / window_height,
        0.1f, 100.0f);
    glm::mat4 modelTransform = projection * view * model;

    GLuint modelTransformLoc = glGetUniformLocation(shaderProgramID, "modelTransform");
    glUniformMatrix4fv(modelTransformLoc, 1, GL_FALSE, glm::value_ptr(modelTransform));
}

// ����ü �׸��� �Լ�
void drawCube() {
    glBindVertexArray(vao_cube);
    glUseProgram(shaderProgramID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cubeTextures[0]);

    // �ε��� ������ ��� (�� �鸶�� 6���� �ε���)
    GLsizei offset = 0 * 6 * sizeof(GLuint);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)offset);

    glBindVertexArray(0);
}

// ��� �׸��� �Լ�
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramID);


    // ��� ���� ��� ����
    glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, -30.0f); // ī�޶� ��ġ
    glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); // ī�޶� ����
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // ī�޶� ����

    glm::mat4 view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
    unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

    // ��ü ȸ�� ������Ʈ
    rotationX += rotationSpeedX;
    rotationY += rotationSpeedY;

    // �ؽ�ó ��� �� ��
    GLint useTextureLoc = glGetUniformLocation(shaderProgramID, "useTexture");
    glUniform1i(useTextureLoc, 0);

    glBindVertexArray(0);

    // �� ��� ����
    glm::mat4 model_cube = glm::mat4(1.0f);
    model_cube = glm::rotate(model_cube, glm::radians(rotationX), glm::vec3(1, 0, 0));
    model_cube = glm::rotate(model_cube, glm::radians(rotationY), glm::vec3(0, 1, 0));
    SetModelTransform(model_cube);

    // �ؽ�ó ���
    glUniform1i(useTextureLoc, 1);

    // ����ü �׸���
    drawCube();

    glutSwapBuffers();
}

// Ű���� �Է� �ݹ� �Լ�
void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    default:
        break;
    }
    glutPostRedisplay();
}

// Ư�� Ű �Է� �ݹ� �Լ� (ȭ��ǥ Ű)
void SpecialKeys(int key, int x, int y) {
    switch (key) {
    default:
        break;
    }
    glutPostRedisplay();
}

// Ÿ�̸� �Լ� (�ִϸ��̼� ������Ʈ)
void Timer(int value) {
    // ��ü ȸ�� ������Ʈ�� �̹� drawScene���� ó��
    glutPostRedisplay();
    glutTimerFunc(16, Timer, 0); // �� 60fps
}

// �������� �ݹ� �Լ�
void Reshape(int w, int h) {
    window_width = w;
    window_height = h;
    glViewport(0, 0, w, h);
}

// �ʱ�ȭ �Լ�
void Initialize() {
    // ���̴� ���α׷� ����
    make_shaderProgram();

    // ���� �ʱ�ȭ
    InitBuffer();

    // �ؽ�ó �ε�
    // ����ü �ؽ�ó �ε� (6��)
    cubeTextures[0] = LoadTexture("map.png");   // Front face
    cubeTextures[1] = LoadTexture("map.png"); // Back face
    cubeTextures[2] = LoadTexture("map.png"); // Left face
    cubeTextures[3] = LoadTexture("map.png"); // Right face
    cubeTextures[4] = LoadTexture("map.png"); // Top face
    cubeTextures[5] = LoadTexture("map.png"); // Bottom face

    // �ؽ�ó ������ ���� (���� �ؽ�ó ���÷� ���)
    glUseProgram(shaderProgramID);
    glUniform1i(glGetUniformLocation(shaderProgramID, "texture1"), 0); // �ؽ�ó ���� 0�� ���ε�

    // ���� ������ ����
    GLint ambientOnLoc = glGetUniformLocation(shaderProgramID, "ambientOn");
    GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
    GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");
    GLint ambientColorLoc = glGetUniformLocation(shaderProgramID, "ambientColor");

    if (ambientOnLoc == -1 || lightColorLoc == -1 || lightPosLoc == -1 || viewPosLoc == -1 || ambientColorLoc == -1) {
        std::cerr << "ERROR: ���̴����� �ʿ��� ������ ������ ã�� �� �����ϴ�." << std::endl;
    }

    glUniform1i(ambientOnLoc, 1); // �ֺ��� Ȱ��ȭ
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // ��� ����
    glUniform3f(lightPosLoc, 5.0f, 5.0f, 5.0f); // ���� ��ġ
    glUniform3f(viewPosLoc, 0.0f, 0.0f, 7.0f); // ī�޶� ��ġ
    glUniform3f(ambientColorLoc, 1.0f, 1.0f, 1.0f); // �ֺ��� ����

    // ���� ����
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f); // ȸ�� ���

    // ���� �׽�Ʈ Ȱ��ȭ
    glEnable(GL_DEPTH_TEST);
}

// ���� �Լ�
int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    // GLUT �ʱ�ȭ
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Example 29");

    // GLEW �ʱ�ȭ
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "GLEW �ʱ�ȭ ����\n");
        return EXIT_FAILURE;
    }

    // �ʱ�ȭ �Լ� ȣ��
    Initialize();

    // �ݹ� �Լ� ���
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeys);
    glutTimerFunc(0, Timer, 0);

    // ���� ���� ����
    glutMainLoop();

    return 0;
}
/**/