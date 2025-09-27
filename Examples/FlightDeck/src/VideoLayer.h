#pragma once
#include <FrameKit/FrameKit.h>
#include <FrameKit/MediaKit/MediaKit.h>

#include "utils/FileDialog.h"

#include <memory>
#include <vector>
#include <cstdint>
#include <mutex>
#include <atomic>

namespace FlightDeck {
    class VideoLayer : public FrameKit::Layer {
    public:
        VideoLayer(std::string name);
        ~VideoLayer() override = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnAsyncUpdate() override;
        void OnSyncUpdate(FrameKit::Timestep ts) override;
        void OnRender() override;
        void OnEvent(FrameKit::Event& e) override;

    private:
        // renderer hooks
        uint64_t CreateTextureRGBA8(int w, int h);
        void     UpdateTextureRGBA8(uint64_t tex, const void* rgba, int w, int h);
        void     DestroyTexture(uint64_t tex);

        void EnsureTexture(int w, int h);
        static const char* ToString(FrameKit::MediaKit::PlayerState s);

    private:
        std::string m_Path;        // user-provided file path
        void OpenMedia(const std::string& path);
        std::unique_ptr<FrameKit::MediaKit::IPlayer> m_Player;
        bool      m_Paused{ true };
        bool      m_Loop{ false };
        double    m_Rate{ 1.0 };

        // Texture
        uint64_t  m_Tex{ 0 };
        int       m_TexW{ 0 };
        int       m_TexH{ 0 };

    private:
        std::atomic<bool> m_Alive{ true };
        std::mutex              m_FrameMtx;
        std::vector<uint8_t>    m_PendingRGBA;
        int                     m_PendingW{ 0 }, m_PendingH{ 0 };
        std::atomic<bool>       m_HasPending{ false };

    private:
        ui::FileDialog m_FileDlg;

    };
} // namespace FlightDeck
