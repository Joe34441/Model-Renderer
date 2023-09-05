#include "ShaderUtil.h"
#include "Utilities.h"
#include <glad/glad.h>
#include <iostream>

//static instance of ShaderUtil
ShaderUtil* ShaderUtil::mInstance = nullptr;
//singleton creation, fetch and destroy functionality
ShaderUtil* ShaderUtil::GetInstance()
{
	if (mInstance == nullptr)
	{
		return ShaderUtil::CreateInstance();
	}
	return mInstance;
}

ShaderUtil* ShaderUtil::CreateInstance()
{
	if (mInstance == nullptr)
	{
		mInstance = new ShaderUtil();
	}
	else
	{
		//print to console that attempt to create multiple instance of ShaderUtil
		std::cout << "Attempt to create multiple instances of ShaderUtil" << std::endl;
	}
	return mInstance;
}

void ShaderUtil::DestroyInstance()
{
	if (mInstance != nullptr)
	{
		delete mInstance;
		mInstance = nullptr;
	}
	else
	{
		//print to console that attempt to destroy null instance of ShaderUtil
		std::cout << "Attempt to destroy null instance of ShaderUtil" << std::endl;
	}
}

ShaderUtil::ShaderUtil()
{

}

ShaderUtil::~ShaderUtil()
{
	//delete any shaders that have not been unloaded
	for (auto iter = mShaders.begin(); iter != mShaders.end(); ++iter)
	{
		glDeleteShader(*iter);
	}
	//destroy any programs that are still dangling about
	for (auto iter = mPrograms.begin(); iter != mPrograms.end(); ++iter)
	{
		glDeleteProgram(*iter);
	}
}

unsigned int ShaderUtil::loadShader(const char* a_filename, unsigned int a_type)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	return instance->loadShaderInternal(a_filename, a_type);
}

unsigned int ShaderUtil::loadShaderInternal(const char* a_filename, unsigned int a_type)
{
	//integer to test for shader creation success
	int success = GL_FALSE;
	//grab the shader source from the file
	char* source = Utility::fileToBuffer(a_filename);
	unsigned int shader = glCreateShader(a_type);
	//set the source buffer for the shader
	glShaderSource(shader, 1, (const char**)&source, 0);
	glCompileShader(shader);
	//as the buffer from fileToBuffer was allocated this needs to be destroyed
	delete[] source;

	//test shader compilation for any errors and display them to console
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (GL_FALSE == success) //shader compilation failed, get logs and display them to console
	{
		int infoLogLength = 0; //variable to store the length of error log
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength]; //allocate buffer to hold data
		glGetShaderInfoLog(shader, infoLogLength, 0, infoLog);
		std::cout << "Unable to compile: " << a_filename << std::endl;
		std::cout << infoLog << std::endl;
		delete[] infoLog;
		return 0;
	}
	//success - add shader to mShader vector
	mShaders.push_back(shader);
	return shader;
}

void ShaderUtil::deleteShader(unsigned int a_shaderID)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	instance->deleteShaderInernal(a_shaderID);
}

void ShaderUtil::deleteShaderInernal(unsigned int a_shaderID)
{
	//loop through the shaders vector
	for (auto iter = mShaders.begin(); iter != mShaders.end(); ++iter)
	{
		if (*iter == a_shaderID) //if we find the shader we are looking for
		{
			glDeleteShader(*iter); //delete the shader
			mShaders.erase(iter); //remove this item from the shaders vector
			break; //break out of this loop
		}
	}
}

unsigned int ShaderUtil::createProgram(const int& a_vertexShader, const int& a_fragmentShader)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	return instance->createProgramInternal(a_vertexShader, a_fragmentShader);
}

unsigned int ShaderUtil::createProgramInternal(const int& a_vertexShader, const int& a_fragmentShader)
{
	//bool value to test for shader program linkage success
	int success = GL_FALSE;

	//create a shader program and attach the shaders to it
	unsigned int handle = glCreateProgram();
	glAttachShader(handle, a_vertexShader);
	glAttachShader(handle, a_fragmentShader);
	//link the shaders together into one shader program
	glLinkProgram(handle);
	//test to see if the program was successfully created
	glGetProgramiv(handle, GL_LINK_STATUS, &success);
	if (GL_FALSE == success) //if something has gone wrong then execute the following
	{
		int infoLogLength = 0; //integer value to tell us the length of the error log
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
		//allocate enough space in a buffer for the error message
		char* infoLog = new char[infoLogLength];
		//fill the buffer with data
		glGetProgramInfoLog(handle, infoLogLength, 0, infoLog);
		//print log message to control
		std::cout << "Shader Linker Error" << std::endl;
		std::cout << infoLog << std::endl;

		//delete the char buffer now we have displayed it
		delete[] infoLog;
		return 0; //return 0, programID 0 is a null program
	}
	//add the program to the shader program vector
	mPrograms.push_back(handle);
	return handle; //return the program ID
}

void ShaderUtil::deleteProgram(unsigned int a_program)
{
	ShaderUtil* instance = ShaderUtil::GetInstance();
	instance->deleteProgramInternal(a_program);
}

void ShaderUtil::deleteProgramInternal(unsigned int a_program)
{
	for (auto iter = mPrograms.begin(); iter != mPrograms.end(); ++iter)
	{
		if (*iter == a_program) //if we find the shader we are looking for
		{
			glDeleteProgram(*iter); //delete the shader
			mPrograms.erase(iter); //remove this item from the shaders vector
			break; //break out of the loop
		}
	}
}