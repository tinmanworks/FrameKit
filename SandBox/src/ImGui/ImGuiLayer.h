// =============================================================================
// Project      : FlightDeck
// File         : src/ImGui/ImGuiLayer.h
// Author       : George Gil
// Created      : 2025-08-10
// Updated      : 2025-09-18
// Description  : Implements Dear ImGui integration for FrameKit.
//   - Creates/tears down ImGui state in OnAttach/OnDetach.
//   - Drives per-frame UI via Begin() / End().
//   - Optionally blocks engine events when interacting with UI.
//   - Exposes a GlobalContext for add-ons / external renderers.
// =============================================================================


#pragma once

#if defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#endif

#include <cstdint>
#include <FrameKit/FrameKit.h>
#include <imgui.h>

namespace SandBox {

	class ImGuiLayer : public FrameKit::Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer() override = default;

		// Layer lifecycle
		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(FrameKit::Event& e) override;

		// Per-frame UI
		void Begin();
		void End();

		void OnRender() override {
			if (ImGui::Begin("ImGui Helper Panel")) {
				if (ImGui::Button("Toggle Panel Direction")) m_HelperTogglePanelDirection = !m_HelperTogglePanelDirection;
				if (m_HelperTogglePanelDirection) ImGui::SameLine();
				if (ImGui::Button("Show ImGui DemoWindow")) ShowDemoWindow();
				if (m_HelperTogglePanelDirection) ImGui::SameLine();
				if (ImGui::Button("Hide ImGui DemoWindow")) HideDemoWindow();
			}
			ImGui::End();

			if (m_ShowDemoWindow) ImGui::ShowDemoWindow(&m_ShowDemoWindow);

		}

		void ShowDemoWindow() { m_ShowDemoWindow = true; }
		void HideDemoWindow() { m_ShowDemoWindow = false; }

		// When true, input events are consumed by the UI and not forwarded.
		void BlockEvents(bool block) { m_BlockEvents = block; }

		// Apply a consistent dark theme to the current ImGui context.
		void SetDarkThemeColors();

		// Returns the currently active widget ID (if applicable).
		FK_NODISCARD uint32_t GetActiveWidgetID() const;

		// Non-copyable, non-movable (UI layer owns context hooks)
		ImGuiLayer(const ImGuiLayer&) = delete;
		ImGuiLayer& operator=(const ImGuiLayer&) = delete;
		ImGuiLayer(ImGuiLayer&&) = delete;
		ImGuiLayer& operator=(ImGuiLayer&&) = delete;

	private:
		bool m_ShowDemoWindow = false;
		bool m_HelperTogglePanelDirection = true;
		bool m_BlockEvents = false;
	};

} // namespace FlightDeck
