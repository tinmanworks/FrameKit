// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/ImageLoader_Stb.cpp
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		Image loader implementation using stb_image
// =============================================================================

#include "FrameKit/MediaKit/MediaKit.h"
#if defined(FK_MEDIA_ENABLE_STB)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_GIF
#include "stb_image/stb_image.h"

#include <stdexcept>
#include <string>

namespace FrameKit::MediaKit {

    class StbImageLoader final : public IImageLoader {
    public:
        ImageData load(std::string_view path) override {
            int w = 0, h = 0, comp = 0;
            stbi_uc* data = stbi_load(std::string(path).c_str(), &w, &h, &comp, 4);
            if (!data) throw std::runtime_error("stb_image: failed to load: " + std::string(path));
            ImageData out;
            out.desc = { w, h, PixelFormat::RGBA8, {} };
            out.owned.assign(data, data + static_cast<size_t>(w) * static_cast<size_t>(h) * 4u);
            stbi_image_free(data);
            return out;
        }
    };

    std::unique_ptr<IImageLoader> MakeImageLoader_Stb() {
        return std::make_unique<StbImageLoader>();
    }

} // namespace FrameKit::MediaKit
#endif // FK_MEDIA_ENABLE_STB