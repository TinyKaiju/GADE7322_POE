#ifndef SHADER_H
#define SHADER_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glew.h>
using namespace std;

class Shader
{
public:
    GLuint Program;
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
    {
        //Retrieve vertex and fragment source code from file paths
        string vertexCode;
        string fragmentCode;
        ifstream vShaderFile;
        ifstream fShaderFile;

        //Exception handling
        vShaderFile.exceptions(ifstream::badbit);
        fShaderFile.exceptions(ifstream::badbit);
        try
        {
            //Open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);

            stringstream vShaderStream, fShaderStream;

            //Store file contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            //Close files
            vShaderFile.close();
            fShaderFile.close();

            //Convert streams into strings
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (ifstream::failure e)
        {
            cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << endl;
        }

        const GLchar* vShaderCode = vertexCode.c_str();
        const GLchar* fShaderCode = fragmentCode.c_str();

        //BUILD AND COMPILE OUR SHADER PROGRAM

        //Compile time errors variables
        GLuint vertexShader, fragmentShader;
        GLint success;
        GLchar infoLog[512];

        //Build our vertex shader
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);

        //Compile our shader
        glCompileShader(vertexShader);

        //Check for compile time errorsand save the status in success variable
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        //Check if the shader has successfully compiled
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); // Get compilation errors and save in infoLog variable
            cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
        }
        //Build our fragment shader
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);

        //Compile our fragment shader
        glCompileShader(fragmentShader);

        //Check for compile time errorsand save the status in success variable
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        //Check if the shader has successfully compiled
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog); // Get compilation errors and save in infoLog variable
            cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
        }

        //Link the Shaders
        this->Program = glCreateProgram();
        glAttachShader(this->Program, vertexShader);
        glAttachShader(this->Program, fragmentShader);
        glLinkProgram(this->Program);

        //Check for linking errorsand save in success variable
        glGetProgramiv(this->Program, GL_LINK_STATUS, &success);

        //Check if the shader has successfully linked
        if (!success)
        {
            glGetShaderInfoLog(this->Program, 512, NULL, infoLog); // Get compilation errors and save in infoLog variable
            cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
        }

        //Shaders have linked successfully and we can now delete the individual shaders as they are linked in shaderProgram
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Use ()
    {
        glUseProgram(this->Program);
    }
};
#endif