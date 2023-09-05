#pragma once
#include <vector>

class ShaderUtil
{
public:
	static ShaderUtil* CreateInstance();
	static ShaderUtil* GetInstance();
	static void DestroyInstance();
	static unsigned int loadShader(const char* a_filename, unsigned int a_type);
	static void deleteShader(unsigned int a_shaderID);
	static unsigned int createProgram(const int& a_vertexShader, const int& a_fragmentShader);
	static void deleteProgram(unsigned int a_program);

private:
	//private Constructor and Destructor
	//ShaderUtil implements a singleton design pattern
	ShaderUtil();
	~ShaderUtil();

	std::vector<unsigned int> mShaders;
	std::vector<unsigned int> mPrograms;

	unsigned int loadShaderInternal(const char* a_filename, unsigned int a_type);
	void deleteShaderInernal(unsigned int a_shaderID);
	unsigned int createProgramInternal(const int& a_vertexShader, const int& a_fragmentShader);
	void deleteProgramInternal(unsigned int a_program);
	static ShaderUtil* mInstance;
};