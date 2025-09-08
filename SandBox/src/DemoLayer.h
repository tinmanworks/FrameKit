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

	void OnSyncUpdate(double deltaTime) override {
		// Example update logic
		printf("DemoLayer::OnSyncUpdate - Delta Time: %f\n", deltaTime);
	}

	void OnEvent(FrameKit::Event& event) override {
		// Example event handling
		printf("DemoLayer::OnEvent\n");
	}
};