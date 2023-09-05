#pragma once

#include "Application.h"
#include "ApplicationEvent.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui.h>
#include <ImFileBrowser.h>

//forward declare OBJ model
class OBJModel;

class ObjectRenderer : public Application
{
public:
	ObjectRenderer();
	virtual ~ObjectRenderer();

	void onWindowResize(WindowResizeEvent* e);

protected:
	virtual bool onCreate();
	virtual void Update(float _deltaTime);
	virtual void SetupGUI();
	virtual void UpdateGUI();
	virtual int GetActorIndex(std::string _actor);
	virtual void LoadModel(std::string _filename);
	virtual void Draw();
	virtual void Destroy();

private:
	//structure for a simple vertex - interleaved (position, colour)
	typedef struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
	}Vertex;

	typedef struct Line
	{
		Vertex v0;
		Vertex v1;
	}Line;

	glm::mat4 m_cameraMatrix;
	glm::mat4 m_projectionMatrix;

	//shader programs
	unsigned int m_uiProgram;
	unsigned int m_objProgram;
	unsigned int m_lineVBO;
	unsigned int m_objModelBuffer[2];

	//model
	OBJModel* m_objModel;
	Line* lines;
	std::vector<OBJModel*> m_actorModels;

	//skybox
	unsigned int m_CubeMapTexID;
	unsigned int m_SBVAO;
	unsigned int m_SBVBO;
	unsigned int m_SBProgramID;

	glm::vec3 m_backgroundColour;

	//IMGUI
	struct GuiPanel
	{
		std::string windowName;
		ImVec2 windowSize;
		ImVec2 windowPosition;
		bool expanded;
	};

	GuiPanel m_settingsPanel;
	GuiPanel m_objLoadingPanel;
	GuiPanel m_actorsPanel;
	GuiPanel m_actorPropertiesPanel;

	//actor properties
	std::vector<std::string> m_actors;
	std::string m_selectedActor;
	std::vector<bool> m_isActorSelected;
	std::vector<std::vector<float>> m_actorPosition;
	std::vector<std::vector<float>> m_actorRotation;
	std::vector<float> m_actorScale;

	ImGui::FileBrowser m_fileDialog;
	std::string m_currentFile;

	bool m_skyboxEnabled;
	bool m_frameDataEnabled;
	bool m_gridLinesEnabled;
};