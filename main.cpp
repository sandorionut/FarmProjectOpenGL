//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include <vector>
#include <chrono>
#include <iostream>

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;


// camera
gps::Camera myCamera(
	glm::vec3(6.0f, 1.8f, 1.0f), 
	glm::vec3(0.0f, 1.8f, -10.0f), 
	glm::vec3(0.0f, 1.0f, 0.0f)
);
GLfloat cameraSpeed = 0.035f;
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D ground;
gps::Model3D barn;
gps::Model3D house;
gps::Model3D chickenCoop;
gps::Model3D silo;
gps::Model3D greenhouse;
gps::Model3D stable;
gps::Model3D dogsHouse;
gps::Model3D CatMac;
gps::Model3D farmer;
gps::Model3D fence;
gps::Model3D tree;
gps::Model3D firepit;
gps::Model3D shovel;
gps::Model3D haybale;
gps::Model3D dog;
gps::Model3D lemon_tree;
gps::Model3D cart;
gps::Model3D chicken;
gps::Model3D egg;
gps::Model3D pig;
gps::Model3D farmer3;
gps::Model3D haystack;
gps::Model3D pigsty;
glm::vec3 farmerPosition(6.0f, 0.0f, 4.0f);
glm::vec3 farmerEndPosition(6.0f, 0.0f, -5.0f);
float farmerMovementSpeed = 0.02f;
float farmerRotation = 180.0f;
bool farmerReachedEnd = false;
bool farmerRotationComplete = false;
bool farmerMovingForward = true;
bool farmerMoving = false;
bool farmerCheck = false;

gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

gps::SkyBox mySkyBox;  
gps::Shader skyboxShader;


GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void setWireframeMode() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void setPolygonMode() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_9 && action == GLFW_PRESS)
		setWireframeMode();

	if (key == GLFW_KEY_0 && action == GLFW_PRESS)
		setPolygonMode();

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);
		firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - lastX;
	float yoffset = lastY - static_cast<float>(ypos);
	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);

	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void processMovement()
{
	bool cameraMoved = false;

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);	
		cameraMoved = true;
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
		cameraMoved = true;
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
		cameraMoved = true;
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);	
		cameraMoved = true;
	}

	if (pressedKeys[GLFW_KEY_X]) {
		farmerMoving = true;
		if (farmerMoving) {
			if (farmerMovingForward) {
				if (farmerPosition.z > farmerEndPosition.z) {
					farmerPosition.z -= farmerMovementSpeed; 
				}
				else {
					farmerReachedEnd = true;
					farmerMovingForward = false;
				}
			}
			else {
				if (farmerPosition.z < 4.0f) {
					farmerPosition.z += farmerMovementSpeed; 
				}
				else {
					farmerReachedEnd = true;
					farmerMovingForward = true;
				}
			}
			if (farmerReachedEnd && !farmerRotationComplete) {
				if (farmerMovingForward) {
					farmerRotation += 5.0f; 
					if (farmerRotation >= 180.0f) {
						farmerRotationComplete = true;
						farmerReachedEnd = false; 
					}
				}
				else {
					farmerRotation -= 5.0f;
					if (farmerRotation <= 0.0f) {
						farmerRotation = 0.0f; 
						farmerRotationComplete = true;
						farmerReachedEnd = false;
					}
				}
			}
			else if (farmerRotationComplete) {
				farmerRotationComplete = false;
			}
		}
	}

	if (cameraMoved) {
		glm::vec3 cameraPos = myCamera.getCameraPosition();
		std::cout << "Camera Position: x=" << cameraPos.x << ", y=" << cameraPos.y << ", z=" << cameraPos.z << std::endl;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	barn.LoadModel("objects/barn/barn.obj");
	house.LoadModel("objects/house/house.obj");
	chickenCoop.LoadModel("objects/chickencoop/chicken_coop.obj");
	silo.LoadModel("objects/silo/silo.obj");
	greenhouse.LoadModel("objects/greenhouse/greenhouse.obj");
	stable.LoadModel("objects/stable/stable.obj");
	dogsHouse.LoadModel("objects/dogshouse/dogs_house.obj");
	CatMac.LoadModel("objects/cat/CatMac.obj");
	farmer.LoadModel("objects/farmer/farmer.obj");
	ground.LoadModel("objects/ground/ground1.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	fence.LoadModel("objects/fence/fence.obj");
	firepit.LoadModel("objects/firepit/firepit.obj");
	shovel.LoadModel("objects/shovel/shovel.obj");
	tree.LoadModel("objects/tree/tree.obj");
	haybale.LoadModel("objects/haybale/Hay.obj");
	dog.LoadModel("objects/dog/dog.obj");
	lemon_tree.LoadModel("objects/lemon_tree/Lemon_fruit.obj");
	cart.LoadModel("objects/cart/Cart.obj");
	chicken.LoadModel("objects/chicken/chicken.obj");
	egg.LoadModel("objects/egg/egg.obj");
	pig.LoadModel("objects/pig/pig.obj");
	farmer3.LoadModel("objects/farmer3/farmer.obj");
	haystack.LoadModel("objects/haystack/haystack.obj");
	pigsty.LoadModel("objects/pigsty/pigsty.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();

	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	
	//depth map shader
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initCollisionObjects(gps::Camera& camera) {
	//barnModel collision objects
	camera.addCollisionObject({ glm::vec3(-3.8f, 0.0f, -3.8f), glm::vec3(3.8f, 10.0f, 3.8f) });

	//chickenCoopModel collision objects
	camera.addCollisionObject({ glm::vec3(5.78f, 0.0f, -12.22f), glm::vec3(10.22f, 10.0f, -7.82f) });

	//houseModel collision objects
	camera.addCollisionObject({ glm::vec3(10.3f, 0.0f, -6.7f), glm::vec3(15.7f, 10.0f, -1.2f) });

	//dogsHouseModel collision objects
	camera.addCollisionObject({ glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(12.0f, 3.0f, 2.0f) });

	//siloModel collision objects
	camera.addCollisionObject({ glm::vec3(-8.75f, 0.0f, -9.75f), glm::vec3(-5.25f, 10.0f, -6.25f) });

	//greenhouseModel collision objects
	camera.addCollisionObject({ glm::vec3(-3.5f, 0.0f, -15.5f), glm::vec3(-0.5f, 5.0f, -10.5f) });

	//stableModel collision objects
	camera.addCollisionObject({ glm::vec3(1.75f, 0.0f, -19.0f), glm::vec3(4.25f, 5.0f, -15.0f) });

	// Fence collision objects
	camera.addCollisionObject({ glm::vec3(-13.0f, 0.0f, 9.7f), glm::vec3(25.0f, 3.0f, 15.0f) });
	camera.addCollisionObject({ glm::vec3(20.4f, 0.0f, -25.0f), glm::vec3(23.0f, 3.0f, 9.7f) });
	camera.addCollisionObject({ glm::vec3(-15.0f, 0.0f, -24.0f), glm::vec3(21.3f, 3.0f, -22.0f)});
	camera.addCollisionObject({ glm::vec3(-15.0f, 0.0f, -25.0f), glm::vec3(-11.4f, 3.0f, 12.0f) });

	//Tree collision objects
	camera.addCollisionObject({ glm::vec3(-8.29041f, 0.0f, 2.26009f), glm::vec3(-5.852f, 3.0f, 5.01825f) });

	//Cart collision objects
	camera.addCollisionObject({ glm::vec3(-7.95237f, 0.0f, -17.3518f), glm::vec3(-5.67422f, 3.0f, -14.1182f)});

	//Haybale collision objects
	camera.addCollisionObject({ glm::vec3(-10.7695f, 0.0f, -20.4823f), glm::vec3(-5.34691f, 3.0f, -19.3366f) });

	//Pigsty collision objects
	camera.addCollisionObject({ glm::vec3(6.40346f, 0.0f, -20.5322f), glm::vec3(11.8951f, 3.0f, -15.4632f) });
}

void initUniforms() {
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
	lightDir = glm::vec3(0.0f, 20.0f, 20.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir)); 

	//send point light position
	glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos"), 1, glm::value_ptr(glm::vec3(16.0f, 0.5f, -16.0f)));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	// Set texture scale
	GLuint textureScaleLoc = glGetUniformLocation(myCustomShader.shaderProgram, "textureScale");
	glUniform1f(textureScaleLoc, 1.0f); // Set the texture scale to 1.0f (or any other value you need)

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO); 
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO 
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/CloudyCrown_Midday_Right.png");
	faces.push_back("skybox/CloudyCrown_Midday_Left.png");
	faces.push_back("skybox/CloudyCrown_Midday_Top.png");
	faces.push_back("skybox/CloudyCrown_Midday_Bottom.png");
	faces.push_back("skybox/CloudyCrown_Midday_Back.png");
	faces.push_back("skybox/CloudyCrown_Midday_Front.png");
	mySkyBox.Load(faces);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	//glm::vec3 newLightDir = glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir;
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 80.0f; 
	glm::mat4 lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView; 
	return lightSpaceTrMatrix;
}

void renderGround(gps::Shader shader, bool depthPass) {
	for (int i = 0; i < 50; ++i) {
		for (int j = 0; j < 50; ++j) {
			glm::mat4 groundModel = glm::mat4(1.0f);
			groundModel = glm::translate(groundModel, glm::vec3(-100 + i * 4.0f, 0.0f, -100 + j * 4.0f)); 
			groundModel = glm::scale(groundModel, glm::vec3(1.0f, 1.0f, 1.0f));
			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(groundModel));

			if (!depthPass) {
				normalMatrix = glm::mat3(glm::inverseTranspose(view * groundModel));
				glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
			}

			ground.Draw(shader);
		}
	}
}

void renderBarn(gps::Shader shader, bool depthPass) {
	glm::mat4 barnModel = glm::mat4(1.0f);
	barnModel = glm::translate(barnModel, glm::vec3(0.0f, 0.0f, 0.0f));
	barnModel = glm::scale(barnModel, glm::vec3(60.0f, 60.0f, 60.0f));
	barnModel = glm::rotate(barnModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(barnModel));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * barnModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	barn.Draw(shader);
}

void renderHouse(gps::Shader shader, bool depthPass) {
	glm::mat4 houseModelMatrix = glm::mat4(1.0f);
	houseModelMatrix = glm::translate(houseModelMatrix, glm::vec3(13.0f, 0.0f, -4.0f));
	houseModelMatrix = glm::scale(houseModelMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	houseModelMatrix = glm::rotate(houseModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(houseModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * houseModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	house.Draw(shader);
}

void renderChickenCoop(gps::Shader shader, bool depthPass) {
	glm::mat4 chickenCoopModelMatrix = glm::mat4(1.0f);
	chickenCoopModelMatrix = glm::translate(chickenCoopModelMatrix, glm::vec3(8.0f, 0.0f, -10.0f));
	chickenCoopModelMatrix = glm::scale(chickenCoopModelMatrix, glm::vec3(60.0f, 60.0f, 60.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(chickenCoopModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * chickenCoopModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	chickenCoop.Draw(shader);
}

void renderSilo(gps::Shader shader, bool depthPass) {
	glm::mat4 siloModelMatrix = glm::mat4(1.0f);
	siloModelMatrix = glm::translate(siloModelMatrix, glm::vec3(-7.0f, 0.0f, -8.0f));
	siloModelMatrix = glm::scale(siloModelMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	siloModelMatrix = glm::rotate(siloModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(siloModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * siloModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	silo.Draw(shader);
}

void renderGreenhouse(gps::Shader shader, bool depthPass) {
	glm::mat4 greenhouseModelMatrix = glm::mat4(1.0f);
	greenhouseModelMatrix = glm::translate(greenhouseModelMatrix, glm::vec3(-2.0f, 0.0f, -13.0f));
	greenhouseModelMatrix = glm::scale(greenhouseModelMatrix, glm::vec3(72.0f, 72.0f, 72.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(greenhouseModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * greenhouseModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	greenhouse.Draw(shader);
}

void renderStable(gps::Shader shader, bool depthPass) {
	glm::mat4 stableModelMatrix = glm::mat4(1.0f);
	stableModelMatrix = glm::translate(stableModelMatrix, glm::vec3(3.0f, 0.0f, -17.0f));
	stableModelMatrix = glm::scale(stableModelMatrix, glm::vec3(60.0f, 60.0f, 60.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(stableModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * stableModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	stable.Draw(shader);
}

void renderDogsHouse(gps::Shader shader, bool depthPass) {
	glm::mat4 dogsHouseModelMatrix = glm::mat4(1.0f);
	dogsHouseModelMatrix = glm::translate(dogsHouseModelMatrix, glm::vec3(11.0f, 0.0f, 1.0f));
	dogsHouseModelMatrix = glm::scale(dogsHouseModelMatrix, glm::vec3(60.0f, 60.0f, 60.0f));
	dogsHouseModelMatrix = glm::rotate(dogsHouseModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(dogsHouseModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * dogsHouseModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	dogsHouse.Draw(shader);
}

void renderRiggedCat(gps::Shader shader, bool depthPass) {
	glm::mat4 catMacModelMatrix = glm::mat4(1.0f);
	glm::vec3 dogHousePosition = glm::vec3(11.0f, 0.0f, 1.0f);
	glm::vec3 catPosition = dogHousePosition + glm::vec3(-1.5f, 0.0f, 0.0f);
	catMacModelMatrix = glm::translate(catMacModelMatrix, catPosition);
	catMacModelMatrix = glm::scale(catMacModelMatrix, glm::vec3(0.02f, 0.02f, 0.02f));

	glm::vec3 cameraPosition = myCamera.getCameraPosition();
	glm::vec3 direction = glm::normalize(cameraPosition - catPosition);
	float angleC = atan2(direction.x, direction.z);
	catMacModelMatrix = glm::rotate(catMacModelMatrix, angleC, glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(catMacModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * catMacModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	CatMac.Draw(shader);
}

void renderFarmer(gps::Shader shader, bool depthPass) {
	shader.useShaderProgram();

	glm::mat4 farmerModelMatrix = glm::mat4(1.0f);
	farmerModelMatrix = glm::translate(farmerModelMatrix, farmerPosition); 
	farmerModelMatrix = glm::rotate(farmerModelMatrix, glm::radians(farmerRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	farmerModelMatrix = glm::scale(farmerModelMatrix, glm::vec3(0.6f, 0.6f, 0.6f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(farmerModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * farmerModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	farmer.Draw(shader);
}

void renderFarmer2(gps::Shader shader, bool depthPass) {
	shader.useShaderProgram();

	glm::mat4 farmerModelMatrix = glm::mat4(1.0f);
	farmerModelMatrix = glm::translate(farmerModelMatrix, glm::vec3(14.5f,0.0f,-16.0f));
	farmerModelMatrix = glm::rotate(farmerModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	farmerModelMatrix = glm::scale(farmerModelMatrix, glm::vec3(0.6f, 0.6f, 0.6f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(farmerModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * farmerModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	farmer.Draw(shader);
}

void renderFence(gps::Shader shader, bool depthPass) {
	glm::mat4 fenceModelMatrix = glm::mat4(1.0f);

	for (int i = 0; i < 8; ++i) {
		fenceModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-16.0f + i * 4.0f, 0.0f, 10.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fenceModelMatrix));
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fenceModelMatrix));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		fence.Draw(shader);
	}
	for (int i = 0; i < 8; ++i) {
		fenceModelMatrix = glm::mat4(1.0f);
		fenceModelMatrix = glm::translate(fenceModelMatrix, glm::vec3(20.5f, 0.0f, 14.5f - i * 4.0f));
		fenceModelMatrix = glm::rotate(fenceModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fenceModelMatrix));
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fenceModelMatrix));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		fence.Draw(shader);
	}
	for (int i = 0; i < 8; ++i) {
		fenceModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-11.5f, 0.0f, 1.5f - i * 4.0f));
		fenceModelMatrix = glm::rotate(fenceModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fenceModelMatrix));
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fenceModelMatrix));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		fence.Draw(shader);
	}
	for (int i = 0; i < 8; ++i) {
		fenceModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(25.0f - i * 4.0f, 0.0f, -22.0f));
		fenceModelMatrix = glm::rotate(fenceModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(fenceModelMatrix));
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * fenceModelMatrix));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}
		fence.Draw(shader);
	}
}

void renderFirepit(gps::Shader shader, bool depthPass) {
	glm::mat4 firepitModelMatrix = glm::mat4(1.0f);
	firepitModelMatrix = glm::translate(firepitModelMatrix, glm::vec3(16.0f, 0.0f, -16.0f));
	firepitModelMatrix = glm::scale(firepitModelMatrix, glm::vec3(0.17f, 0.17f, 0.17f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(firepitModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * firepitModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	firepit.Draw(shader);
}

void renderShovel(gps::Shader shader, bool depthPass) {
    glm::vec3 cameraPos = myCamera.getCameraPosition();
    glm::vec3 cameraFront = myCamera.getCameraFrontDirection();
    glm::vec3 cameraRight = myCamera.getCameraRightDirection();
    glm::vec3 cameraUp = myCamera.getCameraUpDirection();

	glm::vec3 shovelPosition = cameraPos + cameraFront * 0.5f + cameraRight * 0.3f - cameraUp * 0.2f;

    glm::mat4 shovelModelMatrix = glm::mat4(1.0f);
    shovelModelMatrix = glm::translate(shovelModelMatrix, shovelPosition);
    shovelModelMatrix = glm::rotate(shovelModelMatrix, glm::radians(-yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    shovelModelMatrix = glm::rotate(shovelModelMatrix, glm::radians(-pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	shovelModelMatrix = glm::rotate(shovelModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	shovelModelMatrix = glm::rotate(shovelModelMatrix, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	shovelModelMatrix = glm::scale(shovelModelMatrix, glm::vec3(1.1f, 1.1f, 1.1f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(shovelModelMatrix));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * shovelModelMatrix));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    shovel.Draw(shader);
}

void renderTree(gps::Shader shader, bool depthPass) {
	glm::mat4 treeModelMatrix = glm::mat4(1.0f);
	treeModelMatrix = glm::translate(treeModelMatrix, glm::vec3(-7.0f, 0.0f, 3.0f));
	treeModelMatrix = glm::scale(treeModelMatrix, glm::vec3(40.0f, 40.0f, 40.0f));
	treeModelMatrix = glm::rotate(treeModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(treeModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * treeModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	tree.Draw(shader);
}

void renderHaybale(gps::Shader shader, bool depthPass) {
	glm::mat4 haybaleModelMatrix = glm::mat4(1.0f);
	haybaleModelMatrix = glm::translate(haybaleModelMatrix, glm::vec3(-8.0f, 1.3f, -20.0f));
	haybaleModelMatrix = glm::scale(haybaleModelMatrix, glm::vec3(0.02f, 0.02f, 0.02f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(haybaleModelMatrix));
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * haybaleModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	haybale.Draw(shader);
}

void renderDog(gps::Shader shader, bool depthPass) {
	glm::mat4 dogModelMatrix = glm::mat4(1.0f);
	dogModelMatrix = glm::translate(dogModelMatrix, glm::vec3(18.0f, 0.0f, -16.0f));
	dogModelMatrix = glm::scale(dogModelMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	dogModelMatrix = glm::rotate(dogModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(dogModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * dogModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	dog.Draw(shader);
}

void renderLemonTree(gps::Shader shader, bool depthPass) {
	glm::mat4 lemonTreeModelMatrix = glm::mat4(1.0f);
	lemonTreeModelMatrix = glm::translate(lemonTreeModelMatrix, glm::vec3(0.0f, 0.0f, -19.0f));
	lemonTreeModelMatrix = glm::scale(lemonTreeModelMatrix, glm::vec3(0.07f, 0.07f, 0.07f));
	lemonTreeModelMatrix = glm::rotate(lemonTreeModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lemonTreeModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * lemonTreeModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lemon_tree.Draw(shader);
}

void renderLemonTree2(gps::Shader shader, bool depthPass) {
	glm::mat4 lemonTreeModelMatrix = glm::mat4(1.0f);
	lemonTreeModelMatrix = glm::translate(lemonTreeModelMatrix, glm::vec3(17.0f, 0.0f, -20.0f));
	lemonTreeModelMatrix = glm::scale(lemonTreeModelMatrix, glm::vec3(0.06f, 0.06f, 0.06f));
	lemonTreeModelMatrix = glm::rotate(lemonTreeModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lemonTreeModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * lemonTreeModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lemon_tree.Draw(shader);
}

void renderLemonTree3(gps::Shader shader, bool depthPass) {
	glm::mat4 lemonTreeModelMatrix = glm::mat4(1.0f);
	lemonTreeModelMatrix = glm::translate(lemonTreeModelMatrix, glm::vec3(17.0f, 0.0f, 6.0f));
	lemonTreeModelMatrix = glm::scale(lemonTreeModelMatrix, glm::vec3(0.08f, 0.08f, 0.08f));
	lemonTreeModelMatrix = glm::rotate(lemonTreeModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lemonTreeModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * lemonTreeModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lemon_tree.Draw(shader);
}

void renderCart(gps::Shader shader, bool depthPass) {
	glm::mat4 cartModel = glm::mat4(1.0f);
	cartModel = glm::translate(cartModel, glm::vec3(-7.0f, 0.77f, -16.0f));
	cartModel = glm::scale(cartModel, glm::vec3(0.03f, 0.03f, 0.03f));
	cartModel = glm::rotate(cartModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	cartModel = glm::translate(cartModel, glm::vec3(-7.1f, 0.9f, -16.1f));
	cartModel = glm::rotate(cartModel, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cartModel));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * cartModel));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	cart.Draw(shader);
}

void renderChicken(gps::Shader shader, bool depthPass) {
	glm::mat4 chickenModelMatrix = glm::mat4(1.0f);
	chickenModelMatrix = glm::translate(chickenModelMatrix, glm::vec3(7.78f, 0.347f, -6.51043f));
	chickenModelMatrix = glm::scale(chickenModelMatrix, glm::vec3(0.18f, 0.18f, 0.18f));
	chickenModelMatrix = glm::rotate(chickenModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(chickenModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * chickenModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	chicken.Draw(shader);
}

void renderEgg(gps::Shader shader, bool depthPass) {
	glm::mat4 eggModelMatrix = glm::mat4(1.0f);
	eggModelMatrix = glm::translate(eggModelMatrix, glm::vec3(7.78f, 0.07f, -7.00671f));
	eggModelMatrix = glm::scale(eggModelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
	eggModelMatrix = glm::rotate(eggModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(eggModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * eggModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	egg.Draw(shader);
}

void renderPig(gps::Shader shader, bool depthPass) {
	glm::mat4 pigModelMatrix = glm::mat4(1.0f);
	pigModelMatrix = glm::translate(pigModelMatrix, glm::vec3(2.92f, 0.0f, -14.2015f));
	pigModelMatrix = glm::scale(pigModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	pigModelMatrix = glm::rotate(pigModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(pigModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * pigModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	pig.Draw(shader);
}

void renderFarmer3(gps::Shader shader, bool depthPass) {
	glm::mat4 farmer3ModelMatrix = glm::mat4(1.0f);
	farmer3ModelMatrix = glm::translate(farmer3ModelMatrix, glm::vec3(-4.5f, 0.0f, -13.0f));
	farmer3ModelMatrix = glm::scale(farmer3ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	farmer3ModelMatrix = glm::rotate(farmer3ModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(farmer3ModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * farmer3ModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	farmer3.Draw(shader);
}

void renderHaystack(gps::Shader shader, bool depthPass) {
	glm::mat4 haystackModelMatrix = glm::mat4(1.0f);
	haystackModelMatrix = glm::translate(haystackModelMatrix, glm::vec3(0.0f, 0.8f, 7.5f));
	haystackModelMatrix = glm::scale(haystackModelMatrix, glm::vec3(1.3f, 1.3f, 1.3f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(haystackModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * haystackModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	haystack.Draw(shader);
}

void renderHaystack2(gps::Shader shader, bool depthPass) {
	glm::mat4 haystackModelMatrix = glm::mat4(1.0f);
	haystackModelMatrix = glm::translate(haystackModelMatrix, glm::vec3(1.7f, 1.68f, 5.0f)); 
	haystackModelMatrix = glm::scale(haystackModelMatrix, glm::vec3(1.3f, 1.3f, 1.3f));
	haystackModelMatrix = glm::rotate(haystackModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(haystackModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * haystackModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	haystack.Draw(shader);
}

void renderPigsty(gps::Shader shader, bool depthPass) {
	glm::mat4 pigstyModelMatrix = glm::mat4(1.0f);
	pigstyModelMatrix = glm::translate(pigstyModelMatrix, glm::vec3(9.0f, 0.0f, -18.0f));
	pigstyModelMatrix = glm::scale(pigstyModelMatrix, glm::vec3(1.3f, 1.3f, 1.3f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(pigstyModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * pigstyModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	pigsty.Draw(shader);
}

void renderPig2(gps::Shader shader, bool depthPass) {
	glm::mat4 pigModelMatrix = glm::mat4(1.0f);
	pigModelMatrix = glm::translate(pigModelMatrix, glm::vec3(9.0f, 0.1f, -18.0f));
	pigModelMatrix = glm::scale(pigModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
	pigModelMatrix = glm::rotate(pigModelMatrix, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(pigModelMatrix));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * pigModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	pig.Draw(shader);
}

void drawObjects(gps::Shader shader, bool depthPass) {
	shader.useShaderProgram();
	
	renderGround(shader, depthPass);
	renderBarn(shader, depthPass);
	renderHouse(shader, depthPass);
	renderChickenCoop(shader, depthPass);
	renderSilo(shader, depthPass);
	renderGreenhouse(shader, depthPass);
	renderStable(shader, depthPass);
	renderDogsHouse(shader, depthPass);
	renderRiggedCat(shader, depthPass);
	renderFarmer(shader, depthPass);
	renderFarmer2(shader, depthPass);
	renderFence(shader, depthPass);
	renderFirepit(shader, depthPass);
	renderShovel(shader, depthPass);
	renderTree(shader, depthPass);
	renderHaybale(shader, depthPass);
	renderDog(shader, depthPass);
	renderLemonTree(shader, depthPass);
	renderLemonTree2(shader, depthPass);
	renderLemonTree3(shader, depthPass);
	renderCart(shader, depthPass);
	renderChicken(shader, depthPass);
	renderEgg(shader, depthPass);
	renderPig(shader, depthPass);
	renderFarmer3(shader, depthPass);
	renderHaystack(shader, depthPass);
	renderHaystack2(shader, depthPass);
	renderPigsty(shader, depthPass);
	renderPig2(shader, depthPass);
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map
	
	//render the scene to the depth buffer
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	drawObjects(depthMapShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(16.0f, 0.5f, -16.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);

		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initCollisionObjects(myCamera);
	initFBO();
	initSkybox();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
