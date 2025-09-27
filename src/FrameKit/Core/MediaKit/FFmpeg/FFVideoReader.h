// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Core/MediaKit/FFmpeg/FFVideoReader.h
// Author       : George Gil
// Created      : 2025-09-19
// Updated      : 2025-09-19
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : 
//		FFmpeg-based video reader implementation.
// =============================================================================
#pragma once
#include "FrameKit/MediaKit/MediaKit.h"
#include "FFCommon.h"

namespace FrameKit::MediaKit {
    class FFVideoReader final : public IVideoReader {
    public:
        DemuxInfo open(std::string_view path) override;
        bool read(VideoFrame* vf, AudioFrame* af) override;
        bool seek(double seconds, bool exact) override;
        void close() override;
        ~FFVideoReader() override { close(); }
    private:
        AVFormatContext* fmt_ = nullptr;
        AVCodecContext* vctx_ = nullptr;
        AVCodecContext* actx_ = nullptr;
        int vs_ = -1, as_ = -1;
        AVPacket* pkt_ = nullptr;
        AVFrame* frm_ = nullptr;
        SwsContext* sws_ = nullptr; // only if we must convert to RGBA
        DemuxInfo info_{};
    };

} // namespace
