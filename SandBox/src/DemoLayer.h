#pragma once

#include <FrameKit/FrameKit.h>

class DemoLayer : public FrameKit::Layer {
public:
    explicit DemoLayer(const std::string& name = "DemoLayer")
        : FrameKit::Layer(name) {}

    void OnAttach() override {
        FK_PROFILE_FUNCTION();
        FK_INFO("{} attached", GetName());
    }

    void OnDetach() override {
        FK_PROFILE_FUNCTION();
        FK_INFO("{} detached", GetName());
    }

    void OnSyncUpdate(FrameKit::Timestep ts) override {
        FK_PROFILE_FUNCTION();
        FK_TRACE("{} update dt={} ms", GetName(), ts.Milliseconds());
    }

    void OnEvent(FrameKit::Event& event) override {
        FK_PROFILE_FUNCTION();
        FK_TRACE("{} received event type={}", GetName(), event.GetName());
    }
};
