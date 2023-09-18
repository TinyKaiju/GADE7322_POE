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

const GLint WIDTH = 1920, HEIGHT = 1080;
int SCREEN_WIDTH, SCREEN_HEIGHT; // Replace all screenW & screenH with these

// MAIN FUNCTION for MAIN GAME LOOP
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

#pragma region Height Map


	Shader shaderHM("CoreHM.vs", "CoreHM.frag");

	// Create and load texture replace it with your own image path.
	int widthHM, heightHM, nrChannels;

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
	GLfloat yShift = 10.0f; // translate the elevations to our final desired range
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
	unsigned int VAO, VBO, IBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesHM.size() * sizeof(float), &verticesHM[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesHM.size() * sizeof(unsigned), &indicesHM[0], GL_STATIC_DRAW);

#pragma endregion
	//Terrain Generation

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

#pragma region Chessboard Texture

	// Chessboard texture variables
	GLuint textureWhite, textureBlack;
	int widthB, heightB;
#pragma endregion

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
	unsigned char* whiteBlock = SOIL_load_image("res/images/white.jpg", &widthB, &heightB, 0, SOIL_LOAD_RGBA);

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
	unsigned char* blackBlock = SOIL_load_image("res/images/black.jpg", &widthB, &heightB, 0, SOIL_LOAD_RGBA);

	//Specify 2D texture image
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthB, heightB, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackBlock);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(blackBlock);
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion

#pragma endregion
	//Game LOOP
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();

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
		//projection_Board = glm::perspective(glm::radians(camera.GetZoom()), (float)WIDTH / (float)HEIGHT, 0.1f, 100000.0f);

		// Create camera transformation 
		glm::mat4 view_Board(1.0f);
		//view_Board = camera.GetViewMatrix();

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
					glBindTexture(GL_TEXTURE_2D, textureWhite);
					glUniform1i(glGetUniformLocation(chessboardShader.Program, "ourTexture1"), 0);
				}
				else
				{
					// Activate Black texture
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, textureBlack);
					glUniform1i(glGetUniformLocation(chessboardShader.Program, "ourTexture1"), 0);
				}

				// Calculate the model matrix for each object and pass it to the shader before drawing
				glm::mat4 model_Board(1.0f);
				glm::vec3 cubePos(cubePositions[i].x, cubePositions[i].y, cubePositions[i].z - j);
				model_Board = glm::translate(model_Board, cubePos);
				GLfloat angle = 0.0f;
				model_Board = glm::rotate(model_Board, angle, glm::vec3(1.0f, 0.0f, 0.0f));
				glUniformMatrix4fv(modelLoc_Board, 1, GL_FALSE, glm::value_ptr(model_Board));

				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

#pragma endregion

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
		glBindVertexArray(VAO);

		for (int strip = 0; strip < numStrips; strip++)
		{
			glDrawElements(GL_TRIANGLE_STRIP, // primitive type
				numTrisPerStrip + 2, // number of indices to render
				GL_UNSIGNED_INT, // index data type
				(void*)(sizeof(GLuint) * (numTrisPerStrip + 2) * strip)); // offset to starting index
		}
		// Terrain Genaeration

		glBindVertexArray(0); // Unbinding

#pragma endregion
		// DRAW OPENGL WINDOW/VIEWPORT
		glfwSwapBuffers(window);

	}
}

