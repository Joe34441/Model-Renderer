#include "Texture.h"

#include <stb_image.h>
#include <iostream>
#include <glad/glad.h>

Texture::Texture() : m_filename(), m_width(0), m_height(0), m_textureID(0)
{

}

Texture::~Texture()
{
	unload();
}

bool Texture::Load(std::string a_filepath)
{
	//convert the image data into OpenGL format
	int width = 0, height = 0, channels = 0;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imageData = stbi_load(a_filepath.c_str(), &width, &height, &channels, 4);
	if (imageData != nullptr)
	{
		m_filename = a_filepath;
		m_width = width;
		m_height = height;
		glGenTextures(1, &m_textureID);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(imageData);
		std::cout << "Successfully loaded Image File: " << a_filepath << std::endl;
		return true;
	}
	std::cout << "Failed to open Image File: " << a_filepath << std::endl;
	return false;
}

unsigned int Texture::LoadCubeMap(std::vector<std::string> a_filenames, unsigned int* cubemap_face_id)
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

	int width, height, nrChannels;

	for (int i = 0; i <a_filenames.size(); ++i) //for each image file of the skybox
	{
		unsigned char* data;
		stbi_set_flip_vertically_on_load(false);
		data = stbi_load(a_filenames[i].c_str(), &width, &height, &nrChannels, 4);
		if (data != nullptr)
		{
			m_width = width;
			m_height = height;
			//texturing allows elements of an image array to be read by shaders
			glTexImage2D(cubemap_face_id[i], 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);

			std::cout << "Successfully loaded Image File: " << a_filenames[i] << std::endl;
		}
		else
		{
			std::cout << "Failed to open Image File: " << a_filenames[i] << std::endl;
			stbi_image_free(data);
		}
	}
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return m_textureID;
}

void Texture::unload()
{
	glDeleteTextures(1, &m_textureID);
}