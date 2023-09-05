#pragma once
#include <map>
#include <string>

//forward declare texture as we only need to keep a pointer here
//and this avoids cyclic dependency
class Texture;

class TextureManager
{
public:
	//this mananger class will act as a singleton objetc for ease of access
	static TextureManager* CreateInstance();
	static TextureManager* GetInstance();
	static void DestroyInstance();

	bool TetxureExists(const char* a_pName);
	unsigned int LoadTexture(const char* a_pfilename);
	unsigned int GetTexture(const char* a_filename);

	void ReleaseTexture(unsigned int a_texture);

private:
	static TextureManager* m_instance;

	//a small structure to reference count a texture
	//references count indicates how many pointers are
	//currently pointing to this texture -> only unload at 0 references
	typedef struct TextureRef
	{
		Texture* pTexture;
		unsigned int refCount;
	}TextureRef;

	std::map<std::string, TextureRef> m_pTextureMap;

	TextureManager();
	~TextureManager();
};