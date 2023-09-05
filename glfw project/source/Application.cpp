#include "Application.h"
#include "ApplicationEvent.h"
#include "Dispatcher.h"
#include "Utilities.h"
#include "ShaderUtil.h"

//Include the OpenGL Header
#include <glad/glad.h>
//Include GLFW Header
#include <GLFW/glfw3.h>
//Include imgui Header
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

//include iostream for console logging
#include <iostream>

bool Application::Create(const char* a_applicationName, unsigned int a_windowWidth, unsigned int a_windowHeight, bool a_fullscreen)
{
	//Initialise GLFW
	if (!glfwInit()) { return false; }
	
	m_windowWidth = a_windowWidth;
	m_windowHeight = a_windowHeight;
	//create a windowed mode window and its OpenGL context
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, a_applicationName, (a_fullscreen ? glfwGetPrimaryMonitor() : nullptr), nullptr);
	if (!m_window)
	{
		glfwTerminate();
		return false;
	}

	//make the window's context current
	glfwMakeContextCurrent(m_window);

	//initialise GLAD - load in GL extensions
	if (!gladLoadGL())
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return false;
	}
	//get the supported OpenGL version
	int major = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MAJOR);
	int minor = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_VERSION_MINOR);
	int revision = glfwGetWindowAttrib(m_window, GLFW_CONTEXT_REVISION);

	std::cout << "OpenGL Version " << major << "." << minor << "." << revision << std::endl;

	//set up glfw window resize callback function
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow*, int w, int h)
	{
		//call the global dispatcher to handle this function
			Dispatcher* dp = Dispatcher::GetInstance();
			if (dp != nullptr)
			{
				dp->Publish(new WindowResizeEvent(w, h), true);
			}
	});

	//create dispatcher
	Dispatcher::CreateInstance();

	//setup IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//setup ImGui style
	ImGui::StyleColorsDark();
	const char* glsl_version = "#version 150";
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	//implement a call to the derived class onCreate function for any implementation specific code
	bool result = onCreate();
	if (result == false)
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
	return result;
}

void Application::Run(const char* a_name, unsigned int a_width, unsigned int a_height, bool a_fullscreen)
{
	if (Create(a_name, a_width, a_height, a_fullscreen))
	{
		Utility::resetTimer();
		m_running = true;
		do
		{
			//start imgui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			//showFrameData(true);

			float deltaTime = Utility::tickTimer();
			Update(deltaTime);

			Draw();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			//swap front and back buffers
			glfwSwapBuffers(m_window);
			//poll for and process events
			glfwPollEvents();
		} while (m_running && glfwWindowShouldClose(m_window) == 0);

		Destroy();
	}

	//cleanup
	ShaderUtil::DestroyInstance();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	glfwTerminate();

	Dispatcher::DestroyInstance();
}

void Application::showFrameData(bool a_bShowFrameData)
{
	const float distance = 10.0f;
	static int corner = 0;
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 windowPos = ImVec2((corner & 1) ? io.DisplaySize.x - distance : distance, (corner & 2) ? io.DisplaySize.y - distance : distance);
	ImVec2 windowPosPivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
	ImGui::SetNextWindowBgAlpha(0.3f);

	if (ImGui::Begin("Frame Data", &a_bShowFrameData, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Separator();
		ImGui::Text("Application Average: \n FPS : %0.1f \n %0.3f ms/frame", io.Framerate, 1000.0f / io.Framerate);

		if (ImGui::IsMousePosValid())
		{
			ImGui::Text("Mouse Position: \n X : %0.1f \n Y : %0.1f", io.MousePos.x, io.MousePos.y);
		}
		else
		{
			ImGui::Text("Mouse Position: \n <Active in other window>");
		}
	}
	ImGui::End();
}