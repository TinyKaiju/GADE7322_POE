#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

//GLFW
#include <GLFW/glfw3.h>

// SOIL2
#include "SOIL2/SOIL2.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Link Shader File
#include "Shader.h"

//Link Camera File
#include "Camera.h"  //camera 

const GLint WIDTH = 1920, HEIGHT = 1080;
int SCREEN_WIDTH, SCREEN_HEIGHT; // Replace all screenW & screenH with these

// Function declaration
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos); // Get mouse pos in order to hide it
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow* window);

// Initialise camera values
Camera camera(glm::vec3(0.0f, 2.0f, 12.0f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024]; // Array of 1024 different types of keys
bool firstMouse = true; // Only handling one type of mouse, thus true
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Switch Cameras
bool camLocked = true;
bool animate = false;

GLfloat AnimateCPRotation();
glm::vec3 AnimatePosition(glm::vec3 pos);
//glm::vec3 LightPos(1.0f, 1.2f, 3.0f);

int main()
{
	//Initialise GLFW
	glfwInit();

	// GLFW Version Hints	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GADE7322", nullptr, nullptr);

	//Get Screen Resolution
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// check if window is created succesfully
	if (nullptr == window)
	{
		std::cout << "Failed to Create Window." << endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}


	glfwMakeContextCurrent(window); //exit

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	GLuint CreateSkyboxTexture(GLuint texture, vector<std::string> faces, int width, int height);

	// Center  and Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//enable glew
	glewExperimental = GL_TRUE;

	//Initialize GLEW
	if (GLEW_OK != glewInit())
	{
		cout << "FAILED TO INITIALISE GLEW." << endl;
		return EXIT_FAILURE;
	}
	// Setup OpenGL viewport
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	//Enable depth 
	glEnable(GL_DEPTH_TEST);

	//Enable alpha support
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#pragma region Height Map
	Shader shaderHM("CoreHM.vs", "CoreHM.frag");

	int widthHM, heightHM, nrChannels;

	//Assign Height map
	unsigned char* dataHM = SOIL_load_image("res/images/HM1.jpg", &widthHM, &heightHM, &nrChannels, 0);

	// Check if Height Map was loaded succesfully
	if (dataHM)
	{
		cout << "Loaded heightmap of size " << heightHM << " x " << widthHM << endl;
	}
	else
	{
		cout << "Failed to load texture" << endl;
	}

	// set up vertex data (and buffer(s)) and configure vertex attributes
	vector<GLfloat> verticesHM;
	GLfloat yScale = 12.0f / 256.0f; //normalize the height map data and scale it to the desired height
	GLfloat yShift = 10.0f; //translate map y value
	int rez = 1;
	GLuint bytePerPixel = nrChannels;

	for (int i = 0; i < heightHM; i++)
	{
		for (int j = 0; j < widthHM; j++)
		{
			unsigned char* pixelOffset = dataHM + (j + widthHM * i) * bytePerPixel;
			unsigned char y = pixelOffset[0];

			// vertex
			verticesHM.push_back(-heightHM / 2.0f + heightHM * i / (float)heightHM); // vx
			verticesHM.push_back((int)y * yScale - yShift); // vy
			verticesHM.push_back(-widthHM / 2.0f + widthHM * j / (float)widthHM); // vz
		}
	}
	cout << "Loaded " << verticesHM.size() / 3 << " vertices" << endl;
	SOIL_free_image_data(dataHM);

	vector<GLuint> indicesHM;
	for (int i = 0; i < heightHM - 1; i += rez)
	{
		for (int j = 0; j < widthHM; j += rez)
		{
			for (int k = 0; k < 2; k++)
			{
				indicesHM.push_back(j + widthHM * (i + k * rez));
			}
		}
	}
	cout << "Loaded " << indicesHM.size() << " indices" << endl;

	const int numStrips = (heightHM - 1) / rez;
	const int numTrisPerStrip = (widthHM / rez) * 2 - 2;
	cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << endl;
	cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << endl;

	// Generate the vertex arrays, vertex buffers and index buffers and save them into variables
	unsigned int VOA_HM, VBO, IBO;
	glGenVertexArrays(1, &VOA_HM);
	glBindVertexArray(VOA_HM);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesHM.size() * sizeof(float), &verticesHM[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute For Height Map Texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat))); //Texture
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesHM.size() * sizeof(unsigned), &indicesHM[0], GL_STATIC_DRAW);

#pragma endregion

#pragma region  Height Map Texture

	GLuint textureHM;
	int width_HM, height_HM;

	glGenTextures(1, &textureHM);
	glBindTexture(GL_TEXTURE_2D, textureHM);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* HM_Image = SOIL_load_image("res/images/water.png", &width_HM, &height_HM, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_HM, height_HM, 0, GL_RGBA, GL_UNSIGNED_BYTE, HM_Image);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(HM_Image);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion


#pragma region SkyBox Shader

	Shader skyboxShader("Skybox.vs", "SkyBox.frag");
	
	float skyboxVertices[] = {
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	// Generate the vertex arrays and vertex buffers and save them into variables
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	// Bind the vertex array object
	glBindVertexArray(skyboxVAO);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Load textures
	vector<std::string> skyboxFaces
	{
		"res/images/Skyboxs/Pink/px.png",
		"res/images/Skyboxs/Pink/nx.png",
		"res/images/Skyboxs/Pink/py.png",
		"res/images/Skyboxs/Pink/ny.png",
		"res/images/Skyboxs/Pink/pz.png",
		"res/images/Skyboxs/Pink/nz.png"
	};


	// Skybox texture variable
	GLuint skyboxTexture = 0;

	int widthTexture = 1024;
	int heightTexture = 1024;

	skyboxTexture = CreateSkyboxTexture(skyboxTexture, skyboxFaces, widthTexture, heightTexture);

#pragma endregion 

#pragma region Light shader
	//Shader LightingShader("Lighting.vs", "Lighting.frag");
	//Shader LampShader("Lamp.vs", "Lamp.frag");
#pragma endregion


#pragma region BUILD AND COMPILE SHADER - CHESSBOARD

	Shader chessboardShader("CoreCB.vs", "CoreCB.frag");

	// Set vertex data for our cube
	GLfloat verticesBoard[] =
	{
		//Positions                //Texture Coords
		-0.5f, 0.0f, -0.5f,        0.0f, 0.0f,
		0.5f, 0.0f, -0.5f,        1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
		-0.5f, 0.0f, -0.5f,        0.0f, 0.0f, // Left

		-0.5f, 0.0f,  0.5f,        0.0f, 0.0f,
		0.5f, 0.0f,  0.5f,        1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,        1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,        1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,    0.0f, 1.0f,
		-0.5f, 0.0f,  0.5f,        0.0f, 0.0f, // Right

		-0.5f,  0.5f,  0.5f,    1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,    1.0f, 1.0f,
		-0.5f, 0.0f, -0.5f,        0.0f, 1.0f,
		-0.5f, 0.0f, -0.5f,        0.0f, 1.0f,
		-0.5f, 0.0f,  0.5f,        0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,    1.0f, 0.0f, // Back

		0.5f,  0.5f,  0.5f,        1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
		0.5f, 0.0f, -0.5f,        0.0f, 1.0f,
		0.5f, 0.0f, -0.5f,        0.0f, 1.0f,
		0.5f, 0.0f,  0.5f,        0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,        1.0f, 0.0f, // Front

		-0.5f, 0.0f, -0.5f,        0.0f, 1.0f,
		0.5f, 0.0f, -0.5f,        1.0f, 1.0f,
		0.5f, 0.0f,  0.5f,        1.0f, 0.0f,
		0.5f, 0.0f,  0.5f,        1.0f, 0.0f,
		-0.5f, 0.0f,  0.5f,        0.0f, 0.0f,
		-0.5f, 0.0f, -0.5f,        0.0f, 1.0f, // Bottom

		-0.5f,  0.5f, -0.5f,    0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,        1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,        1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,    0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,    0.0f, 1.0f // Top
	};

	GLfloat verticesBorder[] =
	{
		//Positions                //Texture Coords
		-0.25f, 0.0f, -0.25f,        0.0f, 0.0f,
		0.25f, 0.0f, -0.25f,        1.0f, 0.0f,
		0.25f,  0.5f, -0.25f,        1.0f, 1.0f,
		0.25f,  0.5f, -0.25f,        1.0f, 1.0f,
		-0.25f,  0.5f, -0.25f,    0.0f, 1.0f,
		-0.25f, 0.0f, -0.25f,        0.0f, 0.0f, // Left

		-0.25f, 0.0f,  0.25f,        0.0f, 0.0f,
		0.25f, 0.0f,  0.25f,        1.0f, 0.0f,
		0.25f,  0.5f,  0.25f,        1.0f, 1.0f,
		0.25f,  0.5f,  0.25f,        1.0f, 1.0f,
		-0.25f,  0.5f,  0.25f,    0.0f, 1.0f,
		-0.25f, 0.0f,  0.25f,        0.0f, 0.0f, // Right

		-0.25f,  0.5f,  0.25f,    1.0f, 0.0f,
		-0.25f,  0.5f, -0.25f,    1.0f, 1.0f,
		-0.25f, 0.0f, -0.25f,        0.0f, 1.0f,
		-0.25f, 0.0f, -0.25f,        0.0f, 1.0f,
		-0.25f, 0.0f,  0.25f,        0.0f, 0.0f,
		-0.25f,  0.5f,  0.25f,    1.0f, 0.0f, // Back

		0.25f,  0.5f,  0.25f,        1.0f, 0.0f,
		0.25f,  0.5f, -0.25f,        1.0f, 1.0f,
		0.25f, 0.0f, -0.25f,        0.0f, 1.0f,
		0.25f, 0.0f, -0.25f,        0.0f, 1.0f,
		0.25f, 0.0f,  0.25f,        0.0f, 0.0f,
		0.25f,  0.5f,  0.25f,        1.0f, 0.0f, // Front

		-0.25f, 0.0f, -0.25f,        0.0f, 1.0f,
		0.25f, 0.0f, -0.25f,        1.0f, 1.0f,
		0.25f, 0.0f,  0.25f,        1.0f, 0.0f,
		0.25f, 0.0f,  0.25f,        1.0f, 0.0f,
		-0.25f, 0.0f,  0.25f,        0.0f, 0.0f,
		-0.25f, 0.0f, -0.25f,        0.0f, 1.0f, // Bottom

		-0.25f,  0.5f, -0.25f,    0.0f, 1.0f,
		0.25f,  0.5f, -0.25f,        1.0f, 1.0f,
		0.25f,  0.5f,  0.25f,        1.0f, 0.0f,
		0.25f,  0.5f,  0.25f,        1.0f, 0.0f,
		-0.25f,  0.5f,  0.25f,    0.0f, 0.0f,
		-0.25f,  0.5f, -0.25f,    0.0f, 1.0f // Top
	};

	// Positions of different cubes
	glm::vec3 cubePositions[] =
	{
		glm::vec3(-3.0f, 0.0f, 4.0f),
		glm::vec3(-2.0f, 0.0f, 4.0f),
		glm::vec3(-1.0f, 0.0f, 4.0f),
		glm::vec3(0.0f, 0.0f, 4.0f),
		glm::vec3(1.0f, 0.0f, 4.0f),
		glm::vec3(2.0f, 0.0f, 4.0f),
		glm::vec3(3.0f, 0.0f, 4.0f),
		glm::vec3(4.0f, 0.0f, 4.0f)
	};

	float randY[8][8];

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			randY[i][j] = (((rand() % 3)) * 0.1f) - 0.1f;
		}
	}

	glm::vec3 borderPositions[] =
	{
		glm::vec3(-3.75f, 0, 4.0f),
		glm::vec3(4.75f, 0.0f, 4.0f),
		glm::vec3(-3.75f, 0.0f, -3.75f),
		glm::vec3(4.75f, 0.0f, 4.75f),
		glm::vec3(-3.75f, 0.0f, 4.75f),
		glm::vec3(4.75f, 0.0f, -3.75f)
	};

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VBA_Board, VOA_Board;
	glGenVertexArrays(1, &VOA_Board);
	glGenBuffers(1, &VBA_Board);

	// Bind the vertex array object
	glBindVertexArray(VOA_Board);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Board);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBoard), verticesBoard, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); //Texture
	glEnableVertexAttribArray(2);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VOA_LightBoard;
	glGenVertexArrays(1, &VOA_LightBoard);
	glGenBuffers(1, &VBA_Board);

	// Bind the vertex array object
	glBindVertexArray(VOA_LightBoard);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Board);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBoard), verticesBoard, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

#pragma region Chessboard Texture

	// Chessboard texture variables
	GLuint textureWhite, textureBlack, textureGrey;
	int widthB, heightB;


#pragma region White Texture

	// Create and load White texture
	glGenTextures(1, &textureWhite);
	glBindTexture(GL_TEXTURE_2D, textureWhite);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* whiteBlock = SOIL_load_image("res/images/Light square.JPG", &widthB, &heightB, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthB, heightB, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteBlock);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(whiteBlock);
	glBindTexture(GL_TEXTURE_2D, 0);
#pragma endregion

#pragma region Black Texture

	//Create Black texture
	glGenTextures(1, &textureBlack);
	glBindTexture(GL_TEXTURE_2D, textureBlack);

	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Load texture
	unsigned char* blackBlock = SOIL_load_image("res/images/Dark Square 2.JPG", &widthB, &heightB, 0, SOIL_LOAD_RGBA);

	//Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthB, heightB, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackBlock);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(blackBlock);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma region Border Texture

	//Create Black texture
	glGenTextures(1, &textureGrey);
	glBindTexture(GL_TEXTURE_2D, textureGrey);

	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Load texture
	unsigned char* greyBlock = SOIL_load_image("res/images/Paper.png", &widthB, &heightB, 0, SOIL_LOAD_RGBA);

	//Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthB, heightB, 0, GL_RGBA, GL_UNSIGNED_BYTE, greyBlock);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(greyBlock);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma endregion

#pragma endregion

#pragma region Build and Compile Shader - Chess Pieces

#pragma region Pawn
	int i = 0;

	//Build & Compile Shader Program for Pawn Pieces
	Shader ourShaderPawn("CoreCB.vs", "CoreCB.frag");

	// Vertex data for our pawn piece
	GLfloat verticesPawn[24264];

	// Read Vertex data from pawn.txt file //

	ifstream myFile("res/3D models/OBJ Files/pawn.txt");
	i = 0;

	if (myFile.is_open())
	{
		string line;

		while (!myFile.eof())
		{
			getline(myFile, line, ' ');
			//cout << "Val 1: " << line << endl;
			verticesPawn[i] = stof(line);
			i++;
			getline(myFile, line, ' ');
			//cout << "Val 2: " << line << endl;
			verticesPawn[i] = stof(line);
			i++;
			getline(myFile, line, '\n');
			//cout << "Val 3: " << line << endl;
			verticesPawn[i] = stof(line);
			i++;
		}
		myFile.close();
	}
	else
	{
		cout << "Can't open the file";
	}
	// Read Vertex data from pawn.txt file //

	// Positions of pawns
	glm::vec3 pawnPositions[] =
	{
		// Row 1
		glm::vec3(-3.0f, 0.5f, 3.0f), //X,Y,Z
		glm::vec3(-2.0f, 0.5f, 3.0f),
		glm::vec3(-1.0f, 0.5f, 3.0f),
		glm::vec3(0.0f, 0.5f, 3.0f),
		glm::vec3(1.0f, 0.5f, 3.0f),
		glm::vec3(2.0f, 0.5f, 3.0f),
		glm::vec3(3.0f, 0.5f, 3.0f),
		glm::vec3(4.0f, 0.5f, 3.0f),

		// Row 2
		glm::vec3(-3.0f, 0.5f, -2.0f),
		glm::vec3(-2.0f, 0.5f, -2.0f),
		glm::vec3(-1.0f, 0.5f, -2.0f),
		glm::vec3(0.0f, 0.5f, -2.0f),
		glm::vec3(1.0f, 0.5f, -2.0f),
		glm::vec3(2.0f, 0.5f, -2.0f),
		glm::vec3(3.0f, 0.5f, -2.0f),
		glm::vec3(4.0f, 0.5f, -2.0f),
	};

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VBA_Pawn, VOA_Pawn;
	glGenVertexArrays(1, &VOA_Pawn);
	glGenBuffers(1, &VBA_Pawn);

	// Bind the vertex array object
	glBindVertexArray(VOA_Pawn);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Pawn);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPawn), verticesPawn, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
	glEnableVertexAttribArray(2);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

#pragma region Pawn Textures

	//Chess Piece Pawn texture variables
	GLuint pawnTextureW, pawnTextureB;
	int widthPawn, heightPawn;

#pragma region Light Texture

	// Create and load White texture
	glGenTextures(1, &pawnTextureW);
	glBindTexture(GL_TEXTURE_2D, pawnTextureW);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* pawnImageW = SOIL_load_image("res/images/Light square.png", &widthPawn, &heightPawn, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthPawn, heightPawn, 0, GL_RGBA, GL_UNSIGNED_BYTE, pawnImageW);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(pawnImageW);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma region Dark Texture 

	// Create and load Black texture
	glGenTextures(1, &pawnTextureB);
	glBindTexture(GL_TEXTURE_2D, pawnTextureB);

	// Set texture parameters 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* pawnImageB = SOIL_load_image("res/images/Dark square 2.png", &widthPawn, &heightPawn, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthPawn, heightPawn, 0, GL_RGBA, GL_UNSIGNED_BYTE, pawnImageB);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(pawnImageB);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion



#pragma endregion





#pragma endregion

#pragma region Rook
	//Build & Compile Shader Program for Pawn Pieces
	Shader ourShaderRook("CoreCB.vs", "CoreCB.frag");

	// Vertex data for our pawn piece
	GLfloat verticesRook[24876];

	// Read Vertex data from pawn.txt file //
	ifstream myFile2("res/3D models/OBJ Files/rook.txt");
	i = 0;

	if (myFile2.is_open())
	{
		string line;

		while (!myFile2.eof())
		{
			getline(myFile2, line, ' ');
			//cout << "Val 1: " << line << endl;
			verticesRook[i] = stof(line);
			i++;
			getline(myFile2, line, ' ');
			//cout << "Val 2: " << line << endl;
			verticesRook[i] = stof(line);
			i++;
			getline(myFile2, line, '\n');
			//cout << "Val 3: " << line << endl;
			verticesRook[i] = stof(line);
			i++;
		}
		myFile2.close();
	}
	else
	{
		cout << "Can't open the file";
	}
	// Read Vertex data from pawn.txt file //

	// Positions of pawns
	glm::vec3 rookPositions[] =
	{
		// Row 1
		glm::vec3(-3.0f, 0.5f, 4.0f), //X,Y,Z
		glm::vec3(4.0f, 0.5f, 4.0f),

		// Row 2
		glm::vec3(-3.0f, 0.5f, -3.0f),
		glm::vec3(4.0f, 0.5f, -3.0f),
	};

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VBA_Rook, VOA_Rook;
	glGenVertexArrays(1, &VOA_Rook);
	glGenBuffers(1, &VBA_Rook);

	// Bind the vertex array object
	glBindVertexArray(VOA_Rook);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Rook);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRook), verticesRook, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
	glEnableVertexAttribArray(2);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

#pragma region Pawn Textures

	//Chess Piece Pawn texture variables
	GLuint rookTextureW, rookTextureB;
	int widthRook, heightRook;

#pragma region Light Texture

	// Create and load White texture
	glGenTextures(1, &rookTextureW);
	glBindTexture(GL_TEXTURE_2D, rookTextureW);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* rookImageW = SOIL_load_image("res/images/Light square.png", &widthRook, &heightRook, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthRook, heightRook, 0, GL_RGBA, GL_UNSIGNED_BYTE, rookImageW);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(rookImageW);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma region Dark Texture 

	// Create and load Black texture
	glGenTextures(1, &rookTextureB);
	glBindTexture(GL_TEXTURE_2D, rookTextureB);

	// Set texture parameters 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* rookImageB = SOIL_load_image("res/images/Dark square 2.png", &widthRook, &heightRook, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthRook, heightRook, 0, GL_RGBA, GL_UNSIGNED_BYTE, rookImageB);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(rookImageB);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion


#pragma endregion

#pragma endregion

#pragma region Bishop

	//Build & Compile Shader Program for Pawn Pieces
	Shader ourShaderBishop("coreCB.vs", "coreCB.frag");

	// Vertex data for our pawn piece
	GLfloat verticesBishop[65538];

	// Read Vertex data from pawn.txt file //
	ifstream myFile3("res/3D models/OBJ Files/bishop.txt");
	i = 0;

	if (myFile3.is_open())
	{
		string line;

		while (!myFile3.eof())
		{
			getline(myFile3, line, ' ');
			//cout << "Val 1: " << line << endl;
			verticesBishop[i] = stof(line);
			i++;
			getline(myFile3, line, ' ');
			//cout << "Val 2: " << line << endl;
			verticesBishop[i] = stof(line);
			i++;
			getline(myFile3, line, '\n');
			//cout << "Val 3: " << line << endl;
			verticesBishop[i] = stof(line);
			i++;
		}
		myFile3.close();
	}
	else
	{
		cout << "Can't open the file";
	}
	// Read Vertex data from pawn.txt file //

	// Positions of pawns
	glm::vec3 bishopPositions[] =
	{
		// Row 1
		glm::vec3(-1.0f, 0.5f, 4.0f), //X,Y,Z
		glm::vec3(2.0f, 0.5f, 4.0f),

		// Row 2
		glm::vec3(-1.0f, 0.5f, -3.0f),
		glm::vec3(2.0f, 0.5f, -3.0f),

	};

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VBA_Bishop, VOA_Bishop;
	glGenVertexArrays(1, &VOA_Bishop);
	glGenBuffers(1, &VBA_Bishop);

	// Bind the vertex array object
	glBindVertexArray(VOA_Bishop);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Bishop);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBishop), verticesBishop, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
	glEnableVertexAttribArray(2);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

#pragma region Bishop Textures

	//Chess Piece Pawn texture variables
	GLuint bishopTextureW, bishopTextureB;
	int widthBishop, heightBishop;

#pragma region Light Texture

	// Create and load White texture
	glGenTextures(1, &bishopTextureW);
	glBindTexture(GL_TEXTURE_2D, bishopTextureW);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* bishopImageW = SOIL_load_image("res/images/Light square.png", &widthBishop, &heightBishop, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthBishop, heightBishop, 0, GL_RGBA, GL_UNSIGNED_BYTE, bishopImageW);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(bishopImageW);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma region Dark Texture 

	// Create and load Black texture
	glGenTextures(1, &bishopTextureB);
	glBindTexture(GL_TEXTURE_2D, bishopTextureB);

	// Set texture parameters 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* bishopImageB = SOIL_load_image("res/images/Dark square 2.png", &widthBishop, &heightBishop, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthBishop, heightBishop, 0, GL_RGBA, GL_UNSIGNED_BYTE, bishopImageB);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(bishopImageB);
	glBindTexture(GL_TEXTURE_2D, 0);
#pragma endregion



#pragma endregion





#pragma endregion

#pragma region Knight

	//Build & Compile Shader Program for Pawn Pieces
	Shader ourShaderKnight("coreCB.vs", "coreCB.frag");

	// Vertex data for our pawn piece
	GLfloat verticesKnight[60777];

	// Read Vertex data from pawn.txt file //
	ifstream myFile4("res/3D models/OBJ Files/knight.txt");
	i = 0;

	if (myFile4.is_open())
	{
		string line;

		while (!myFile4.eof())
		{
			getline(myFile4, line, ' ');
			//cout << "Val 1: " << line << endl;
			verticesKnight[i] = stof(line);
			i++;
			getline(myFile4, line, ' ');
			//cout << "Val 2: " << line << endl;
			verticesKnight[i] = stof(line);
			i++;
			getline(myFile4, line, '\n');
			//cout << "Val 3: " << line << endl;
			verticesKnight[i] = stof(line);
			i++;
		}
		myFile4.close();
	}
	else
	{
		cout << "Can't open the file";
	}
	// Read Vertex data from pawn.txt file //

	// Positions of pawns
	glm::vec3 knightPositions[] =
	{
		// Row 1
		glm::vec3(-2.0f, 0.5f, 4.0f), //X,Y,Z
		glm::vec3(3.0f, 0.5f, 4.0f),

		// Row 2
		glm::vec3(-2.0f, 0.5f, -3.0f),
		glm::vec3(3.0f, 0.5f, -3.0f),
	};

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VBA_Knight, VOA_Knight;
	glGenVertexArrays(1, &VOA_Knight);
	glGenBuffers(1, &VBA_Knight);

	// Bind the vertex array object
	glBindVertexArray(VOA_Knight);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Knight);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesKnight), verticesKnight, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
	glEnableVertexAttribArray(2);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

#pragma region Knight Textures

	//Chess Piece Pawn texture variables
	GLuint knightextureW, knightTextureB;
	int widthKnight, heightKnight;

#pragma region Light Texture

	// Create and load White texture
	glGenTextures(1, &knightextureW);
	glBindTexture(GL_TEXTURE_2D, knightextureW);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* knightImageW = SOIL_load_image("res/images/Light square.png", &widthKnight, &heightKnight, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthKnight, heightKnight, 0, GL_RGBA, GL_UNSIGNED_BYTE, knightImageW);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(knightImageW);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma region Dark Texture 

	// Create and load Black texture
	glGenTextures(1, &knightTextureB);
	glBindTexture(GL_TEXTURE_2D, knightTextureB);

	// Set texture parameters 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* knightImageB = SOIL_load_image("res/images/Dark square 2.png", &widthKnight, &heightKnight, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthKnight, heightKnight, 0, GL_RGBA, GL_UNSIGNED_BYTE, knightImageB);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(knightImageB);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma endregion







#pragma endregion

#pragma region Queen

	//Build & Compile Shader Program for Pawn Pieces
	Shader ourShaderQueen("coreCB.vs", "coreCB.frag");

	// Vertex data for our pawn piece
	GLfloat verticesQueen[39600];

	// Read Vertex data from pawn.txt file //
	ifstream myFile5("res/3D models/OBJ Files/queen.txt");
	i = 0;

	if (myFile5.is_open())
	{
		string line;

		while (!myFile5.eof())
		{
			getline(myFile5, line, ' ');
			//cout << "Val 1: " << line << endl;
			verticesQueen[i] = stof(line);
			i++;
			getline(myFile5, line, ' ');
			//cout << "Val 2: " << line << endl;
			verticesQueen[i] = stof(line);
			i++;
			getline(myFile5, line, '\n');
			//cout << "Val 3: " << line << endl;
			verticesQueen[i] = stof(line);
			i++;
		}
		myFile5.close();
	}
	else
	{
		cout << "Can't open the file";
	}
	// Read Vertex data from pawn.txt file //

	// Positions of pawns
	glm::vec3 queenPositions[] =
	{
		// Row 1
		glm::vec3(0.0f, 0.5f, 4.0f),

		// Row 2
		glm::vec3(0.0f, 0.5f, -3.0f),
	};

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VBA_Queen, VOA_Queen;
	glGenVertexArrays(1, &VOA_Queen);
	glGenBuffers(1, &VBA_Queen);

	// Bind the vertex array object
	glBindVertexArray(VOA_Queen);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Queen);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesQueen), verticesQueen, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
	glEnableVertexAttribArray(2);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

#pragma region Queen Textures

	//Chess Piece Pawn texture variables
	GLuint queentextureW, queenTextureB;
	int widthQueen, heightQueen;

#pragma region Light Texture

	// Create and load White texture
	glGenTextures(1, &queentextureW);
	glBindTexture(GL_TEXTURE_2D, queentextureW);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* queenImageW = SOIL_load_image("res/images/Light square.png", &widthQueen, &heightQueen, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthQueen, heightQueen, 0, GL_RGBA, GL_UNSIGNED_BYTE, queenImageW);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(queenImageW);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma region Dark Texture 

	// Create and load Black texture
	glGenTextures(1, &queenTextureB);
	glBindTexture(GL_TEXTURE_2D, queenTextureB);

	// Set texture parameters 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* queenImageB = SOIL_load_image("res/images/Dark square 2.png", &widthQueen, &heightQueen, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthQueen, heightQueen, 0, GL_RGBA, GL_UNSIGNED_BYTE, queenImageB);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(queenImageB);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion



#pragma endregion





#pragma endregion

//#pragma region King
//	//Build & Compile Shader Program for Pawn Pieces
//	Shader ourShaderKing("coreCB.vs", "coreCB.frag");
//
//	// Vertex data for our pawn piece
//	GLfloat verticesKing[22860];
//
//	// Read Vertex data from pawn.txt file //
//	ifstream myFile6("res/3D models/OBJ Files/king.txt");
//	i = 0;
//
//	if (myFile6.is_open())
//	{
//		string line;
//
//		while (!myFile6.eof())
//		{
//			getline(myFile6, line, ' ');
//			//cout << "Val 1: " << line << endl;
//			verticesKing[i] = stof(line);
//			i++;
//			getline(myFile6, line, ' ');
//			//cout << "Val 2: " << line << endl;
//			verticesKing[i] = stof(line);
//			i++;
//			getline(myFile6, line, '\n');
//			//cout << "Val 3: " << line << endl;
//			verticesKing[i] = stof(line);
//			i++;
//		}
//		myFile6.close();
//	}
//	else
//	{
//		cout << "Can't open the file";
//	}
//	// Read Vertex data from pawn.txt file //
//
//	// Positions of pawns
//	glm::vec3 kingPositions[] =
//	{
//		// Row 1
//		glm::vec3(1.0f, 0.5f, 4.0f),
//
//		// Row 2
//		glm::vec3(1.0f, 0.5f, -3.0f),
//	};
//
//	// Generate the vertex arrays and vertex buffers and save them into variables
//	GLuint VBA_King, VOA_King;
//	glGenVertexArrays(1, &VOA_King);
//	glGenBuffers(1, &VBA_King);
//
//	// Bind the vertex array object
//	glBindVertexArray(VOA_King);
//
//	// Bind and set the vertex buffers
//	glBindBuffer(GL_ARRAY_BUFFER, VBA_King);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesKing), verticesKing, GL_STATIC_DRAW);
//
//	// Create the vertex pointer and enable the vertex array
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
//	glEnableVertexAttribArray(0);
//
//	// Texture coordinate attribute
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
//	glEnableVertexAttribArray(2);
//
//	// Unbind the vertex array to prevent strange bugs
//	glBindVertexArray(0);
//
//#pragma region King Textures
//
//	//Chess Piece Pawn texture variables
//	GLuint kingtextureW, kingTextureB;
//	int widthKing, heightKing;
//
//#pragma region Light Texture
//
//	// Create and load White texture
//	glGenTextures(1, &kingtextureW);
//	glBindTexture(GL_TEXTURE_2D, kingtextureW);
//
//	// Set texture parameters
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//
//	// Set texture filtering
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	// Actual texture loading code
//	unsigned char* kingImageW = SOIL_load_image("res/images/Light square.png", &widthKing, &heightKing, 0, SOIL_LOAD_RGBA);
//
//	// Specify 2D texture image
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthKing, heightKing, 0, GL_RGBA, GL_UNSIGNED_BYTE, kingImageW);
//
//	// Generate mipmaps
//	glGenerateMipmap(GL_TEXTURE_2D);
//	SOIL_free_image_data(kingImageW);
//	glBindTexture(GL_TEXTURE_2D, 0);
//
//#pragma endregion
//
//#pragma region Dark Texture 
//
//	// Create and load Black texture
//	glGenTextures(1, &kingTextureB);
//	glBindTexture(GL_TEXTURE_2D, kingTextureB);
//
//	// Set texture parameters 
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//
//	// Set texture filtering
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	// Actual texture loading code
//	unsigned char* kingImageB = SOIL_load_image("res/images/Dark square 2.png", &widthKing, &heightKing, 0, SOIL_LOAD_RGBA);
//
//	// Specify 2D texture image
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthKing, heightKing, 0, GL_RGBA, GL_UNSIGNED_BYTE, kingImageB);
//
//	// Generate mipmaps
//	glGenerateMipmap(GL_TEXTURE_2D);
//	SOIL_free_image_data(kingImageB);
//	glBindTexture(GL_TEXTURE_2D, 0);
//
//#pragma endregion
//
//
//#pragma endregion
//
//
//
//
//
//#pragma endregion

#pragma endregion

#pragma region Build Custom Meshes
#pragma region skull
	//Build & Compile Shader Program for Pawn Pieces
	Shader ourShaderSkull("coreCB.vs", "coreCB.frag");

	// Vertex data for our pawn piece
	GLfloat verticesSkull[20916]; //83664

	// Read Vertex data from pawn.txt file //
	ifstream myFile7("res/3D models/OBJ Files/Skull.txt");
	i = 0;

	if (myFile7.is_open())
	{
		string line;

		while (!myFile7.eof())
		{
			getline(myFile7, line, ' ');
			//cout << "Val 1: " << line << endl;
			verticesSkull[i] = stof(line);
			i++;
			getline(myFile7, line, ' ');
			//cout << "Val 2: " << line << endl;
			verticesSkull[i] = stof(line);
			i++;
			getline(myFile7, line, '\n');
			//cout << "Val 3: " << line << endl;
			verticesSkull[i] = stof(line);
			i++;
		}
		myFile7.close();
	}
	else
	{
		cout << "Can't open the file";
	}
	// Read Vertex data from pawn.txt file //

	// Positions of pawns
	glm::vec3 skullPositions[] =
	{
		// Row 1
		glm::vec3(1.0f, 0.5f, 4.0f),

		// Row 2
		glm::vec3(1.0f, 0.5f, -3.0f),
	};

	// Generate the vertex arrays and vertex buffers and save them into variables
	GLuint VBA_Skull, VOA_Skull;
	glGenVertexArrays(1, &VOA_Skull);
	glGenBuffers(1, &VBA_Skull);

	// Bind the vertex array object
	glBindVertexArray(VOA_Skull);

	// Bind and set the vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBA_Skull);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesSkull), verticesSkull, GL_STATIC_DRAW);

	// Create the vertex pointer and enable the vertex array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
	glEnableVertexAttribArray(0);

	// Texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
	glEnableVertexAttribArray(2);

	// Unbind the vertex array to prevent strange bugs
	glBindVertexArray(0);

#pragma region Skull Textures

	//Chess Piece Pawn texture variables
	GLuint skullTexture;
	int widthSkull, heightSkull;

	// Create and load texture
	glGenTextures(1, &skullTexture);
	glBindTexture(GL_TEXTURE_2D, skullTexture);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Actual texture loading code
	unsigned char* skullTmageW = SOIL_load_image("res/images/Light square.png", &widthSkull, &heightSkull, 0, SOIL_LOAD_RGBA);

	// Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthSkull, heightSkull, 0, GL_RGBA, GL_UNSIGNED_BYTE, skullTmageW);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(skullTmageW);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion




#pragma endregion

//#pragma region Palm Tree
//	//Build & Compile Shader Program for Pawn Pieces
//	Shader ourShaderPalm("coreCB.vs", "coreCB.frag");
//
//	// Vertex data for our pawn piece
//	GLfloat verticesPalm[20916]; //20916
//
//	// Read Vertex data from pawn.txt file //
//	ifstream myFile8("res/3D models/OBJ Files/king.txt");
//	i = 0;
//
//	if (myFile8.is_open())
//	{
//		string line;
//
//		while (!myFile8.eof())
//		{
//			getline(myFile8, line, ' ');
//			//cout << "Val 1: " << line << endl;
//			verticesPalm[i] = stof(line);
//			i++;
//			getline(myFile8, line, ' ');
//			//cout << "Val 2: " << line << endl;
//			verticesPalm[i] = stof(line);
//			i++;
//			getline(myFile8, line, '\n');
//			//cout << "Val 3: " << line << endl;
//			verticesPalm[i] = stof(line);
//			i++;
//		}
//		myFile8.close();
//	}
//	else
//	{
//		cout << "Can't open the file";
//	}
//	// Read Vertex data from pawn.txt file //
//
//	// Positions of pawns
//	glm::vec3 palmPositions[] =
//	{
//		// Row 1
//		glm::vec3(1.0f, 0.5f, 4.0f),
//
//		// Row 2
//		glm::vec3(1.0f, 0.5f, -3.0f),
//	};
//
//	// Generate the vertex arrays and vertex buffers and save them into variables
//	GLuint VBA_Palm, VOA_Palm;
//	glGenVertexArrays(1, &VOA_Palm);
//	glGenBuffers(1, &VBA_Palm);
//
//	// Bind the vertex array object
//	glBindVertexArray(VOA_Palm);
//
//	// Bind and set the vertex buffers
//	glBindBuffer(GL_ARRAY_BUFFER, VBA_Palm);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPalm), verticesPalm, GL_STATIC_DRAW);
//
//	// Create the vertex pointer and enable the vertex array
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (GLvoid*)0); //Position
//	glEnableVertexAttribArray(0);
//
//	// Texture coordinate attribute
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //Texture
//	glEnableVertexAttribArray(2);
//
//	// Unbind the vertex array to prevent strange bugs
//	glBindVertexArray(0);
//#pragma endregion 
#pragma endregion

	//Game LOOP
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Checks for events and calls corresponding response
		glfwPollEvents();

		//Render and clear the colour buffer
		glClearColor(0.4f, 0.6f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region Draw chessBoard
		// Activate Shader
		chessboardShader.Use();

		// Create Projection Matrix
		glm::mat4 projection_Board(1.0f);
		//Perspective view 
		projection_Board = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);

		// Create camera transformation 
		glm::mat4 view_Board(1.0f);
		view_Board = camera.GetViewMatrix();

		// Get the uniform locations for our matrices
		GLint modelLoc_Board = glGetUniformLocation(chessboardShader.Program, "model");
		GLint viewLoc_Board = glGetUniformLocation(chessboardShader.Program, "view");
		GLint projLoc_Board = glGetUniformLocation(chessboardShader.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Board, 1, GL_FALSE, glm::value_ptr(view_Board));
		glUniformMatrix4fv(projLoc_Board, 1, GL_FALSE, glm::value_ptr(projection_Board));


		// Draw container
		glBindVertexArray(VOA_Board);

		for (GLuint i = 0; i < 8; i++)
		{
			for (GLuint j = 0; j < 8; j++)
			{
				if ((i + j) % 2 == 0)
				{
					// Activate White texture
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textureBlack);
					glUniform1i(glGetUniformLocation(chessboardShader.Program, "faceTexture"), 0);
				}
				else
				{
					// Activate Black texture
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textureWhite);
					glUniform1i(glGetUniformLocation(chessboardShader.Program, "faceTexture"), 0);
				}

				// Calculate the model matrix for each object and pass it to the shader before drawing
				glm::mat4 model_Board(1.0f);
				glm::vec3 cubePos(cubePositions[i].x, randY[i][j], cubePositions[i].z - j);
				model_Board = glm::translate(model_Board, cubePos);
				GLfloat angle = 0.0f;
				model_Board = glm::rotate(model_Board, angle, glm::vec3(1.0f, 0.0f, 0.0f));
				glUniformMatrix4fv(modelLoc_Board, 1, GL_FALSE, glm::value_ptr(model_Board));

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		for (GLuint i = 0; i < 2; i++)
		{
			for (GLuint j = 0; j < 8; j++)
			{
				// Activate White texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureGrey);
				glUniform1i(glGetUniformLocation(chessboardShader.Program, "faceTexture"), 0);

				// Calculate the model matrix for each object and pass it to the shader before drawing
				glm::mat4 model_Board(1.0f);
				glm::vec3 cubePos(borderPositions[i].z - j, borderPositions[i].y, borderPositions[i].x);
				model_Board = glm::translate(model_Board, cubePos);
				GLfloat angle = 0.0f;
				glm::vec3 scale(1, 1, 0.5f);
				model_Board = glm::scale(model_Board, scale);
				model_Board = glm::rotate(model_Board, angle, glm::vec3(1.0f, 0.0f, 0.0f));
				glUniformMatrix4fv(modelLoc_Board, 1, GL_FALSE, glm::value_ptr(model_Board));

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
		for (GLuint i = 0; i < 2; i++)
		{
			for (GLuint j = 0; j < 8; j++)
			{
				// Activate White texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textureGrey);
				glUniform1i(glGetUniformLocation(chessboardShader.Program, "faceTexture"), 0);

				// Calculate the model matrix for each object and pass it to the shader before drawing
				glm::mat4 model_Board(1.0f);
				glm::vec3 cubePos(borderPositions[i].x, borderPositions[i].y, borderPositions[i].z - j);
				model_Board = glm::translate(model_Board, cubePos);
				GLfloat angle = 0.0f;
				glm::vec3 scale(0.5f, 1, 1);
				model_Board = glm::scale(model_Board, scale);
				model_Board = glm::rotate(model_Board, angle, glm::vec3(1.0f, 0.0f, 0.0f));
				glUniformMatrix4fv(modelLoc_Board, 1, GL_FALSE, glm::value_ptr(model_Board));

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		for (int i = 2; i < 6; i++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureGrey);
			glUniform1i(glGetUniformLocation(chessboardShader.Program, "faceTexture"), 0);

			glm::mat4 model_Board(1.0f);
			glm::vec3 cubePos(borderPositions[i].x, borderPositions[i].y, borderPositions[i].z);
			model_Board = glm::translate(model_Board, cubePos);
			GLfloat angle = 0.0f;

			glm::vec3 scale(0.5f, 1, 0.5f);
			model_Board = glm::scale(model_Board, scale);
			model_Board = glm::rotate(model_Board, angle, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(modelLoc_Board, 1, GL_FALSE, glm::value_ptr(model_Board));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
#pragma endregion

#pragma region Draw Chess Pieces

#pragma region Draw Pawn

		// Activate Shader
		ourShaderPawn.Use();

		// Create Projection Matrix (moved into while loop in order to update zoom)
		glm::mat4 projection_Pawn(1.0f);
		//Perspective view
		projection_Pawn = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);
		// Create camera transformation
		glm::mat4 view_Pawn(1.0f);
		view_Pawn = camera.GetViewMatrix();

		// Get the uniform locations for our matrices
		GLint modelLoc_Pawn = glGetUniformLocation(ourShaderPawn.Program, "model");
		GLint viewLoc_Pawn = glGetUniformLocation(ourShaderPawn.Program, "view");
		GLint projLoc_Pawn = glGetUniformLocation(ourShaderPawn.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Pawn, 1, GL_FALSE, glm::value_ptr(view_Pawn));
		glUniformMatrix4fv(projLoc_Pawn, 1, GL_FALSE, glm::value_ptr(projection_Pawn));

		// Draw container
		glBindVertexArray(VOA_Pawn);

		for (GLuint i = 0; i < 16; i++)
		{
			if (i <= 7)
			{
				// Activate White Texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, pawnTextureW);
				glUniform1i(glGetUniformLocation(ourShaderPawn.Program, "faceTexture"), 0);
			}
			else
			{
				// Activate Black Texture 
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, pawnTextureB);
				glUniform1i(glGetUniformLocation(ourShaderPawn.Program, "faceTexture"), 0);
			}

			// Calculate the model matrix for each object and pass it to the shader before drawing
			glm::mat4 model_Pawn(1.0f);

			glm::vec3 newPos = AnimatePosition(pawnPositions[i]);
			model_Pawn = glm::translate(model_Pawn, newPos); // Original code 
			GLfloat angle = AnimateCPRotation(); //for animation

			// Handles Piece Rotation
			model_Pawn = glm::rotate(model_Pawn, angle, glm::vec3(1.0f, 0.0f, 0.0f));

			glUniformMatrix4fv(modelLoc_Pawn, 1, GL_FALSE, glm::value_ptr(model_Pawn));

			glDrawArrays(GL_TRIANGLES, 0, 8088); //number of lines times by 2

		}
#pragma endregion

#pragma region Draw Rook

		// Activate Shader
		ourShaderRook.Use();



		/*GLint objectColorLoc = glGetUniformLocation(LightingShader.Program, "objectColor");
		GLint lightLoc = glGetUniformLocation(LightingShader.Program, "lightColor");

		glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
		glUniform3f(lightLoc, 1.0f, 0.5f, 1.0f);
		LightPos = glm::vec3(1.0f, 1.3f, 3.0f);*/

		// Create Projection Matrix (moved into while loop in order to update zoom)
		glm::mat4 projection_Rook(1.0f);
		//Perspective view
		projection_Rook = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);
		// Create camera transformation
		glm::mat4 view_Rook(1.0f);
		view_Rook = camera.GetViewMatrix();

		// Get the uniform locations for our matrices
		GLint modelLoc_Rook = glGetUniformLocation(ourShaderRook.Program, "model");
		GLint viewLoc_Rook = glGetUniformLocation(ourShaderRook.Program, "view");
		GLint projLoc_Rook = glGetUniformLocation(ourShaderRook.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Rook, 1, GL_FALSE, glm::value_ptr(view_Rook));
		glUniformMatrix4fv(projLoc_Rook, 1, GL_FALSE, glm::value_ptr(projection_Rook));
		//glUniform3f(glGetUniformLocation(LightingShader.Program, "lightPos"), LightPos.x, LightPos.y, LightPos.z);

		// Draw container
		glBindVertexArray(VOA_Rook);

		for (GLuint i = 0; i < 4; i++)
		{
			if (i <= 1)
			{
				// Activate White Texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rookTextureW);
				glUniform1i(glGetUniformLocation(ourShaderRook.Program, "faceTexture"), 0);
			}
			else
			{
				// Activate Black Texture 
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rookTextureB);
				glUniform1i(glGetUniformLocation(ourShaderRook.Program, "faceTexture"), 0);
			}

			// Calculate the model matrix for each object and pass it to the shader before drawing
			glm::mat4 model_Rook(1.0f);
			model_Rook = glm::translate(model_Rook, rookPositions[i]); // Original code 
			GLfloat angle = 0.0f; // Original code
			model_Rook = glm::rotate(model_Rook, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Original code

			glUniformMatrix4fv(modelLoc_Rook, 1, GL_FALSE, glm::value_ptr(model_Rook));

			glDrawArrays(GL_TRIANGLES, 0, 7620); //number of lines times by 2

		}
		
#pragma endregion
		
#pragma region Draw Bishop

		// Activate Shader
		ourShaderBishop.Use();

		// Create Projection Matrix (moved into while loop in order to update zoom)
		glm::mat4 projection_Bishop(1.0f);
		//Perspective view
		projection_Bishop = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);

		// Create camera transformation
		glm::mat4 view_Bishop(1.0f);
		view_Bishop = camera.GetViewMatrix();

		// Get the uniform locations for our matrices
		GLint modelLoc_Bishop = glGetUniformLocation(ourShaderBishop.Program, "model");
		GLint viewLoc_Bishop = glGetUniformLocation(ourShaderBishop.Program, "view");
		GLint projLoc_Bishop = glGetUniformLocation(ourShaderBishop.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Bishop, 1, GL_FALSE, glm::value_ptr(view_Bishop));
		glUniformMatrix4fv(projLoc_Bishop, 1, GL_FALSE, glm::value_ptr(projection_Bishop));

		// Draw container
		glBindVertexArray(VOA_Bishop);

		for (GLuint i = 0; i < 4; i++)
		{
			if (i <= 1)
			{
				// Activate White Texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, bishopTextureW);
				glUniform1i(glGetUniformLocation(ourShaderBishop.Program, "faceTexture"), 0);
			}
			else
			{
				// Activate Black Texture 
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, bishopTextureB);
				glUniform1i(glGetUniformLocation(ourShaderBishop.Program, "faceTexture"), 0);
			}

			// Calculate the model matrix for each object and pass it to the shader before drawing
			glm::mat4 model_Bishop(1.0f);
			model_Bishop = glm::translate(model_Bishop, bishopPositions[i]); // Original code 
			GLfloat angle = 0.0f; // Original code
			model_Bishop = glm::rotate(model_Bishop, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Original code

			glUniformMatrix4fv(modelLoc_Bishop, 1, GL_FALSE, glm::value_ptr(model_Bishop));

			glDrawArrays(GL_TRIANGLES, 0, 21864); //number of lines times by 2

		}
#pragma endregion

#pragma region Draw Knight
		
		// Activate Shader
		ourShaderKnight.Use();

		// Create Projection Matrix (moved into while loop in order to update zoom)
		glm::mat4 projection_Knight(1.0f);
		//Perspective view
		projection_Knight = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);

		// Create camera transformation
		glm::mat4 view_Knight(1.0f);
		view_Knight = camera.GetViewMatrix();

		// Get the uniform locations for our matrices
		GLint modelLoc_Knight = glGetUniformLocation(ourShaderKnight.Program, "model");
		GLint viewLoc_Knight = glGetUniformLocation(ourShaderKnight.Program, "view");
		GLint projLoc_Knight = glGetUniformLocation(ourShaderKnight.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Knight, 1, GL_FALSE, glm::value_ptr(view_Knight));
		glUniformMatrix4fv(projLoc_Knight, 1, GL_FALSE, glm::value_ptr(projection_Knight));

		// Draw container
		glBindVertexArray(VOA_Knight);

		for (GLuint i = 0; i < 4; i++)
		{
			if (i <= 1)
			{
				// Activate White Texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, knightextureW);
				glUniform1i(glGetUniformLocation(ourShaderKnight.Program, "faceTexture"), 0);
			}
			else
			{
				// Activate Black Texture 
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, knightTextureB);
				glUniform1i(glGetUniformLocation(ourShaderKnight.Program, "faceTexture"), 0);
			}

			// Calculate the model matrix for each object and pass it to the shader before drawing
			glm::mat4 model_Knight(1.0f);

			glm::vec3 newPos = AnimatePosition(knightPositions[i]);
			model_Knight = glm::translate(model_Knight, newPos); // Original code 
			GLfloat angleK = AnimateCPRotation(); //for animation

			// Handles Piece Rotation
			model_Knight = glm::rotate(model_Knight, angleK, glm::vec3(0.0f, 1.0f, 0.0f));

			glUniformMatrix4fv(modelLoc_Knight, 1, GL_FALSE, glm::value_ptr(model_Knight));

			glDrawArrays(GL_TRIANGLES, 0, 20259); //number of lines times by 2

		}
#pragma endregion

#pragma region Draw Queen

		// Activate Shader
		ourShaderQueen.Use();

		// Create Projection Matrix (moved into while loop in order to update zoom)
		glm::mat4 projection_Queen(1.0f);
		//Perspective view
		projection_Queen = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);

		// Create camera transformation
		glm::mat4 view_Queen(1.0f);
		view_Queen = camera.GetViewMatrix();

		// Get the uniform locations for our matrices
		GLint modelLoc_Queen = glGetUniformLocation(ourShaderQueen.Program, "model");
		GLint viewLoc_Queen = glGetUniformLocation(ourShaderQueen.Program, "view");
		GLint projLoc_Queen = glGetUniformLocation(ourShaderQueen.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Queen, 1, GL_FALSE, glm::value_ptr(view_Queen));
		glUniformMatrix4fv(projLoc_Queen, 1, GL_FALSE, glm::value_ptr(projection_Queen));

		// Draw container
		glBindVertexArray(VOA_Queen);

		for (GLuint i = 0; i < 2; i++)
		{
			if (i <= 0)
			{
				// Activate White Texture
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, queentextureW);
				glUniform1i(glGetUniformLocation(ourShaderQueen.Program, "faceTexture"), 0);
			}
			else
			{
				// Activate Black Texture 
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, queenTextureB);
				glUniform1i(glGetUniformLocation(ourShaderQueen.Program, "faceTexture"), 0);
			}

			// Calculate the model matrix for each object and pass it to the shader before drawing
			glm::mat4 model_Queen(1.0f);
			model_Queen = glm::translate(model_Queen, queenPositions[i]); // Original code 
			GLfloat angle = 0.0f; // Original code
			model_Queen = glm::rotate(model_Queen, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Original code

			glUniformMatrix4fv(modelLoc_Queen, 1, GL_FALSE, glm::value_ptr(model_Queen));

			glDrawArrays(GL_TRIANGLES, 0, 13200); //number of lines times by 2

		}
#pragma endregion

//#pragma region Draw King
//
//		// Activate Shader
//		ourShaderKing.Use();
//
//		// Create Projection Matrix (moved into while loop in order to update zoom)
//		glm::mat4 projection_King(1.0f);
//		//Perspective view
//		projection_King = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);
//
//		// Create camera transformation
//		glm::mat4 view_King(1.0f);
//		view_King = camera.GetViewMatrix();
//
//		// Get the uniform locations for our matrices
//		GLint modelLoc_King = glGetUniformLocation(ourShaderKing.Program, "model");
//		GLint viewLoc_King = glGetUniformLocation(ourShaderKing.Program, "view");
//		GLint projLoc_King = glGetUniformLocation(ourShaderKing.Program, "projection");
//
//		// Pass locations to shaders
//		glUniformMatrix4fv(viewLoc_King, 1, GL_FALSE, glm::value_ptr(view_King));
//		glUniformMatrix4fv(projLoc_King, 1, GL_FALSE, glm::value_ptr(projection_King));
//
//		// Draw container
//		glBindVertexArray(VOA_King);
//
//		for (GLuint i = 0; i < 2; i++)
//		{
//			if (i <= 0)
//			{
//				// Activate White Texture
//				glActiveTexture(GL_TEXTURE0);
//				glBindTexture(GL_TEXTURE_2D, kingtextureW);
//				glUniform1i(glGetUniformLocation(ourShaderKing.Program, "faceTexture"), 0);
//			}
//			else
//			{
//				// Activate Black Texture 
//				glActiveTexture(GL_TEXTURE0);
//				glBindTexture(GL_TEXTURE_2D, kingTextureB);
//				glUniform1i(glGetUniformLocation(ourShaderKing.Program, "faceTexture"), 0);
//			}
//
//			// Calculate the model matrix for each object and pass it to the shader before drawing
//			glm::mat4 model_King(1.0f);
//			model_King = glm::translate(model_King, kingPositions[i]); // Original code 
//			GLfloat angle = 0.0f; // Original code
//			model_King = glm::rotate(model_King, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Original code
//
//			glUniformMatrix4fv(modelLoc_King, 1, GL_FALSE, glm::value_ptr(model_King));
//
//			glDrawArrays(GL_TRIANGLES, 0, 7620); //number of lines times by 2
//
//		}
//#pragma endregion
		
#pragma endregion
		
#pragma region Draw Custom Models
#pragma region Draw Skull 
		// Activate Shader
		ourShaderSkull.Use();

		// Create Projection Matrix (moved into while loop in order to update zoom)
		glm::mat4 projection_Skull(1.0f);
		//Perspective view
		projection_Skull = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);

		// Create camera transformation
		glm::mat4 view_Skull(1.0f);
		view_Skull = camera.GetViewMatrix();

		// Get the uniform locations for our matrices
		GLint modelLoc_Skull = glGetUniformLocation(ourShaderSkull.Program, "model");
		GLint viewLoc_Skull = glGetUniformLocation(ourShaderSkull.Program, "view");
		GLint projLoc_Skull = glGetUniformLocation(ourShaderSkull.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Skull, 1, GL_FALSE, glm::value_ptr(view_Skull));
		glUniformMatrix4fv(projLoc_Skull, 1, GL_FALSE, glm::value_ptr(projection_Skull));

		// Draw container
		glBindVertexArray(VOA_Skull);

		for (GLuint i = 0; i < 2; i++)
		{

			// Activate White Texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, skullTexture);
			glUniform1i(glGetUniformLocation(ourShaderSkull.Program, "faceTexture"), 0);

			// Calculate the model matrix for each object and pass it to the shader before drawing
			glm::mat4 model_Skull(1.0f);
			model_Skull= glm::translate(model_Skull, skullPositions[i]); // Original code 
			GLfloat angle = 0.0f; // Original code
			model_Skull = glm::rotate(model_Skull, angle, glm::vec3(1.0f, 0.0f, 0.0f)); // Original code

			glUniformMatrix4fv(modelLoc_Skull, 1, GL_FALSE, glm::value_ptr(model_Skull));

			glDrawArrays(GL_TRIANGLES, 0, 6972); //number of lines times 27888

		}
#pragma endregion
#pragma endregion
		
//Terrain Generation
#pragma region Height Map

		// Activate Shader
		shaderHM.Use();

		// view/projection transformations
		glm::mat4 projectionHM = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);
		glm::mat4 viewHM = camera.GetViewMatrix();
		GLint projLocHM = glGetUniformLocation(shaderHM.Program, "projection");
		GLint viewLocHM = glGetUniformLocation(shaderHM.Program, "view");

		glUniformMatrix4fv(viewLocHM, 1, GL_FALSE, glm::value_ptr(viewHM));
		glUniformMatrix4fv(projLocHM, 1, GL_FALSE, glm::value_ptr(projectionHM));

		// world transformation
		glm::mat4 modelHM = glm::mat4(1.0f);
		GLint modelLocHM = glGetUniformLocation(shaderHM.Program, "model");
		glUniformMatrix4fv(modelLocHM, 1, GL_FALSE, glm::value_ptr(modelHM));

		// Draw container
		glBindVertexArray(VOA_HM);

		for (int strip = 0; strip < numStrips; strip++)
		{

			//For Height Map Texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureHM);
			glUniform1i(glGetUniformLocation(shaderHM.Program, "ourHM_texture"), 0);
			//For Height Map Texture

			glDrawElements(GL_TRIANGLE_STRIP,
				numTrisPerStrip + 2,
				GL_UNSIGNED_INT,
				(void*)(sizeof(GLuint) * (numTrisPerStrip + 2) * strip));
		}

		glBindVertexArray(0); // Unbinding

#pragma endregion

#pragma region SkyBox Creation
		// draw skybox as last
				// change depth function so depth test passes when values are equal to depth buffer's content
		glDepthFunc(GL_LEQUAL);

		skyboxShader.Use();
		// Create Projection Matrix
		glm::mat4 projection_Skybox = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

		// Create camera transformation
		// remove translation from the view matrix
		glm::mat4 view_Skybox = glm::mat4(glm::mat3(camera.GetViewMatrix()));

		// Get the uniform locations for our matrices
		GLint viewLoc_Skybox = glGetUniformLocation(skyboxShader.Program, "view");
		GLint projLoc_Skybox = glGetUniformLocation(skyboxShader.Program, "projection");

		// Pass locations to shaders
		glUniformMatrix4fv(viewLoc_Skybox, 1, GL_FALSE, glm::value_ptr(view_Skybox));
		glUniformMatrix4fv(projLoc_Skybox, 1, GL_FALSE, glm::value_ptr(projection_Skybox));
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default
#pragma endregion
		
		
		//DRAW OPENGL WINDOW/VIEWPORT
		glfwSwapBuffers(window);

	}

	// Terminate GLFW and clear recources from GLFW
	glfwTerminate();


	return EXIT_SUCCESS;
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// For Camera  Enable Camera Switch Camera
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		if (camLocked == true)
		{
			camLocked = false;
		}
		else
		{
			camLocked = true;
		}
	}

	//Cycle Camera Left
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		camLocked = true;
		camera.CycleCamera("Left");
	}

	//Cycle Camera Right
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		camLocked = true;
		camera.CycleCamera("Right");
	}

	// for animations
	// Start and Stop the Chess Piece Animations
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		if (animate == true)
		{
			animate = false;
		}
		else
		{
			animate = true;
		}
	}


	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}

		ProcessInput(window);
	}
}

// GLFW: whenever the mouse moves, this callback is called
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (camLocked == false)
	{
		if (firstMouse)
		{
			lastX = xPos;
			lastY = yPos;
			firstMouse = false;
		}

		GLfloat xOffset = xPos - lastX;
		GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

		lastX = xPos;
		lastY = yPos;
		
		camera.ProcessMouseMovement(xOffset, yOffset);
	}

}

// GLFW: whenever the mouse scroll wheel scrolls, this callback is called
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	camera.ProcessMouseScroll(yOffset);
}

// Moves/alters the camera positions based on user input
// WASD and Arrow keys
void ProcessInput(GLFWwindow* window)
{
	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	
}

GLfloat AnimateCPRotation()
{
	if (animate)
	{
		return (GLfloat)glfwGetTime() * 1.0f;
		// Animate Chess Piece
	}

	else
	{
		return 0.0f;
		// Just set rotation to 0 is original rotation
	}

}

GLfloat AnimateCPSlide()
{
	if (animate)
	{
		return (GLfloat)glfwGetTime() * 1.0f;
		// Animate Chess Piece
	}

	else
	{
		return 0.0f;
		// Just set rotation to 0 is original rotation
	}

}

glm::vec3 AnimatePosition(glm::vec3 pos)
{
	if (animate)
	{
		// Animate Chess Piece Position

		return glm::vec3(pos.x, pos.y + 1, pos.z);

	}

	else

	{

		// Just set position to 0 is original rotation
		return pos;
	}
}

glm::vec3 AnimatePosition3(glm::vec3 pos)
{
	if (animate)
	{
		// Animate Chess Piece Position
		if (pos.y >= 3)
		{
			return glm::vec3(pos.x, pos.y - 0.1f, pos.z);
		}
		else if (pos.y <= 0)
		{
			return glm::vec3(pos.x, pos.y + 0.1f, pos.z);
		}


	}

	else

	{

		// Just set position to 0 is original rotation
		return pos;
	}
}

GLuint CreateSkyboxTexture(GLuint texture, vector<std::string> faces, int width, int height) // Method for Skybox Texture
{
	int widthTexture = 0, heightTexture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	int nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* textureImg = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (textureImg)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImg);
			SOIL_free_image_data(textureImg);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			SOIL_free_image_data(textureImg);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texture;
}
