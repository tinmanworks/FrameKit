// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit/Application/ApplicationBase.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Defines the ApplicationBase class and related specifications for the
//      FrameKit framework. Handles lifecycle, threading, event dispatch, and
//      layer management for derived applications.
// =============================================================================

#pragma once

#include "FrameKit/Application/AppSpec.h"

namespace FrameKit
{
	// forward declarations
	class Event;

	// ---------- Application Base Class ----------
	class ApplicationBase {
	public:
		ApplicationBase() = default;
		explicit ApplicationBase(const ApplicationSpecification& spec) : m_Specification(spec) {};
		virtual ~ApplicationBase() = default;

		ApplicationBase(const ApplicationBase&) = delete;
		ApplicationBase& operator=(const ApplicationBase&) = delete;
		ApplicationBase(ApplicationBase&&) = delete;
		ApplicationBase& operator=(ApplicationBase&&) = delete;

		ApplicationSpecification& GetSpec()				{ return m_Specification; }
		const ApplicationSpecification& GetSpec() const { return m_Specification; }

		// lifecycle
		virtual bool Init() = 0;
		virtual void Shutdown() = 0;

		// run-loop hooks (override what you need)
		virtual void OnBeforePoll() {}
		virtual void OnAfterPoll() {}
		virtual void OnBeforeUpdate(double /*deltaTime*/) {}
		virtual void OnAfterUpdate(double /*deltaTime*/) {}
		virtual void OnBeforeRender() {}
		virtual void OnAfterRender() {}
		virtual void OnFrameEnd() {}

		// app behavior
		virtual bool OnUpdate(double /*deltaTime*/) { return true; }	// return false to close app
		virtual void OnRender() {}										// only called in windowed mode
		virtual void OnEvent(Event& /*e*/) {}							// executed after layers; mark handled to stop propagation
		virtual void OnUnhandledEvent(Event& /*e*/) {}					// executed if event was unhandled by layers and app

	private:
		ApplicationSpecification m_Specification;
	};
}