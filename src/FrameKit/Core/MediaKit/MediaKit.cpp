// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/MediaKit.cpp
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		Central implementation file for media system. Includes media backends
//		and provides utility functions to access media resources.
// =============================================================================

#include "FrameKit/MediaKit/MediaKit.h"

#include <stdexcept>

namespace FrameKit::MediaKit {

    // Makers provided by backends we build
#if defined(FK_MEDIA_ENABLE_STB)
    std::unique_ptr<IImageLoader> MakeImageLoader_Stb();
#endif

#if defined(FK_MEDIA_ENABLE_FFMPEG)
    std::unique_ptr<IVideoReader> MakeVideoReader_FFmpeg();
    std::unique_ptr<IPlayer>      MakePlayer_FFmpeg();
#endif

    // -------- Central factories --------
    std::unique_ptr<IImageLoader> CreateImageLoader(ImageBackend b) {
        switch (b) {
#if defined(FK_MEDIA_ENABLE_STB)
        case ImageBackend::Stb:  return MakeImageLoader_Stb();
#endif
            // case ImageBackend::OIIO: return MakeImageLoader_OIIO();
        default: throw std::runtime_error("Image backend not built");
        }
    }

    std::unique_ptr<IVideoReader> CreateVideoReader(VideoBackend b) {
        switch (b) {
#if defined(FK_MEDIA_ENABLE_FFMPEG)
        case VideoBackend::FFmpeg: return MakeVideoReader_FFmpeg();
#endif
        default: throw std::runtime_error("Video backend not built");
        }
    }

    std::unique_ptr<IPlayer> CreatePlayer(PlayerBackend b) {
        switch (b) {
#if defined(FK_MEDIA_ENABLE_FFMPEG)
        case PlayerBackend::FFmpeg: return MakePlayer_FFmpeg();
#endif
        default: throw std::runtime_error("Player backend not built");
        }
    }
} // namespace FrameKit::MediaKit
