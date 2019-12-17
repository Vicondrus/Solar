//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::vec3 lightColor2;
GLuint lightColorLoc2;
glm::vec3 lightPos;
GLuint lightPosLoc;
glm::vec3 lightPos2;
GLuint lightPosLoc2;
glm::vec3 lightColor3;
GLuint lightColorLoc3;
glm::vec3 lightPos3;
GLuint lightPosLoc3;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::Camera myCamera(glm::vec3(0.0f, 1.0f, 2.5f), glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.03f;
const GLfloat near_plane = 1.0f, far_plane = 30.0f;

bool pressedKeys[1024];
float angle = 0.0f;
bool firstMouse = true;
float lastX = 400, lastY = 300;
float pitch = 0.0f, yaw = 90.0f;

gps::Model3D tree, tree2, bison, ufo, ground, lightCube;
gps::Shader myCustomShader, skyboxShader, shadowShader, lightShader;
gps::SkyBox mySkyBox;

glm::mat4 cubeModel, ufoModel;

bool animationOn = false;

int count = 0;

double animationStartTime, animationActualTime, animationTotalTime;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

int step;

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

float offset = 0;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (animationOn)
		return;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.001;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw = xoffset;
	pitch = yoffset;

	offset += xoffset;

	myCamera.rotate(pitch, yaw);
}

float str = 0;
float str2 = 1;
float lightStep = 0.1;

float delta = 0;
float movementSpeed = 10; // units per second
float updateDelta(double elapsedSeconds, float speed, float delta) {
	float d = delta + movementSpeed * elapsedSeconds;
	if (d >= 360)
		d = d - 360;
	return d;
}

bool checkIfPointInsideBox(glm::vec3 point, glm::vec3 mins, glm::vec3 maxs)
{
	glm::vec3 a = glm::vec3(mins.x, mins.y, maxs.z);
	glm::vec3 b = glm::vec3(maxs.x, mins.y, maxs.z);
	glm::vec3 c = glm::vec3(maxs.x, maxs.y, maxs.z);
	glm::vec3 d = glm::vec3(mins.x, maxs.y, maxs.z);
	glm::vec3 e = glm::vec3(mins.x, mins.y, mins.z);
	glm::vec3 f = glm::vec3(maxs.x, mins.y, mins.z);
	glm::vec3 g = glm::vec3(maxs.x, maxs.y, mins.z);
	glm::vec3 h = glm::vec3(mins.x, maxs.y, mins.z);
	/*std::vector<glm::vec3> normals;
	normals.push_back(glm::cross(a - b, c - b));
	normals.push_back(glm::cross(c - b, f - b));
	normals.push_back(glm::cross(a - b, c - b));
	normals.push_back(glm::cross(a - b, c - b));
	normals.push_back(glm::cross(a - b, c - b));
	normals.push_back(glm::cross(a - b, c - b));*/
	glm::vec3 u = a - e;
	glm::vec3 v = a - b;
	glm::vec3 w = a - d;
	bool ok1 = glm::dot(u, a) < glm::dot(u, point);
	ok1 = ok1 && glm::dot(u, point) < glm::dot(u, e);
	bool ok2 = glm::dot(v, a) < glm::dot(v, point);
	ok2 = ok2 && glm::dot(v, point) < glm::dot(v, b);
	bool ok3 = glm::dot(w, a) < glm::dot(w, point);
	ok3 = glm::dot(w, point) < glm::dot(w, d);
	bool ok4 = false;
	if (ok1 && ok2 && ok3)
		ok4 = true;
	bool ok5 = glm::dot(u, a) > glm::dot(u, point);
	ok5 = ok5 && glm::dot(u, point) > glm::dot(u, e);
	bool ok6 = glm::dot(v, a) > glm::dot(v, point);
	ok6 = ok6 && glm::dot(v, point) > glm::dot(v, b);
	bool ok7 = glm::dot(w, a) > glm::dot(w, point);
	ok7 = glm::dot(w, point) > glm::dot(w, d);
	bool ok8 = false;
	if (ok5 && ok6 && ok7)
		ok8 = true;
	if (ok4 || ok8)
		return true;
	return false;

}

bool checkCollisions(glm::vec3 mins1, glm::vec3 maxs1, glm::vec3 mins2, glm::vec3 maxs2, glm::mat4 model1, glm::mat4 model2)
{
	mins1 = glm::vec3(model1 * glm::vec4(mins1, 1.0));
	maxs1 = glm::vec3(model1 * glm::vec4(maxs1, 1.0));
	mins2 = glm::vec3(model2 * glm::vec4(mins2, 1.0));
	maxs2 = glm::vec3(model2 * glm::vec4(maxs2, 1.0));
	glm::vec3 mins = mins1;
	glm::vec3 maxs = maxs1;
	glm::vec3 a1 = glm::vec3(mins.x, mins.y, maxs.z);
	if (checkIfPointInsideBox(a1, mins2, maxs2))
		return true;
	glm::vec3 b1 = glm::vec3(maxs.x, mins.y, maxs.z);
	if (checkIfPointInsideBox(b1, mins2, maxs2))
		return true;
	glm::vec3 c1 = glm::vec3(maxs.x, maxs.y, maxs.z);
	if (checkIfPointInsideBox(c1, mins2, maxs2))
		return true;
	glm::vec3 d1 = glm::vec3(mins.x, maxs.y, maxs.z);
	if (checkIfPointInsideBox(d1, mins2, maxs2))
		return true;
	glm::vec3 e1 = glm::vec3(mins.x, mins.y, mins.z);
	if (checkIfPointInsideBox(e1, mins2, maxs2))
		return true;
	glm::vec3 f1 = glm::vec3(maxs.x, mins.y, mins.z);
	if (checkIfPointInsideBox(f1, mins2, maxs2))
		return true;
	glm::vec3 g1 = glm::vec3(maxs.x, maxs.y, mins.z);
	if (checkIfPointInsideBox(g1, mins2, maxs2))
		return true;
	glm::vec3 h1 = glm::vec3(mins.x, maxs.y, mins.z);
	if (checkIfPointInsideBox(h1, mins2, maxs2))
		return true;
	mins = mins2;
	maxs = maxs2;
	glm::vec3 a2 = glm::vec3(mins.x, mins.y, maxs.z);
	if (checkIfPointInsideBox(a2, mins1, maxs1))
		return true;
	glm::vec3 b2 = glm::vec3(maxs.x, mins.y, maxs.z);
	if (checkIfPointInsideBox(b2, mins1, maxs1))
		return true;
	glm::vec3 c2 = glm::vec3(maxs.x, maxs.y, maxs.z);
	if (checkIfPointInsideBox(c2, mins1, maxs1))
		return true;
	glm::vec3 d2 = glm::vec3(mins.x, maxs.y, maxs.z);
	if (checkIfPointInsideBox(d2, mins1, maxs1))
		return true;
	glm::vec3 e2 = glm::vec3(mins.x, mins.y, mins.z);
	if (checkIfPointInsideBox(e2, mins1, maxs1))
		return true;
	glm::vec3 f2 = glm::vec3(maxs.x, mins.y, mins.z);
	if (checkIfPointInsideBox(f2, mins1, maxs1))
		return true;
	glm::vec3 g2 = glm::vec3(maxs.x, maxs.y, mins.z);
	if (checkIfPointInsideBox(g2, mins1, maxs1))
		return true;
	glm::vec3 h2 = glm::vec3(mins.x, maxs.y, mins.z);
	if (checkIfPointInsideBox(h2, mins1, maxs1))
		return true;
	return false;
}

float deltaC = 0;

void updateDeltaCamera(double elapsedSeconds) {
	deltaC = deltaC + cameraSpeed * elapsedSeconds;
}

float deltaCube = 0;

double lastTimeStamp = glfwGetTime();

void processMovement()
{
	if (pressedKeys[GLFW_KEY_P]) {
		std::cout << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << "\n";
		std::cout << checkIfPointInsideBox(glm::vec3(0, 0, 0), glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1)) << "\n";
	}

	/*if (pressedKeys[GLFW_KEY_M]) {
		animationOn = true;
		animationTotalTime = 10;
		animationStartTime = glfwGetTime();
		step = 0;
	}*/

	if (!animationOn) {

		if (pressedKeys[GLFW_KEY_Q]) {
			if (str < 1)
				str += lightStep;
		}

		if (pressedKeys[GLFW_KEY_E]) {
			if (str > 0)
				str -= lightStep;
		}

		if (pressedKeys[GLFW_KEY_F]) {
			if (str2 < 1)
				str2 += lightStep;
		}

		if (pressedKeys[GLFW_KEY_G]) {
			if (str2 > 0)
				str2 -= lightStep;
		}

		if (pressedKeys[GLFW_KEY_W]) {
			myCamera.move(gps::MOVE_FORWARD, deltaC);
		}

		if (pressedKeys[GLFW_KEY_S]) {
			myCamera.move(gps::MOVE_BACKWARD, deltaC);
		}

		if (pressedKeys[GLFW_KEY_A]) {
			myCamera.move(gps::MOVE_LEFT, deltaC);
		}

		if (pressedKeys[GLFW_KEY_D]) {
			myCamera.move(gps::MOVE_RIGHT, deltaC);
		}

		if (pressedKeys[GLFW_KEY_SPACE]) {
			myCamera.move(gps::MOVE_UP, deltaC);
		}

		if (pressedKeys[GLFW_KEY_LEFT_CONTROL]) {
			myCamera.move(gps::MOVE_DOWN, deltaC);
		}
	}
}

void processLightColor() {
	//set light color
	lightColor = glm::vec3(0.3f + str, 0.5f + str, 0.3f + str); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightColor2 = glm::vec3(str2, str2, str2); //white light
	lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
	glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//glEnable(GL_FRAMEBUFFER_SRGB);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

std::vector<float> treePosx;
std::vector<float> treePosz;
std::vector<float> treeType;
std::vector<float> treeSize;
std::vector<float> treeRot;

const int trees = 8;

void genTrees() {

	for (int i = 0; i < trees; i++) {
		for (int j = 0; j < trees; j++) {
			int x = rand() % 100 + 50;
			float size = x / 100.0;
			int type = rand() % 2;
			int posx = rand() % 5;
			int posz = rand() % 5;
			int rot = rand() % 628;
			float rotation = rot / 100;

			posx = -30 + i * (60 / trees) + posx;
			posz = -30 + j * (60 / trees) + posz;

			treePosx.push_back(posx);
			treePosz.push_back(posz);
			treeType.push_back(type);
			treeSize.push_back(size);
			treeRot.push_back(rotation);
		}
	}
}

std::vector<const GLchar*> faces;

glm::vec3 ufoMins, ufoMaxs;
glm::vec3 groundMins, groundMaxs;

void initModels()
{
	faces.push_back("textures/skybox/skyrender0001.tga");
	faces.push_back("textures/skybox/skyrender0004.tga");
	faces.push_back("textures/skybox/skyrender0003.tga");
	faces.push_back("textures/skybox/skyrender0006.tga");
	faces.push_back("textures/skybox/skyrender0005.tga");
	faces.push_back("textures/skybox/skyrender0002.tga");
	tree = gps::Model3D("objects/tree1/Tree.obj", "objects/tree1/");
	tree2 = gps::Model3D("objects/tree2/Tree.obj", "objects/tree2/");
	bison = gps::Model3D("objects/bison/Bison.obj", "objects/bison/");
	ufo = gps::Model3D("objects/ufo/UFO2.obj", "objects/ufo/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");
	genTrees();
	ufoMaxs = ufo.getMaxs();
	ufoMins = ufo.getMins();
	groundMaxs = ground.getMaxs();
	groundMins = ground.getMins();
}

void initShaders()
{
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	shadowShader.loadShader("shaders/shaderShadow.vert", "shaders/shaderShadow.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	myCustomShader.useShaderProgram();
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	////set light color
	//lightColor = glm::vec3(0.3f, 0.5f, 0.3f); //white light
	//lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	//glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set light color
	//lightColor2 = glm::vec3(0.0f, 0.0f, 0.0f); //white light
	//lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
	//glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));

	//lightPos = glm::vec3(1.0f, 1.0f, 0.0f);
	//lightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos");
	//glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	lightPos2 = glm::vec3(0.0f, 1.0f, 5.0f);
	lightPosLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos2");
	glUniform3fv(lightPosLoc2, 1, glm::value_ptr(lightPos2));

	lightPos3 = glm::vec3(10.0f, 10.0f, 10.0f);
	lightPosLoc3 = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos3");
	glUniform3fv(lightPosLoc3, 1, glm::value_ptr(lightPos3));

	lightColor3 = glm::vec3(0.7f, 0.3f, 0.3f); //white light
	lightColorLoc3 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor3");
	glUniform3fv(lightColorLoc3, 1, glm::value_ptr(lightColor3));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	skyboxShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

}

void renderTrees(gps::Shader shader)
{

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	for (int i = 0; i < treePosx.size(); i++) {
		model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(treePosx[i], 0, treePosz[i]));
		model = glm::rotate(model, glm::radians(treeRot[i]), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(treeSize[i]));

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model") , 1, GL_FALSE, glm::value_ptr(model));

		if (treeType[i] == 0)
			tree.Draw(shader);
		else
			tree2.Draw(shader);
	}
}

void renderAnimals(gps::Shader shader)
{

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix") , 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//1
	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::translate(model, glm::vec3(0, 0.3, 1));

	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bison.Draw(shader);

	//2
	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::translate(model, glm::vec3(1, 0.3, 1));

	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	bison.Draw(shader);
}

void renderLightCubes(gps::Shader shader) {

	shader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::mat4(1.0f);
	//model = glm::translate(model, glm::vec3(-1, -1, -1));
	//model = glm::translate(model, glm::vec3(1, 1, 1));
	model = glm::translate(model, glm::vec3(10,10,10));
	//model = glm::rotate(model, deltaCube, glm::vec3(1, 1, 0));
	model = glm::scale(model, glm::vec3(0.07f, 0.07f, 0.07f));

	cubeModel = model;

	//cubeModel = glm::scale(cubeModel, glm::vec3(0.6, 0.6, 0.6));

	if (checkCollisions(ufo.getMins(), ufo.getMaxs(), lightCube.getMins(), lightCube.getMaxs(), ufoModel, cubeModel)) {
		std::cout << "Collision between ufo and cube" << count << "\n";
		count++;
		animationOn = true;
		animationTotalTime = 10;
		animationStartTime = glfwGetTime();
		step = 0;
	}

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCube.Draw(shader);
}

void renderUfo(gps::Shader shader) {
	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//1
	//initialize the model matrix
	model = glm::mat4(1.0f);

	//model = glm::translate(model, glm::vec3(0, 4, 0));

	glm::mat4 viewMatrixInverse = glm::inverse(view);
	glm::vec3 cameraPositionWorldSpace = glm::vec3(viewMatrixInverse[3][0], viewMatrixInverse[3][1], viewMatrixInverse[3][2]);

	model = glm::translate(model, cameraPositionWorldSpace);

	//offset = offset + yaw;
	//glm::mat4 aux = glm::translate(model, glm::vec3(0, 0, -5));
	//glm::vec4 q = glm::vec4(1);
	//q = model * q;
	//float angle = glm::degrees(glm::acos(glm::dot(glm::vec3(q), myCamera.getCameraDirection())));

	//model = glm::rotate(model, offset, glm::vec3(0, 1, 0));

	model = glm::translate(model, 5.0f*myCamera.getCameraDirection());

	model = glm::rotate(model, glm::radians(delta), glm::vec3(0, 1, 0));

	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

	ufoModel = model;

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
	lightPos = glm::vec3(model * glm::vec4(lightPos, 1.0f));
	lightPosLoc = glGetUniformLocation(shader.shaderProgram, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	ufo.Draw(shader);
}

void renderGround(gps::Shader shader) {
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			//1
			model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3((-20 + 20*i),0,(-20 + 20*j)));

			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			ground.Draw(shader);
		}
	}
}

void renderScene(gps::Shader shader)
{
	shader.useShaderProgram();

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processLightColor();

	renderGround(shader);

	renderTrees(shader);

	renderAnimals(shader);

	renderUfo(shader);

	double currentTimeStamp = glfwGetTime();
	delta = updateDelta(currentTimeStamp - lastTimeStamp, movementSpeed, delta);
	deltaCube = updateDelta(currentTimeStamp - lastTimeStamp, 20, deltaCube);
	updateDeltaCamera(currentTimeStamp - lastTimeStamp);
	lastTimeStamp = currentTimeStamp;
}

glm::mat4 computeLightSpaceTrMatrix()
{
	glm::mat4 lightView = glm::lookAt(2.0f * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void renderSceneDepthMap() {
	shadowShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderScene(shadowShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderSceneMain() {

	myCustomShader.useShaderProgram();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, retina_width, retina_height);
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

	//initialize the view matrix
	view = myCamera.getViewMatrix();
	//send view matrix data to shader	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glActiveTexture(GL_TEXTURE5);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 5);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

	renderScene(myCustomShader);
}

void renderWhole() {

	glfwSetCursorPosCallback(glWindow, mouseCallback);

	processMovement();

	renderSceneDepthMap();
	renderSceneMain();

	renderLightCubes(lightShader);

	mySkyBox.Draw(skyboxShader, view, projection);

	if (animationOn) {
		if(step==0)
			myCamera.interpolate(myCamera.getCameraPosition(), glm::vec3(20, 5, -20), myCamera.getCameraDirection() * 5.0f, glm::vec3(0, 5, 0), animationActualTime - animationStartTime, animationTotalTime);
		if (step == 1)
			myCamera.interpolate(glm::vec3(20, 5, -20), glm::vec3(40, 10, 20), glm::vec3(0, 5, 0), glm::vec3(0, 10, 0), animationActualTime - animationStartTime, animationTotalTime);
		if(step == 2)
			myCamera.interpolate(glm::vec3(40, 10, 20), glm::vec3(30, 7, -10), glm::vec3(0, 10, 0), glm::vec3(0, 7, 0), animationActualTime - animationStartTime, animationTotalTime);
		if (animationActualTime - animationStartTime >= animationTotalTime) {
			step++;
			animationStartTime = glfwGetTime();
			if(step >= 3)
				animationOn = false;
		}
		animationActualTime = glfwGetTime();
	}
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();
	//genTrees();
	glCheckError();

	//TODO
	//fog for skybox //DONE
	//collision with ground and floating cubes	//MAYBE DONE
	//when reach a floating cube, do presentation animation

	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	mySkyBox.Load(faces);


	while (!glfwWindowShouldClose(glWindow)) {
		renderWhole();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
