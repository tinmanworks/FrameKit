#pragma once

#include <FrameKit/FrameKit.h>
#include <FrameKit/MediaKit/MediaKit.h>

namespace SandBox {
	class VideoLayer : public FrameKit::Layer
	{
	public:
		VideoLayer(std::string name);
		virtual ~VideoLayer() = default;

	public:
		// derived Methods
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnAsyncUpdate() override;
		virtual void OnSyncUpdate(FrameKit::Timestep ts) override;
		virtual void OnRender() override;
		virtual void OnEvent(FrameKit::Event& e) override;

	private:
		// renderer hooks you already have (implement somewhere in your gfx layer)
		uint64_t CreateTextureRGBA8(int w, int h);
		void     UpdateTextureRGBA8(uint64_t tex, const void* rgba, int w, int h);
		void     DestroyTexture(uint64_t tex);

		void EnsureTexture(int w, int h);

	private:
		std::unique_ptr<FrameKit::MediaKit::IPlayer> m_Player;
		bool      m_Paused{ true };
		uint64_t  m_Tex{ 0 };
		int       m_TexW{ 0 }, m_TexH{ 0 };
	};
}