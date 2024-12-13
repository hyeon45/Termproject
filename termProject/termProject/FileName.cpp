/*#define _CRT_SECURE_NO_WARNINGS 
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

void make_vertexShaders();
void make_fragmentShaders();
void InitBuffer();
GLuint make_shaderProgram();
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Timer(int value);
GLvoid Keyboard(unsigned char key, int x, int y);

GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

void ChangeWtoOpenGL(int windowx, int windowy, int windowwidth, int windowheight);
double xopenGL = 0;
double yopenGL = 0;
void UpdateUniforms(const glm::mat4& model, const glm::vec3& color);
void LoadAndBindTexture();
GLuint LoadTexture(const char* path);

GLuint vao, vbo[2];

float Botton[] = {
	// ��ġ (x, y, z)   | ���� (r, g, b)  | �ؽ�ó ��ǥ (u, v) | ���� ���� (nx, ny, nz)
	1000.0f, 0.0f,  1000.0f,     1.0f, 0.0f, 0.0f,      1.0f, 1.0f,    0.0f, 1.0f, 0.0f,	// P1

	1000.0f, 0.0f, -1000.0f,     0.0f, 1.0f, 0.0f,      1.0f, 0.0f,    0.0f, 1.0f, 0.0f,	// P2

   -1000.0f, 0.0f,  1000.0f,     0.0f, 0.0f, 1.0f,      0.0f, 1.0f,    0.0f, 1.0f, 0.0f,	// P3


    1000.0f, 0.0f, -1000.0f,     0.0f, 1.0f, 0.0f,      1.0f, 0.0f,    0.0f, 1.0f, 0.0f,   // P2

   -1000.0f, 0.0f, -1000.0f,     1.0f, 1.0f, 0.0f,      0.0f, 0.0f,    0.0f, 1.0f, 0.0f,   // P4

   -1000.0f, 0.0f,  1000.0f,     0.0f, 0.0f, 1.0f,      0.0f, 1.0f,    0.0f, 1.0f, 0.0f,   // P3
};

GLuint textureID;

void LoadAndBindTexture() {
	textureID = LoadTexture("map.png");
}


void main(int argc, char** argv) {

	width = 800;
	height = 800;

	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // ���÷��� ��� ����
	glutInitWindowPosition(100, 100); // �������� ��ġ ����
	glutInitWindowSize(width, height); // �������� ũ�� ����
	glutCreateWindow("Example1"); // ������ ����


	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl
			;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	srand(time(NULL));


	//--- ���̴� �о�ͼ� ���̴� ���α׷� �����
	make_vertexShaders(); //--- ���ؽ� ���̴� �����
	make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
	shaderProgramID = make_shaderProgram();
	//--- ���̴� ���α׷� �����
	make_shaderProgram();
	InitBuffer();


	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutMainLoop();
}

GLvoid drawScene() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); // CW:�ð����, CCW:�ݽð����
	glClearColor(1.0, 1.0, 1.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	// �ؽ�ó Ȱ��ȭ
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

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

	glm::mat4 Botton = glm::mat4(1.0f);
	UpdateUniforms(Botton, glm::vec3(0.0f, 1.0f, 0.2f)); // ������Ʈ

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glutSwapBuffers();
}


GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}



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

void InitBuffer() {
	glGenVertexArrays(1, &vao); // VAO ���� �� ���ε�
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Botton), Botton, GL_STATIC_DRAW);

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

	// �ؽ�ó �ε�
	LoadAndBindTexture();

	glUseProgram(shaderProgramID);

	// �ؽ�ó Ȱ��ȭ �� ������ ����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	GLint texUniform = glGetUniformLocation(shaderProgramID, "ourTexture");
	glUniform1i(texUniform, 0);  // �ؽ�ó ���� 0
}




GLvoid Keyboard(unsigned char key, int x, int y) {

	glutPostRedisplay();	//���� ������ ��ȭ�ϰ� ����ϴ� �ݹ��Լ�
}



GLvoid Timer(int value) {	//Ÿ�̸� �Լ����� �������� ������ �������

	glutTimerFunc(16, Timer, 0);
	glutPostRedisplay();


}


void ChangeWtoOpenGL(int windowx, int windowy, int windowwidth, int windowheight) { //��ǥ ��ȯ(�ϴ� �־��)
	xopenGL = (2.0f * windowx / windowwidth) - 1.0f;
	yopenGL = 1.0f - (2.0f * windowy / windowheight);
	printf("%0.4f, %0.4f\n", xopenGL, yopenGL);

}

void UpdateUniforms(const glm::mat4& model, const glm::vec3& color) { //model���� objectcolor�� �����Լ�
	// ��ȯ ��� ������Ʈ
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "modelTransform");
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// ���� ������Ʈ
	unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3fv(colorLocation, 1, glm::value_ptr(color));
}

void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
	//--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
	vertexSource = filetobuf("vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
	fragmentSource = filetobuf("fragment.glsl"); // �����׼��̴� �о����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- ���̴� ���α׷� �����
	glAttachShader(shaderID, vertexShader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
	glAttachShader(shaderID, fragmentShader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�
	glLinkProgram(shaderID); //--- ���̴� ���α׷� ��ũ�ϱ�
	glDeleteShader(vertexShader); //--- ���̴� ��ü�� ���̴� ���α׷��� ��ũ��������, ���̴� ��ü ��ü�� ���� ����
	glDeleteShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- ������� ���̴� ���α׷� ����ϱ�
	//--- ���� ���� ���̴����α׷� ���� �� �ְ�, �� �� �Ѱ��� ���α׷��� ����Ϸ���
	//--- glUseProgram �Լ��� ȣ���Ͽ� ��� �� Ư�� ���α׷��� �����Ѵ�.
	//--- ����ϱ� ������ ȣ���� �� �ִ�.
	return shaderID;
}

/**/