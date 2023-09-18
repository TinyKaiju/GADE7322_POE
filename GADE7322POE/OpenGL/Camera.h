#pragma once

#include <iostream>
#include <vector>
using namespace std;

// GLEW
#define GLEW_STATIC
#include <GL/glew.h> 

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// The directions our camera can move
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 10.0f;
const GLfloat SENSITIVITY = 0.25f;
const GLfloat ZOOM = 45.0f;

class Camera
{
private:

    // Camera Attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Eular Angles
    GLfloat yaw;
    GLfloat pitch;

    // Camera options
    GLfloat movementSpeed;
    GLfloat mouseSensitivity;
    GLfloat zoom;

    // Cycle Camera Positions
    glm::vec3 ccPos1 = glm::vec3(0, 15, 0);
    glm::vec2 ccPos1Rotate = glm::vec2(0, -90);
    glm::vec3 ccPos2 = glm::vec3(0, 7, 10);
    glm::vec2 ccPos2Rotate = glm::vec2(-90, -40);
    glm::vec3 ccPos3 = glm::vec3(7, 2, 7);
    glm::vec2 ccPos3Rotate = glm::vec2(-135, -10);

    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        front.y = sin(glm::radians(this->pitch));
        front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        this->front = glm::normalize(front);

        // Recalculate the right and up vectors
        this->right = glm::normalize(glm::cross(this->front, this->worldUp));
        this->up = glm::normalize(glm::cross(this->right, this->front));
        // Normalise the vectors for slower movement.
    }

public:

    //Constructor with Vectors
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        GLfloat yaw = YAW, GLfloat pitch = PITCH) : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
    {
        this->position = position;
        this->worldUp = up;
        this->yaw = yaw;
        this->pitch = pitch;
        this->updateCameraVectors();
    }

    //Constructor with Scalars
    Camera(
        GLfloat posX, GLfloat posY, GLfloat posZ,
        GLfloat upX, GLfloat upY, GLfloat upZ,
        GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)),
        movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
    {
        this->position = glm::vec3(posX, posY, posZ);
        this->worldUp = glm::vec3(upX, upY, upZ);
        this->yaw = yaw;
        this->pitch = pitch;
        this->updateCameraVectors();
    }

    // Getter for view matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->position, this->position + this->front, this->up);
    }

    // Process keyboard input to detect which way the camera should move
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->movementSpeed * deltaTime;

        if (direction == FORWARD)
        {
            this->position += this->front * velocity;
        }

        if (direction == BACKWARD)
        {
            this->position -= this->front * velocity;
        }

        if (direction == LEFT)
        {
            this->position -= this->right * velocity;
        }

        if (direction == RIGHT)
        {
            this->position += this->right * velocity;
        }
    }

    // Getter for Zoom variable
    GLfloat GetZoom()
    {
        return this->zoom;
    }

    // Getter
    glm::vec3 GetPosition()
    {
        return this->position;
    }

    void CycleCamera(string str)
    {
        //Initial values
        this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        this->yaw = -128.1f;
        this->pitch = -42.4f;

        if (str == "Left")
        {
            if (this->position == ccPos1)
            {
                this->position = ccPos3;
                this->yaw = ccPos3Rotate.x;
                this->pitch = ccPos3Rotate.y;
            }
            else if (this->position == ccPos2)
            {
                this->position = ccPos1;
                this->yaw = ccPos1Rotate.x;
                this->pitch = ccPos1Rotate.y;
            }
            else if (this->position == ccPos3)
            {
                this->position = ccPos2;
                this->yaw = ccPos2Rotate.x;
                this->pitch = ccPos2Rotate.y;

            }
            else
            {
                this->position = ccPos1;
                this->yaw = ccPos1Rotate.x;
                this->pitch = ccPos1Rotate.y;
            }
        }
        else
        {
            if (this->position == ccPos1)
            {
                this->position = ccPos2;
                this->yaw = ccPos2Rotate.x;
                this->pitch = ccPos2Rotate.y;
            }
            else if (this->position == ccPos2)
            {
                this->position = ccPos3;
                this->yaw = ccPos3Rotate.x;
                this->pitch = ccPos3Rotate.y;
            }
            else if (this->position == ccPos3)
            {
                this->position = ccPos1;
                this->yaw = ccPos1Rotate.x;
                this->pitch = ccPos1Rotate.y;
            }
            else
            {
                this->position = ccPos1;
                this->yaw = ccPos1Rotate.x;
                this->pitch = ccPos1Rotate.y;
            }
        }

        this->updateCameraVectors();
    }


};