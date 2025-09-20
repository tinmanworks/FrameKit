#include "ImGuiLayer.h"

#include <FrameKit/Window.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace FrameKit;

namespace SandBox {
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

    void ImGuiLayer::OnAttach()
    {
        FK_PROFILE_FUNCTION();
        IWindow* fk_window = GetPrimaryWindow();
        if (!fk_window) {
            FK_ERROR("No primary window available");
            return;
        }
        GLFWwindow* window = static_cast<GLFWwindow*>(fk_window->nativeHandle());
        IM_ASSERT(window && "Native handle is null");
        if (!window) {
            FK_ERROR("No native window handle available");
            return;
		}

        // Ensure context is current here (some apps move threads)
        if (glfwGetCurrentContext() != window)
            glfwMakeContextCurrent(window);


        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            FK_ERROR("gladLoadGLLoader failed");
            return;
        }

        const char* glsl = "#version 330 core";

        // --- ImGui core ---
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO(); (void)io;

      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Keyboard navigation
      // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Gamepad navigation (optional)
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // Docking
      io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Multi-Viewport

    //// --- ImGui style ---
      ImGui::StyleColorsDark();
      if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
          ImGuiStyle& style = ImGui::GetStyle();
          style.WindowRounding = 0.0f;
          style.Colors[ImGuiCol_WindowBg].w = 1.0f;
      }
      SetDarkThemeColors();

      // --- Backends ---
      ImGui_ImplGlfw_InitForOpenGL(window, /*install_callbacks=*/true);
      ImGui_ImplOpenGL3_Init(glsl);

    }


    void ImGuiLayer::OnDetach()
    {
        FK_PROFILE_FUNCTION();

        // Shutdown backends first, then destroy context
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext();
	}

    void ImGuiLayer::OnEvent(FrameKit::Event& e)
    {
        if (m_BlockEvents) {
            ImGuiIO& io = ImGui::GetIO();
            // Note: bitwise '&' is fine on bools; mirrors ImGui examples.
            e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
	}

    void ImGuiLayer::Begin()
    {
        FK_PROFILE_FUNCTION();
        
        // update viewport to framebuffer size
        int fbw = 0, fbh = 0;
        GLFWwindow* window = static_cast<GLFWwindow*>(GetPrimaryWindow()->nativeHandle());
        glfwGetFramebufferSize(window, &fbw, &fbh);
        if (fbw == 0 || fbh == 0) { // minimized
            glfwWaitEventsTimeout(0.016);
            return;
        }
        glViewport(0, 0, fbw, fbh);

        // New frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ---------- Fullscreen DockSpace ----------
        static bool dockspaceOpen = true;
        static bool opt_fullscreen_persist = true;
        bool opt_fullscreen = opt_fullscreen_persist;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        if (opt_fullscreen) {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar(); // WindowPadding

        if (opt_fullscreen)
            ImGui::PopStyleVar(2); // WindowRounding, WindowBorderSize

        // DockSpace itself
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        const float prevMinWinSizeX = style.WindowMinSize.x;
        const float prevMinWinSizeY = style.WindowMinSize.y;
        style.WindowMinSize.x = 100.0f;
        style.WindowMinSize.y = 50.0f;

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.0f), dockspace_flags);
        }

        style.WindowMinSize.x = prevMinWinSizeX;
        style.WindowMinSize.y = prevMinWinSizeY;
    }

    void ImGuiLayer::End()
    {
        FK_PROFILE_FUNCTION();

        ImGui::End();
        ImGuiIO& io = ImGui::GetIO();
        auto* fk_window = GetPrimaryWindow();
        if (!fk_window) {
            FK_ERROR("No primary window available");
            return;
        }
        io.DisplaySize = ImVec2((float)fk_window->width(), (float)fk_window->height());

        // Render main viewport
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
	}


    void ImGuiLayer::SetDarkThemeColors()
    {
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.10f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.20f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.30f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.20f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.30f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frames
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.20f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.30f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.20f, 0.205f, 0.21f, 1.0f };

        // Title bars
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_TitleBg];
        colors[ImGuiCol_TitleBgCollapsed] = colors[ImGuiCol_TitleBg];
    }
} // namespace FlightDeck
