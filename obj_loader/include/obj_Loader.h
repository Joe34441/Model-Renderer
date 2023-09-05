#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>

//A basic vertex class for an OBJ file, supports vertex position, vertex normal, vertex uv coord
class OBJVertex
{
public:
	enum VertexAttributeFlags
	{
		POSITION = (1 << 0), //the position of the vertex
		NORMAL = (1 << 1), //the normal for the vertex
		UVCOORD = (1 << 2), //the uv coordinates for the vertex
	};

	enum Offsets
	{
		PositionOffset = 0,
		NormalOffset = PositionOffset + sizeof(glm::vec4),
		UVCoordOffset = NormalOffset + sizeof(glm::vec4),
	};

	OBJVertex();
	~OBJVertex();

	glm::vec4 position;
	glm::vec4 normal;
	glm::vec2 uvcoord;

	bool operator == (const OBJVertex& a_rhs) const;
	bool operator < (const OBJVertex& a_rhs) const;
};

//inline constructor destructor for OBJVertex class
inline OBJVertex::OBJVertex() : position(0, 0, 0, 1), normal(0, 0, 0, 0), uvcoord(0, 0) {}
inline OBJVertex::~OBJVertex() {}

//inline comparitor methods for OBJVertex
inline bool OBJVertex::operator == (const OBJVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(OBJVertex)) == 0;
}

inline bool OBJVertex::operator < (const OBJVertex& a_rhs) const
{
	return memcmp(this, &a_rhs, sizeof(OBJVertex)) < 0;
}

//An OBJ Material
//Materials have properties such as lights, textures, roughness
class OBJMaterial
{
public:
	OBJMaterial() : name(), kA(0.0f), kD(0.0f), kS(0.0f) {};
	~OBJMaterial() {};

	std::string name;
	//colour amd illumination variables
	glm::vec4 kA; //ambient light colour - alpha componenr stores optial density (Ni)(Refraction Index 0.001 - 10)
	glm::vec4 kD; //Diffuse light colour - alpha component stores dissolve (d)(0-1)
	glm::vec4 kS; //specular light colour - *exponent stored in alpha

	//enum for the texture our OBJ model will support
	enum TextureTypes
	{
		DiffuseTexture = 0,
		SpecularTexture,
		NormalTexture,

		TextureTypes_Count
	};
	//texture will have filenames for loading, once loading ID's stored in ID array
	std::string textureFileNames[TextureTypes_Count];
	unsigned int textureIDs[TextureTypes_Count];
};

//An OBJ Model can be composed of many meshes. Much like any 3D model
//lets use a class to store individual mesh data
class OBJMesh
{
public:
	OBJMesh();
	~OBJMesh();

	glm::vec4 calculateFaceNormal(const unsigned int& a_indexA, const unsigned int& a_indexB, const unsigned int& a_indexC) const;
	void calculateFaceNormals();

	std::string m_name;
	std::vector<OBJVertex> m_vertices;
	std::vector<unsigned int> m_indicies;

	OBJMaterial* m_material;
};

//inline constructor destructor -- to be expanded upon as required
inline OBJMesh::OBJMesh() {}
inline OBJMesh::~OBJMesh() {}

class OBJModel
{
public:
	OBJModel() : m_worldMatrix(glm::mat4(1.0f)), m_path(), m_meshes() {};
	~OBJModel()
	{
		unload(); //function to inload any data loaded in from file
	};

	//load from file location
	bool load(const char* a_filename, float a_scale = 0.1f);
	//function to unload and free memory
	void unload();
	//functions to retrieve path, number of meshes and world matrix of model
	const char* getPath() const { return m_path.c_str(); }
	unsigned int getMeshCount() const { return m_meshes.size(); }
	const glm::mat4& getWorldMatrix() const { return m_worldMatrix; }
	//functions to retrieve mesh by name or index for models that contain multiple meshes
	OBJMesh* getMeshByName(const char* a_name);
	OBJMesh* getMeshByIndex(unsigned int a_index);
	OBJMaterial* getMaterialByName(const char* a_name);
	OBJMaterial* getMaterialByIndex(unsigned int a_index);
	unsigned int GetMaterialCount() const { return m_materials.size(); }

private:
	//function to process line data read in from file
	std::string lineType(const std::string& a_in);
	std::string lineData(const std::string& a_in);
	glm::vec4 processVectorString(const std::string a_data);
	std::vector<std::string> splitStringAtCharacter(std::string data, char a_character);

	void LoadMaterialLibrary(std::string a_mtllib);

	//obj face triplet struct
	typedef struct obj_face_triplet
	{
		unsigned int v;
		unsigned int vt;
		unsigned int vn;
	}obj_face_triplet;
	//function to extract triplet data from OBJ file
	obj_face_triplet ProcessTriplet(std::string a_triplet);

	std::vector<OBJMaterial*> m_materials;
	//vector to storem esh data
	std::vector<OBJMesh*> m_meshes;
	//path to model data - useful for things like texture lookups
	std::string m_path;
	//root mat4 world matrix
	glm::mat4 m_worldMatrix;
};