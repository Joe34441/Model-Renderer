#include "TextureManager.h"
#include "Texture.h"

//set up static poitner for singleton object
TextureManager* TextureManager::m_instance = nullptr;

TextureManager* TextureManager::CreateInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new TextureManager();
	}
	return m_instance;
}

TextureManager* TextureManager::GetInstance()
{
	if (m_instance == nullptr)
	{
		return TextureManager::CreateInstance();
	}
	return m_instance;
}

void TextureManager::DestroyInstance()
{
	if (m_instance != nullptr)
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

TextureManager::TextureManager() : m_pTextureMap()
{

}

TextureManager::~TextureManager()
{
	m_pTextureMap.clear();
}

bool TextureManager::TetxureExists(const char* a_filename)
{
	auto dictIter = m_pTextureMap.find(a_filename);
	return (dictIter != m_pTextureMap.end());
}

unsigned int TextureManager::LoadTexture(const char* a_filename)
{
	if (a_filename != nullptr)
	{
		auto dictionaryIter = m_pTextureMap.find(a_filename);
		if (dictionaryIter != m_pTextureMap.end())
		{
			//texture is already in map, increment ref and return texture ID
			TextureRef& texRef = (TextureRef&)(dictionaryIter->second);
			++texRef.refCount;
			return texRef.pTexture->GetTextureID();
		}
		else
		{
			//texture is not in dictionary load in from file
			Texture* pTexture = new Texture();
			if (pTexture->Load(a_filename))
			{
				//successful load
				TextureRef texRef = { pTexture, 1 };
				m_pTextureMap[a_filename] = texRef;
				return pTexture->GetTextureID();
			}
			else
			{
				delete pTexture;
				return 0;
			}
		}
	}
	return 0;
}

unsigned int TextureManager::GetTexture(const char* a_filename)
{
	auto dictIter = m_pTextureMap.find(a_filename);
	if (dictIter != m_pTextureMap.end())
	{
		TextureRef& texRef = (TextureRef&)(dictIter->second);
		texRef.refCount++;
		return texRef.pTexture->GetTextureID();
	}
	return 0;
}

void TextureManager::ReleaseTexture(unsigned int a_texture)
{
	for (auto dictionaryIter = m_pTextureMap.begin(); dictionaryIter != m_pTextureMap.end(); ++dictionaryIter)
	{
		TextureRef& texRef = (TextureRef&)(dictionaryIter->second);
		if (a_texture == texRef.pTexture->GetTextureID())
		{
			//pre decrement will happen prior to call to ==
			if (--texRef.refCount == 0)
			{
				delete texRef.pTexture;
				texRef.pTexture = nullptr;
				m_pTextureMap.erase(dictionaryIter);
				break;
			}
		}
	}
}
