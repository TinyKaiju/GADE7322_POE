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
    glm::vec3 camPos1 = glm::vec3(0, 15, 0);
    glm::vec2 camRotate1 = glm::vec2(0, -90);
    glm::vec3 camPos2 = glm::vec3(0, 7, 10);
    glm::vec2 camRotate2 = glm::vec2(-90, -40);
    glm::vec3 camPos3 = glm::vec3(7, 2, 7);
    glm::vec2 camRotate3 = glm::vec2(-135, -10);

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

    // Process Mouse input in x and y directions
    void ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch = true)
    {
        xOffset *= this->mouseSensitivity;
        yOffset *= this->mouseSensitivity;

        this->yaw += xOffset;
        this->pitch += yOffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->pitch > 89.0f)
            {
                this->pitch = 89.0f;
            }

            if (this->pitch < -89.0f)
            {
                this->pitch = -89.0f;
            }
        }
    // Update front, right and up vectors using the updated Euler angles
    this->updateCameraVectors();
    }
    // Process mouse scroll for zooming
    void ProcessMouseScroll(GLfloat yOffset)
    {
        if (this->zoom >= 1.0f && this->zoom <= 45.0f)
        {
            this->zoom -= yOffset;
        }

        if (this->zoom <= 1.0f)
        {
            this->zoom = 1.0f;;
        }

        if (this->zoom >= 45.0f)
        {
            this->zoom = 45.0f;;
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
            if (this->position == camPos1)
            {
                this->position = camPos3;
                this->yaw = camRotate3.x;
                this->pitch = camRotate3.y;
            }
            else if (this->position == camPos2)
            {
                this->position = camPos1;
                this->yaw = camRotate1.x;
                this->pitch = camRotate1.y;
            }
            else if (this->position == camPos3)
            {
                this->position = camPos2;
                this->yaw = camRotate2.x;
                this->pitch = camRotate2.y;

            }
            else
            {
                this->position = camPos1;
                this->yaw = camRotate1.x;
                this->pitch = camRotate1.y;
            }
        }
        else
        {
            if (this->position == camPos1)
            {
                this->position = camPos2;
                this->yaw = camRotate2.x;
                this->pitch = camRotate2.y;
            }
            else if (this->position == camPos2)
            {
                this->position = camPos3;
                this->yaw = camRotate3.x;
                this->pitch = camRotate3.y;
            }
            else if (this->position == camPos3)
            {
                this->position = camPos1;
                this->yaw = camRotate1.x;
                this->pitch = camRotate1.y;
            }
            else
            {
                this->position = camPos1;
                this->yaw = camRotate1.x;
                this->pitch = camRotate1.y;
            }
        }

        this->updateCameraVectors();
    }


};