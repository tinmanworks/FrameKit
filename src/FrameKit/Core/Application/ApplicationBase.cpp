// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Application/ApplicationBase.cpp
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-18
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Implements the ApplicationBase class for the FrameKit framework.
//      Provides lifecycle management, threading, event dispatch, and
//      layer management functionality for derived applications.
// =============================================================================

#include "FrameKit/Application/ApplicationBase.h"
#include "FrameKit/Engine/Layer.h"
#include "FrameKit/Core/Engine/LayerStack.h"
#include "FrameKit/Debug/Log.h"

namespace FrameKit {

	ApplicationBase::ApplicationBase()
		: ApplicationBase(ApplicationSpecification()) {
	}

	ApplicationBase::ApplicationBase(const ApplicationSpecification& spec)
		: m_Specification(spec) {
		m_LayerStack = new LayerStack();
		FK_CORE_INFO("ApplicationBase ctor: name='{}' mode={}", m_Specification.Name, (int)m_Specification.Mode);
	}

	void ApplicationBase::PushLayer(Layer* layer) {
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
		if (!m_LayerStack) { FK_CORE_ERROR("PushLayer failed: LayerStack is null"); return; }
		if (!layer) { FK_CORE_ERROR("PushLayer failed: layer is null"); return; }

		m_LayerStack->PushLayer(layer);
		FK_CORE_INFO("Layer pushed: {}", layer->GetName());
		layer->OnAttach();
		FK_CORE_TRACE("Layer attached: {}", layer->GetName());
	}

	void ApplicationBase::PushOverlay(Layer* layer) {
		std::scoped_lock<std::mutex> lock(m_LayerStackMutex);
		if (!m_LayerStack) { FK_CORE_ERROR("PushOverlay failed: LayerStack is null"); return; }
		if (!layer) { FK_CORE_ERROR("PushOverlay failed: layer is null"); return; }

		m_LayerStack->PushOverlay(layer);
		FK_CORE_INFO("Overlay pushed: {}", layer->GetName());
		layer->OnAttach();
		FK_CORE_TRACE("Overlay attached: {}", layer->GetName());
	}

} // namespace FrameKit
