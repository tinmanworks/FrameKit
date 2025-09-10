// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Application/ApplicationBase.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Implements the ApplicationBase class for the FrameKit framework.
//      Provides lifecycle management, threading, event dispatch, and
//      layer management functionality for derived applications.
// =============================================================================

#include "FrameKit/Application/ApplicationBase.h"
#include "FrameKit/Engine/Layer.h"
#include "FrameKit/Core/Engine/LayerStack.h"

namespace FrameKit {

	ApplicationBase::ApplicationBase() : ApplicationBase(ApplicationSpecification()) {}
	
	ApplicationBase::ApplicationBase(const ApplicationSpecification& spec) : m_Specification(spec) { m_LayerStack = new LayerStack(); }

	void ApplicationBase::PushLayer(Layer* layer) {
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
		if (m_LayerStack) {
			m_LayerStack->PushLayer(layer);
			layer->OnAttach();
		}
	}

	void ApplicationBase::PushOverlay(Layer* layer) {
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
		if (m_LayerStack) {
			m_LayerStack->PushOverlay(layer);
			layer->OnAttach();
		}
	}



} // namespace FrameKit
