#pragma once
#include <string>
#include <vector>

//a class to store texture data
//a texture is a data buffer that contains values which relate to pixel colours

class Texture
{
public:
	//constructor & destructor
	Texture();
	~Texture();

	//function to load a texture from file
	bool Load(std::string a_filename);
	unsigned int LoadCubeMap(std::vector<std::string> a_filenames, unsigned int* cubemap_face_id);
	void unload();
	//get filename
	const std::string& GetFileName() const { return m_filename; }
	unsigned int GetTextureID() const { return m_textureID; }
	void GetDimensions(unsigned int& a_w, unsigned int& a_h) const;

private:
	std::string m_filename;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_textureID;
};

inline void Texture::GetDimensions(unsigned int& a_w, unsigned int& a_h) const
{
	a_w = m_width;
	a_h = m_height;
}