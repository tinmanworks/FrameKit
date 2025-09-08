#pragma once

#include "FrameKit/Engine/Layer.h"

#include <iostream>

class DemoLayer : public FrameKit::Layer
{
public:
	explicit DemoLayer(const std::string& name = "DemoLayer")
		: FrameKit::Layer(name) {}

	void OnAttach() override {
		printf("DemoLayer::OnAttach\n");
	}

	void OnDetach() override {
		printf("DemoLayer::OnDetach\n");
	}

	void OnSyncUpdate(FrameKit::Timestep ts) override {
		// Example update logic
		static int num = 0;
		printf("DemoLayer::OnSyncUpdate (%d) - Delta Time: %f\n", num, ts.Seconds());
		num++;
	}

	void OnEvent(FrameKit::Event& event) override {
		// Example event handling
		printf("DemoLayer::OnEvent\n");
	}
};