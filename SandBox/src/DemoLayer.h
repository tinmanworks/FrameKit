#pragma once

#include "FrameKit/Engine/Layer.h"

#include <iostream>

class DemoLayer : public FrameKit::Layer
{
public:
	explicit DemoLayer(const std::string& name = "DemoLayer")
		: FrameKit::Layer(name) {}

	void OnAttach() override {
		FK_PROFILE_FUNCTION();
		FK_INFO("DemoLayer Attached");
	}

	void OnDetach() override {
		FK_PROFILE_FUNCTION();

		FK_INFO("DemoLayer Detached");
	}

	void OnSyncUpdate(FrameKit::Timestep ts) override {
		// Example update logic
		FK_PROFILE_FUNCTION();
		static int num = 0;
		FK_CRITICAL("DemoLayer::OnSyncUpdate {} - Delta Time: {}", num, ts.Seconds());
		num++;
	}

	void OnEvent(FrameKit::Event& event) override {
		// Example event handling
		printf("DemoLayer::OnEvent\n");
	}
};