#include "VideoLayer.h"

// #include <imgui.h>

using namespace FrameKit;
using namespace FrameKit::MediaKit;


namespace SandBox {

    VideoLayer::VideoLayer(std::string name)
        : Layer(name) {
    }

    void VideoLayer::OnAttach() {
        FK_PROFILE_FUNCTION();

        // Open a sample file. Replace with your file picker or config path.
        const char* path = "S:/FlightDeck/FDVizApp/Resources/Test.mp4";

        PlayerConfig cfg;
        cfg.hwDecode = false;
        cfg.outFmt = PixelFormat::RGBA8;
        cfg.deliverGPU = false;

        m_Player = CreatePlayer(PlayerBackend::FFmpeg);
        m_Player->open(path, cfg);
        m_Player->play();
        m_Paused = false;
    }

    void VideoLayer::OnDetach() {
        FK_PROFILE_FUNCTION();
        if (m_Tex) { DestroyTexture(m_Tex); m_Tex = 0; }
        m_Player.reset();
    }

    void VideoLayer::OnAsyncUpdate() {
        FK_PROFILE_FUNCTION();
        // No background decode in MVP.
    }

    void VideoLayer::OnSyncUpdate(Timestep /*ts*/) {
        FK_PROFILE_FUNCTION();

        if (!m_Player) return;

        // Pull one frame if available and upload
        VideoFrame vf;
        if (m_Player->getVideo(vf)) {
            EnsureTexture(vf.info.w, vf.info.h);
            if (m_Tex) {
                // vf.planes[0] is packed RGBA8 of size w*h*4
                UpdateTextureRGBA8(m_Tex, vf.planes[0].data(), vf.info.w, vf.info.h);
            }
        }

        // Optional: pull audio and feed your audio mixer
        // AudioFrame af; if (m_Player->getAudio(af)) { queue to audio device }
    }

    void VideoLayer::OnEvent(Event& e) {
        FK_PROFILE_FUNCTION();
        (void)e;
    }

    void VideoLayer::OnRender() {
        FK_PROFILE_FUNCTION();

        // ImGui::Begin("VideoPort");
        // if (ImGui::Button(m_Paused ? "Play" : "Pause")) {
        //     if (m_Paused) m_Player->play(); else m_Player->pause();
        //     m_Paused = !m_Paused;
        // }
        // ImGui::SameLine();
        // if (ImGui::Button("Stop")) { m_Player->stop(); m_Paused = true; }

        // ImGui::SameLine();
        // if (ImGui::Button("Seek 0")) { m_Player->seek(0.0); }

        // ImGui::Text("Clock: %.3f s", m_Player ? m_Player->clock() : 0.0);

        // if (m_Tex) {
        //     const float w = (float)m_TexW, h = (float)m_TexH;
        //     ImGui::Image((ImTextureID)(intptr_t)m_Tex, ImVec2(w, h));
        // }
        // else {
        //     ImGui::TextUnformatted("No frame yet");
        // }
        // ImGui::End();
    }

    // --- private -------------------------------------------------------------

    void VideoLayer::EnsureTexture(int w, int h) {
        if (m_Tex && w == m_TexW && h == m_TexH) return;
        if (m_Tex) DestroyTexture(m_Tex);
        m_Tex = CreateTextureRGBA8(w, h);
        m_TexW = w; m_TexH = h;
    }

    // Stub hooks: implement using your renderer backend.
    // For example, GL: glTexImage2D + glTexSubImage2D. Vulkan: staging upload.
    uint64_t VideoLayer::CreateTextureRGBA8(int, int) { return 0; }
    void     VideoLayer::UpdateTextureRGBA8(uint64_t, const void*, int, int) {}
    void     VideoLayer::DestroyTexture(uint64_t) {}

} // namespace SandBox
