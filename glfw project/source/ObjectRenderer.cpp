#include "ObjectRenderer.h"
#include "ShaderUtil.h"
#include "Dispatcher.h"
#include "Utilities.h"
#include "TextureManager.h"
#include "Texture.h"
#include "obj_Loader.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>


ObjectRenderer::ObjectRenderer()
{
	
}

ObjectRenderer::~ObjectRenderer()
{

}

bool ObjectRenderer::onCreate()
{
	Dispatcher* dp = Dispatcher::GetInstance();
	if (dp)
	{
		dp->Subscribe(this, &ObjectRenderer::onWindowResize);
	}
	//get an instance of the texture manager
	TextureManager::CreateInstance();

	m_backgroundColour = glm::vec3(0.45f, 0.8f, 1.0f); //default light blue 0.45f, 0.8f, 1.0f, 1.0f

	//set the clear colour and enable depth testing and backface cullng
	glClearColor(m_backgroundColour.x, m_backgroundColour.y, m_backgroundColour.z, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//create shader program 
	unsigned int vertexShader = ShaderUtil::loadShader("./resource/shaders/vertex.glsl", GL_VERTEX_SHADER);
	unsigned int fragmentShader = ShaderUtil::loadShader("./resource/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	m_uiProgram = ShaderUtil::createProgram(vertexShader, fragmentShader);

#pragma region Grid Lines

	//create a grid of lines to be drawn during our update
	//create a 50x50 square grid
	lines = new Line[240];

	for (int i = 0, j = 0; i < 101; ++i, j += 2)
	{
		lines[j].v0.position = glm::vec4(-50 + i, 0.0f, 50.0f, 1.0f);
		lines[j].v0.colour = (i == 50) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		lines[j].v1.position = glm::vec4(-50 + i, 0.0f, -50.0f, 1.0f);
		lines[j].v1.colour = (i == 50) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		lines[j + 1].v0.position = glm::vec4(50, 0.0f, -50.0f + i, 1.0f);
		lines[j + 1].v0.colour = (i == 50) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		lines[j + 1].v1.position = glm::vec4(-50, 0.0f, -50.0f + i, 1.0f);
		lines[j + 1].v1.colour = (i == 50) ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	//create a vertex buffer to hold our line data
	glGenBuffers(1, &m_lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
	//fill vertex buffer with line data
	glBufferData(GL_ARRAY_BUFFER, 240 * sizeof(Line), lines, GL_STATIC_DRAW);

	//enable the vertex array state, since we're sending in an array of verticies
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//specify where our vertex array is, how many components each vertex has, the data type of
	//each compnent and whether the data is normalised or not
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

#pragma endregion

	//create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

	//create a perspective projection matrix with a 90 degree field of view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);
	

#pragma region Skybox

	int width, height, nrChannels;
	unsigned char* data;

	glGenTextures(1, &m_CubeMapTexID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTexID);

	std::vector<std::string> textures_faces = { "./resource/skybox/right.png", "./resource/skybox/left.png",
										"./resource/skybox/top.png", "./resource/skybox/bottom.png",
										"./resource/skybox/front.png", "./resource/skybox/back.png" };

	unsigned int cubemap_image_tag[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
										  GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
										  GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };


	Texture* pTexture = new Texture();
	m_CubeMapTexID = pTexture->LoadCubeMap(textures_faces, cubemap_image_tag);

	//make skybox VBO and VAO and load shaders for skybox
	unsigned int sb_vertexShader = ShaderUtil::loadShader("./resource/shaders/SB_vertex.glsl", GL_VERTEX_SHADER);
	unsigned int sb_fragmentShader = ShaderUtil::loadShader("./resource/shaders/SB_fragment.glsl", GL_FRAGMENT_SHADER);

	m_SBProgramID = ShaderUtil::createProgram(sb_vertexShader, sb_fragmentShader);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
	 	 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glDepthFunc(GL_LEQUAL);

	//create a vertex buffer to hold our line data
	glGenBuffers(1, &m_SBVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_SBVBO);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 108, skyboxVertices, 0);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(float) * 108, skyboxVertices, 0);

	//enable the vertex array state, since we're sending in an array of verticies
	glGenVertexArrays(1, &m_SBVAO);
	glBindVertexArray(m_SBVAO);

	//enable the vertex array state, since we're sending in an array of vertices
	glEnableVertexAttribArray(0);

	//specify where our vertex array is, how many components each vertex has, the data type of
	//each compnent and whether the data is normalised or not
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
	glBindBuffer(GL_ARRAY_BUFFER, m_SBVBO);

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

#pragma endregion
	
#pragma region ImGui Setup

	SetupGUI();

#pragma endregion

	return true;
}

void ObjectRenderer::Update(float _deltaTime)
{
	Utility::freeMovement(m_cameraMatrix, _deltaTime, 2.0f);

	UpdateGUI();
}

void ObjectRenderer::Draw()
{
	glDepthFunc(GL_LESS);

	glClearColor(m_backgroundColour.x, m_backgroundColour.y, m_backgroundColour.z, 1.0f);
	//clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	glm::mat4 projectionViewMatrix = m_projectionMatrix * viewMatrix;

	//enable shaders 
	glUseProgram(m_uiProgram);

	//send the projection matrix to the vertex shader
	//ask the shader program for the location of the projection view matrix uniform variable
	unsigned int projectionViewUniformLocation = glGetUniformLocation(m_uiProgram, "ProjectionViewMatrix");
	//send this location a pointer to our glm::mat4 (send across float data)
	glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(projectionViewMatrix));

#pragma region Lines

	if (m_gridLinesEnabled)
	{
		//draw to the screen 
		glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
		glBufferData(GL_ARRAY_BUFFER, 240 * sizeof(Line), lines, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		//specify where our vertex array is, how many components each vertex has,
		//the data type of each component and whether the data is normalised or not
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

		glDrawArrays(GL_LINES, 0, 240 * 2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glUseProgram(0);
	}

#pragma endregion

#pragma region Object Mesh & Material
	

	for (int i = 0; i < m_actorModels.size(); ++i)
	{
		m_objModel = m_actorModels[i];

		if (m_objModel)
		{
			glUseProgram(m_objProgram);

			//set the projection view matrix for this shader
			projectionViewUniformLocation = glGetUniformLocation(m_objProgram, "ProjectionViewMatrix");
			//send this location a pointer to our glm::mat4 (send across float data)
			glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(projectionViewMatrix));

			int index = GetActorIndex(m_actors[i]);
			for (int i = 0; i < m_objModel->getMeshCount(); ++i)
			{
				//use a mat4 to set position, rotation and scale
				glm::mat4 trans = glm::mat4(1.0f);

				//apply translation for the objects position
				trans = glm::translate(trans, glm::vec3(m_actorPosition[index][0], m_actorPosition[index][2], m_actorPosition[index][1])); //switch around Y and Z axis to be correct

				//apply rotation for each axis
				trans = glm::rotate(trans, glm::radians(m_actorRotation[index][0]), glm::vec3(1.0, 0.0, 0.0));
				trans = glm::rotate(trans, glm::radians(m_actorRotation[index][1]), glm::vec3(0.0, 1.0, 0.0));
				trans = glm::rotate(trans, glm::radians(m_actorRotation[index][2]), glm::vec3(0.0, 0.0, 1.0));

				//apply the scale factor
				trans = glm::scale(trans, glm::vec3(m_actorScale[index]));

				//get the model matrix location from the shader program
				int modelMatrixUniformLocation = glGetUniformLocation(m_objProgram, "transform");
				//send the OBJ Model's world matrix data across to the shader program
				glUniformMatrix4fv(modelMatrixUniformLocation, 1, false, glm::value_ptr(trans));

				int cameraPositionUniformLocation = glGetUniformLocation(m_objProgram, "camPos");
				glUniform4fv(cameraPositionUniformLocation, 1, glm::value_ptr(m_cameraMatrix[3]));

				OBJMesh* pMesh = m_objModel->getMeshByIndex(i);
				//send material data to shader
				int kA_location = glGetUniformLocation(m_objProgram, "kA");
				int kD_location = glGetUniformLocation(m_objProgram, "kD");
				int kS_location = glGetUniformLocation(m_objProgram, "kS");

				OBJMaterial* pMaterial = pMesh->m_material;
				if (pMaterial != nullptr)
				{
					//send the OBJ Model's world matrix data across to the shader program
					glUniform4fv(kA_location, 1, glm::value_ptr(pMaterial->kA));
					glUniform4fv(kD_location, 1, glm::value_ptr(pMaterial->kD));
					glUniform4fv(kS_location, 1, glm::value_ptr(pMaterial->kS));

					//get the location of the diffuse texture
					int texUniformLoc = glGetUniformLocation(m_objProgram, "DiffuseTexture");
					glUniform1i(texUniformLoc, 0); //set diffuse texture to be GL_Texture0

					glActiveTexture(GL_TEXTURE0); //set the active texture unit to texture0
					//bind the texture for diffuse for this material to the texture0
					glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::DiffuseTexture]);

					//get the location of the specular texture
					texUniformLoc = glGetUniformLocation(m_objProgram, "SpecularTexture");
					glUniform1i(texUniformLoc, 1); //set diffuse texture to be gl_texture1

					glActiveTexture(GL_TEXTURE1); //set the active texture unit to texture1
					//bind the texture for specular for this material to the texture1
					glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::SpecularTexture]);

					//get the location of the normal texture
					texUniformLoc = glGetUniformLocation(m_objProgram, "NormalTexture");
					glUniform1i(texUniformLoc, 2); //set normal texture to be GL_Texture2

					glActiveTexture(GL_TEXTURE2); //set the active texture unit to texture2
					//bind the texture for specular for this material to the texture2
					glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::NormalTexture]);
				}
				else //no material to obtain lighting information from use defaults
				{
					//send the OBJ Model's world matrix data across to the shader program
					glUniform4fv(kA_location, 1, glm::value_ptr(glm::vec4(0.25f, 0.25f, 0.25f, 1.0f)));
					glUniform4fv(kD_location, 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
					glUniform4fv(kS_location, 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 64.0f)));
				}

				glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);
				glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(OBJVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_objModelBuffer[1]);
				glEnableVertexAttribArray(0); //position
				glEnableVertexAttribArray(1); //normal
				glEnableVertexAttribArray(2); //uv coord

				glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), ((char*)0) + OBJVertex::PositionOffset);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(OBJVertex), ((char*)0) + OBJVertex::NormalOffset);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(OBJVertex), ((char*)0) + OBJVertex::UVCoordOffset);

				glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indicies.size() * sizeof(unsigned int), pMesh->m_indicies.data(), GL_STATIC_DRAW);
				glDrawElements(GL_TRIANGLES, pMesh->m_indicies.size(), GL_UNSIGNED_INT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);

			glUseProgram(0);
		}
	}




#pragma endregion

#pragma region Skybox

	if (m_skyboxEnabled)
	{
		//draw skybox
		glDepthFunc(GL_LEQUAL);
		glUseProgram(m_SBProgramID);
		glDepthMask(GL_FALSE);

		projectionViewUniformLocation = glGetUniformLocation(m_SBProgramID, "ProjectionViewMatrix");
		//send this location a pointer to our glm::mat4 (send across float data)
		glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(projectionViewMatrix));

		glBindVertexArray(m_SBVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTexID);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		glUseProgram(0);

		glDepthFunc(GL_LESS);
	}

#pragma endregion

}

void ObjectRenderer::Destroy()
{
	delete m_objModel;
	delete[] lines;
	glDeleteBuffers(1, &m_lineVBO);
	ShaderUtil::deleteProgram(m_uiProgram);
	TextureManager::DestroyInstance();
	ShaderUtil::DestroyInstance();
}

void ObjectRenderer::onWindowResize(WindowResizeEvent* e)
{
	if (e->GetWidth() > 0 && e->GetHeight() > 0)
	{
		//create a perspective projection matrix with a 90 degree field of view and widescreen aspect ratio
		m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, e->GetWidth() / (float)e->GetHeight(), 0.1f, 1000.0f);
		glViewport(0, 0, e->GetWidth(), e->GetHeight());
	}

	e->Handled();
}

void ObjectRenderer::LoadModel(std::string _filename)
{
	m_objModel = new OBJModel();
	if (m_objModel->load(_filename.c_str()), 0.1f)
	{

		TextureManager* pTM = TextureManager::GetInstance();
		//load in texture for model if any are present
		for (int i = 0; i < m_objModel->GetMaterialCount(); ++i)
		{
			OBJMaterial* mat = m_objModel->getMaterialByIndex(i);
			for (int n = 0; n < OBJMaterial::TextureTypes::TextureTypes_Count; ++n)
			{
				if (mat->textureFileNames[n].size() > 0)
				{
					unsigned int textureID = pTM->LoadTexture(mat->textureFileNames[n].c_str());
					mat->textureIDs[n] = textureID;
				}
			}
		}

		//setup shaders for OBJ Model rendering
		unsigned int obj_vertexShader = ShaderUtil::loadShader("./resource/shaders/obj_vertex.glsl", GL_VERTEX_SHADER);
		unsigned int obj_fragmentShader = ShaderUtil::loadShader("./resource/shaders/obj_fragment.glsl", GL_FRAGMENT_SHADER);
		//create OBJ shader program
		m_objProgram = ShaderUtil::createProgram(obj_vertexShader, obj_fragmentShader);
		//set up vertex and index buffer for OBJ rendering
		glGenBuffers(2, m_objModelBuffer);
		//set up vertex buffer date
		glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);


		std::string newName = "Actor";

		m_actorModels.push_back(m_objModel);
		//m_actors.push_back(newName);
		m_isActorSelected.push_back(false);
		m_actorPosition.push_back({ 0.0f, 0.0f, 0.0f });
		m_actorRotation.push_back({ 0.0f, 0.0f, 0.0f });
		m_actorScale.push_back(1.0f);

		std::vector<std::string> currentActors = m_actors;

		if (std::count(currentActors.begin(), currentActors.end(), newName))
		{
			//new actor name already exists in scene
			std::string newNameDuplicate = newName + " (1)";
			newName = newNameDuplicate;
			for (int i = 0; i < currentActors.size(); ++i)
			{
				if (std::count(currentActors.begin(), currentActors.end(), newNameDuplicate))
				{
					newNameDuplicate = newNameDuplicate.substr(0, newNameDuplicate.length() - 3) + "(" + std::to_string(i + 2) + ")";
				}
				else
				{
					newName = newNameDuplicate;
					break;
				}
			}
		}

		m_actors.push_back(newName);
	}
	else
	{
		std::cout << "Failed to load model" << std::endl;
	}
}

void ObjectRenderer::UpdateGUI()
{
	//setup imgui window to control colour
	ImGuiIO io = ImGui::GetIO();

	ImVec2 windowSize = ImVec2(300.0f, 200.0f);
	ImVec2 windowPos = ImVec2(io.DisplaySize.x * 0.01f, io.DisplaySize.y * 0.2f);

#pragma region Settings Panel

	ImGui::SetNextWindowPos(m_settingsPanel.windowPosition, ImGuiCond_Always);
	ImGui::SetNextWindowSize(m_settingsPanel.windowSize, ImGuiCond_Always);
	
	if (ImGui::Begin(m_settingsPanel.windowName.c_str(), &m_settingsPanel.expanded, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (ImGui::CollapsingHeader("GUI"))
		{
			ImGui::Checkbox("Show frame data", &m_frameDataEnabled);

			ImGui::Text("Reset to default settings");
			if (ImGui::Button("Reset", ImVec2(70, 20)))
			{
				SetupGUI();
			}
		}

		if (ImGui::CollapsingHeader("Background"))
		{
			ImGui::Checkbox("Skybox Enabled", &m_skyboxEnabled);
			ImGui::Text("Default Colour:");
			ImGui::ColorEdit3(" ", glm::value_ptr(m_backgroundColour));
		}

		if (ImGui::CollapsingHeader("Grid Lines"))
		{
			ImGui::Checkbox("Draw Grid Lines", &m_gridLinesEnabled);
		}
	}
	m_settingsPanel.expanded = ImGui::IsWindowCollapsed() ? false : true;

	ImGui::End();

#pragma endregion

#pragma region OBJ Loading Panel

	float yPos = m_settingsPanel.expanded ? m_settingsPanel.windowSize.y - 1.0f : m_settingsPanel.windowPosition.y + 18.0f;
	ImGui::SetNextWindowPos(ImVec2(0.0f, yPos), ImGuiCond_Always);
	ImGui::SetNextWindowSize(m_objLoadingPanel.windowSize);


	if (ImGui::Begin(m_objLoadingPanel.windowName.c_str(), &m_objLoadingPanel.expanded, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (ImGui::Button("Select Model"))
		{
			m_fileDialog.Open();
		}
		
		m_fileDialog.Display();
		
		if (m_fileDialog.HasSelected())
		{
			std::cout << "Selected filename" << m_fileDialog.GetSelected().string() << std::endl;
			LoadModel(m_fileDialog.GetSelected().string());
			m_fileDialog.ClearSelected();
		}
	}
	m_objLoadingPanel.expanded = ImGui::IsWindowCollapsed() ? false : true;

	ImGui::End();

#pragma endregion

#pragma region Actor Hierarchy Panel

	m_actorsPanel.windowPosition = ImVec2(io.DisplaySize.x - m_actorsPanel.windowSize.x, 0.0f);
	ImGui::SetNextWindowPos(m_actorsPanel.windowPosition, ImGuiCond_Always);
	ImGui::SetNextWindowSize(m_actorsPanel.windowSize, ImGuiCond_Always);

	if (ImGui::Begin(m_actorsPanel.windowName.c_str(), &m_actorsPanel.expanded, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (ImGui::BeginListBox("##titleActorList", ImVec2(m_actorsPanel.windowSize.x - 17.0f, m_actorsPanel.windowSize.y - 35.0f)))
		{
			for (int actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex)
			{
				if (ImGui::Selectable(m_actors[actorIndex].c_str(), &m_isActorSelected[actorIndex]))
				{
					m_isActorSelected[actorIndex] = true;
					m_selectedActor = m_actors[actorIndex];
				}
				else if (m_isActorSelected[actorIndex])
				{
					m_isActorSelected[actorIndex] = false;
				}
			}
			ImGui::EndListBox();
		}
	}
	m_actorsPanel.expanded = ImGui::IsWindowCollapsed() ? false : true;

	ImGui::End();

#pragma endregion

#pragma region Actor Properties Panel

	m_actorPropertiesPanel.windowPosition = ImVec2(io.DisplaySize.x - m_actorPropertiesPanel.windowSize.x, 0.0f);
	m_actorPropertiesPanel.windowSize.y = m_actorsPanel.expanded ? io.DisplaySize.y - m_actorsPanel.windowSize.y + 1.0f : io.DisplaySize.y - 18.0f;
	yPos = m_actorsPanel.expanded ? m_actorsPanel.windowSize.y - 1.0f : m_actorsPanel.windowPosition.y + 18.0f;
	ImGui::SetNextWindowPos(ImVec2(m_actorPropertiesPanel.windowPosition.x, yPos), ImGuiCond_Always);
	ImGui::SetNextWindowSize(m_actorPropertiesPanel.windowSize);

	if (ImGui::Begin(m_actorPropertiesPanel.windowName.c_str(), &m_actorPropertiesPanel.expanded, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		std::string title = m_selectedActor + "##titleName";
		if (m_selectedActor.length() > 0)
		{
			if (ImGui::CollapsingHeader(title.c_str()))
			{
				ImGui::Text("Rename Actor");

				static std::string newName;
				if (ImGui::InputText("##titleRename", &newName))
				{

				}

				if (ImGui::Button("Set##titleRename", ImVec2(70, 20)))
				{
					if (std::count(m_actors.begin(), m_actors.end(), newName))
					{
						//new actor name already exists in scene
						std::string newNameDuplicate = newName + " (1)";
						for (int i = 0; i < m_actors.size(); ++i)
						{
							if (std::count(m_actors.begin(), m_actors.end(), newNameDuplicate))
							{
								newNameDuplicate = newNameDuplicate.substr(0, newNameDuplicate.length() - 3) + "(" + std::to_string(i + 2) + ")";
							}
							else
							{
								int index = GetActorIndex(m_selectedActor);
								if (index >= 0)
								{
									m_actors.at(index) = newNameDuplicate;
									m_selectedActor = m_actors[index];
								}
								break;
							}
						}
					}
					else
					{
						int index = GetActorIndex(m_selectedActor);
						if (index >= 0)
						{
							m_actors.at(index) = newName;
							m_selectedActor = m_actors[index];
						}
					}
				}

				ImGui::Text("Duplicate Actor");
				if (ImGui::Button("Duplicate##titleDuplicate", ImVec2(70, 20)))
				{
					std::string newName = m_selectedActor;

					m_actorModels.push_back(m_actorModels[GetActorIndex(m_selectedActor)]);

					//m_actors.push_back(newName);
					m_isActorSelected.push_back(false);
					m_actorPosition.push_back({ 0.0f, 0.0f, 0.0f });
					m_actorRotation.push_back({ 0.0f, 0.0f, 0.0f });
					m_actorScale.push_back(1.0f);

					std::vector<std::string> currentActors = m_actors;

					if (std::count(currentActors.begin(), currentActors.end(), newName))
					{
						//new actor name already exists in scene
						std::string newNameDuplicate = newName + " (1)";
						newName = newNameDuplicate;
						for (int i = 0; i < currentActors.size(); ++i)
						{
							if (std::count(currentActors.begin(), currentActors.end(), newNameDuplicate))
							{
								newNameDuplicate = newNameDuplicate.substr(0, newNameDuplicate.length() - 3) + "(" + std::to_string(i + 2) + ")";
							}
							else
							{
								newName = newNameDuplicate;
								break;
							}
						}
					}

					m_actors.push_back(newName);
				}

				ImGui::Text("Deselect Actor");
				if (ImGui::Button("Deselect##titleDeselect", ImVec2(70, 20)))
				{
					m_selectedActor = "";
				}

				ImGui::Text("Destroy Actor");
				if (ImGui::Button("Destroy##titleDestroy", ImVec2(70, 20)))
				{
					int index = GetActorIndex(m_selectedActor);
					if (index >= 0)
					{
						m_actorModels.erase(m_actorModels.begin() + index);
						m_actors.erase(m_actors.begin() + index);
						m_isActorSelected.erase(m_isActorSelected.begin() + index);
						m_actorPosition.erase(m_actorPosition.begin() + index);
						m_actorRotation.erase(m_actorRotation.begin() + index);
						m_actorScale.erase(m_actorScale.begin() + index);
						m_selectedActor = "";
					}
				}
				
				ImGui::Text("");
				if (ImGui::CollapsingHeader("Transform##titleTransform") && m_selectedActor != "")
				{
					int index = GetActorIndex(m_selectedActor);

					ImGui::Text("Position");
					ImGui::Text("X:		Y:		Z:");
					float sliderValuePosition[3] = { m_actorPosition[index][0], m_actorPosition[index][1], m_actorPosition[index][2] };
					if (ImGui::SliderFloat3("##titlePosition", &sliderValuePosition[0], -50.0f, 50.0f))
					{
						std::vector<float> vec3newValuesPosition = { sliderValuePosition[0], sliderValuePosition[1], sliderValuePosition[2] };
						if (m_actorPosition[index] != vec3newValuesPosition)
						{
							m_actorPosition.at(index) = vec3newValuesPosition;
						}
					}

					ImGui::Text("\nRotation");
					ImGui::Text("X:		Y:		Z:");
					float sliderValueRotation[3] = { m_actorRotation[index][0], m_actorRotation[index][1], m_actorRotation[index][2] };
					if (ImGui::SliderFloat3("##titleRotation", &sliderValueRotation[0], 0.0f, 360.0f))
					{
						std::vector<float> vec3newValuesRotation = { sliderValueRotation[0], sliderValueRotation[1], sliderValueRotation[2] };
						if (m_actorRotation[index] != vec3newValuesRotation)
						{
							m_actorRotation.at(index) = vec3newValuesRotation;
						}
					}

					ImGui::Text("\nScale:");
					static float inputValueScale = m_actorScale[index];
					static std::string currentActor = m_selectedActor;
					if (m_selectedActor != currentActor)
					{
						currentActor = m_selectedActor;
						inputValueScale = m_actorScale[index];
					}
					if (ImGui::InputFloat("##titleScale", &inputValueScale))
					{

					}
					
					if (ImGui::Button("Apply##titleApply", ImVec2(50, 20)))
					{
						//value in the ImGui::InputFloat is rounded to 3dp, but the hidden, actual value is not.
						//string manipulation to convert the actual value into the displayed value
						std::string value = std::to_string(inputValueScale);
						for (int i = 0; i < value.length(); ++i)
						{
							if (value[i] == '.')
							{
								if (value.length() >= i + 4)
								{
									std::string decimalValue = value.substr(i + 1, value.length() - i);
									int finalDecimalValue = std::stoi(decimalValue.substr(0, 4));
									std::string lastValue = decimalValue.substr(3, 1);

									if (std::stoi(lastValue) >= 5)
									{
										finalDecimalValue = std::stof(std::to_string(finalDecimalValue).substr(0, 3));
										finalDecimalValue++;
									}
									std::string result = value.substr(0, i) + "." + std::to_string(finalDecimalValue).substr(0, 3);
									inputValueScale = std::stof(result);
								}
								break;
							}
						}
						if (m_actorScale[index] != inputValueScale)
						{
							m_actorScale.at(index) = inputValueScale;
						}
					}
				}
			}
		}
	}
	m_actorPropertiesPanel.expanded = ImGui::IsWindowCollapsed() ? false : true;

	ImGui::End();

#pragma endregion

#pragma region Frame Data

	if (m_frameDataEnabled)
	{
		const float distance = 10.0f;
		static int corner = 0;

		windowPos = ImVec2((corner & 1) ? io.DisplaySize.x - distance : distance, (corner & 2) ? io.DisplaySize.y - distance : distance);
		ImVec2 windowPosPivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);

		windowPos = ImVec2(windowSize.x + 10.0f, 10.0f);

		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
		ImGui::SetNextWindowBgAlpha(0.3f);

		if (ImGui::Begin("Frame Data", &m_frameDataEnabled, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
			ImGui::Separator();
			ImGui::Text("Application Average:   \n FPS : %0.1f \n %0.3f ms/frame", io.Framerate, 1000.0f / io.Framerate);

			if (ImGui::IsMousePosValid())
			{
				ImGui::Text("Mouse Position: \n X : %0.1f \n Y : %0.1f", io.MousePos.x, io.MousePos.y);
			}
			else
			{
				ImGui::Text("Mouse Position: \n <Not active in window>");
			}
		}

		ImGui::End();
	}


#pragma endregion
}

int ObjectRenderer::GetActorIndex(std::string _actor)
{
	if (_actor == "") return -1;

	auto iter = find(m_actors.begin(), m_actors.end(), _actor);
	if (iter != m_actors.end())
	{
		return iter - m_actors.begin();
	}
	else
	{
		return -1;
	}
}

void ObjectRenderer::SetupGUI()
{
	m_settingsPanel.windowName = "Settings";
	m_settingsPanel.windowSize = ImVec2(300.0f, 275.0f);
	m_settingsPanel.windowPosition = ImVec2(0.0f, 0.0f);
	m_settingsPanel.expanded = true;


	m_objLoadingPanel.windowName = "Load Model";
	m_objLoadingPanel.windowSize = ImVec2(300.0f, 60.0f);
	m_objLoadingPanel.windowPosition = m_settingsPanel.windowSize;
	m_objLoadingPanel.expanded = true;

	m_actorsPanel.windowName = "Actors";
	m_actorsPanel.windowSize = ImVec2(300.0f, 300.0f);
	m_actorsPanel.windowPosition = ImVec2(0.0f, 0.0f); //is set later using io reference
	m_actorsPanel.expanded = true;

	m_actorPropertiesPanel.windowName = "Actor Properties";
	m_actorPropertiesPanel.windowSize = ImVec2(300.0f, 0.0f); // x axis size is set later using io reference
	m_actorPropertiesPanel.windowPosition = ImVec2(0.0f, 0.0f); //is set later using io reference
	m_actorPropertiesPanel.expanded = true;


	m_skyboxEnabled = true;
	m_frameDataEnabled = false;
	m_gridLinesEnabled = true;

	m_backgroundColour = glm::vec3(0.45f, 0.8f, 1.0f);
}