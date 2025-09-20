#include "VideoLayer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glad/glad.h>

using namespace FrameKit;
using namespace FrameKit::MediaKit;

namespace SandBox {
    static GLenum kUploadFmt = GL_RGBA;

    VideoLayer::VideoLayer(std::string name) : Layer(std::move(name)) {}

    void VideoLayer::OnAttach() {
        FK_PROFILE_FUNCTION();

        const char* path = "S:/FlightDeck/FDVizApp/Resources/footage.mp4";

        PlayerConfig cfg{};
        cfg.hwDecode = false;
        cfg.outFmt = PixelFormat::RGBA8;   // force RGBA output from backend
        cfg.deliverGPU = false;
        cfg.videoQueue = 8;
        cfg.audioQueue = 32;

        m_Player = CreatePlayer(PlayerBackend::FFmpeg);
        if (!m_Player || !m_Player->open(path, cfg)) {
            FK_CORE_ERROR("VideoLayer: open failed for '{}'", path);
            m_Player.reset();
            return;
        }
        m_Player->setLoop(m_Loop);
        m_Player->setRate(m_Rate);
        m_Player->play();
        m_Paused = false;

        m_Player->setVideoSink([this](const FrameKit::MediaKit::VideoFrame& f) {
            // Expect RGBA/BGRA per cfg.outFmt
            if (f.planes.empty()) return;
            std::lock_guard<std::mutex> lk(m_FrameMtx);
            m_PendingW = f.info.w;
            m_PendingH = f.info.h;
            m_PendingRGBA = f.planes[0];     // copy; small and safe
            m_HasPending.store(true, std::memory_order_release);
            });

    }

    void VideoLayer::OnDetach() {
        FK_PROFILE_FUNCTION();
        if (m_Tex) { DestroyTexture(m_Tex); m_Tex = 0; }
        if (m_Player) m_Player->close();
        m_Player.reset();
    }

    void VideoLayer::OnAsyncUpdate() { FK_PROFILE_FUNCTION(); }

    void VideoLayer::OnSyncUpdate(Timestep) {}

    void VideoLayer::OnEvent(Event&) {}

    void VideoLayer::OnRender() {
        FK_PROFILE_FUNCTION();

        if (!m_Player) {
            ImGui::Begin("VideoPort");
            ImGui::TextUnformatted("Player not initialized");
            ImGui::End();
            return;
        }

        if (m_HasPending.exchange(false, std::memory_order_acq_rel)) {
            std::vector<uint8_t> rgba; int w = 0, h = 0;
            {
                std::lock_guard<std::mutex> lk(m_FrameMtx);
                rgba = std::move(m_PendingRGBA); w = m_PendingW; h = m_PendingH;
            }
            EnsureTexture(w, h);
            kUploadFmt = GL_RGBA; // cfg requests RGBA8
            UpdateTextureRGBA8(m_Tex, rgba.data(), w, h);
        }

        ImGui::Begin("VideoPort", nullptr,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const auto  st = m_Player->state();
        const double now = m_Player->time();
        const double dur = m_Player->info().demux.durationSec;

        if (ImGui::Button(m_Paused ? "Play" : "Pause")) {
            if (m_Paused) m_Player->play(); else m_Player->pause();
            m_Paused = !m_Paused;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) { m_Player->stop(); m_Paused = true; }
        ImGui::SameLine();
        if (ImGui::Button("Seek 0")) { m_Player->seek(0.0); }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::SliderFloat("Rate", reinterpret_cast<float*>(&m_Rate), 0.25f, 2.0f, "%.2fx"))
            m_Player->setRate(m_Rate);

        if (ImGui::Checkbox("Loop", &m_Loop)) m_Player->setLoop(m_Loop);

        // Seek bar if known duration
        if (dur > 0.0) {
            double t = now;
            double tmin = 0.0, tmax = dur;
            ImGui::SetNextItemWidth(280.0f);
            if (ImGui::SliderScalar("Seek", ImGuiDataType_Double, &t, &tmin, &tmax, "%.2f s")) {
                if (std::fabs(t - now) > 0.0005) m_Player->seek(t, false);
            }
        }


        ImGui::Text("State: %s | t %.3f s / %.3f s", ToString(st), now, dur);

        // Image
        if (m_Tex) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float imgAspect = m_TexH > 0 ? float(m_TexW) / float(m_TexH) : 1.0f;
            float availAspect = avail.y > 0 ? avail.x / avail.y : 1.0f;

            ImVec2 draw = avail;
            if (imgAspect > availAspect)  draw.y = draw.x / imgAspect;
            else                          draw.x = draw.y * imgAspect;

            ImVec2 cur = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cur.x + (avail.x - draw.x) * 0.5f,
                cur.y + (avail.y - draw.y) * 0.5f));

            ImGui::Image((ImTextureID)(intptr_t)m_Tex, draw, ImVec2(0, 0), ImVec2(1, 1));
        }
        else {
            ImGui::TextUnformatted("No frame yet");
        }

        ImGui::End();
    }

    // ---- helpers ---------------------------------------------------------------
    void VideoLayer::EnsureTexture(int w, int h) {
        if (m_Tex && w == m_TexW && h == m_TexH) return;
        if (m_Tex) DestroyTexture(m_Tex);
        m_Tex = CreateTextureRGBA8(w, h);
        m_TexW = w; m_TexH = h;
    }

    uint64_t VideoLayer::CreateTextureRGBA8(int w, int h) {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, kUploadFmt, GL_UNSIGNED_BYTE, nullptr);
        return static_cast<uint64_t>(tex);
    }

    void VideoLayer::UpdateTextureRGBA8(uint64_t tex, const void* data, int w, int h) {
        glBindTexture(GL_TEXTURE_2D, (GLuint)tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, kUploadFmt, GL_UNSIGNED_BYTE, data);
    }

    void VideoLayer::DestroyTexture(uint64_t handle) {
        GLuint tex = (GLuint)handle;
        if (tex) glDeleteTextures(1, &tex);
    }

    const char* VideoLayer::ToString(PlayerState s) {
        switch (s) {
        case PlayerState::Idle:    return "Idle";
        case PlayerState::Opening: return "Opening";
        case PlayerState::Paused:  return "Paused";
        case PlayerState::Playing: return "Playing";
        case PlayerState::Stopped: return "Stopped";
        case PlayerState::Ended:   return "Ended";
        case PlayerState::Error:   return "Error";
        default:                   return "?";
        }
    }
} // namespace SandBox
